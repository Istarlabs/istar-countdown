// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "worktime.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QStringList>

using namespace Qt::StringLiterals;

namespace
{
    constexpr int MinutesPerDay = 24 * 60;

    constexpr auto PauseKey = "PauseMinutes"_L1;
    constexpr auto TargetKey = "TargetMinutes"_L1;
    constexpr auto DayStartKey = "DayStart"_L1;
    constexpr auto PauseStampsKey = "PauseStamps"_L1;

    /// The [WorkTime] group of ~/.config/istar-countdownrc.
    KConfigGroup settings()
    {
        return KSharedConfig::openConfig()->group(u"WorkTime"_s);
    }

    void remember(QLatin1StringView key, int minutes)
    {
        KConfigGroup group = settings();

        group.writeEntry(key, minutes);
        group.sync();
    }

    /// The picker has no seconds, so neither does anything compared against it.
    QDateTime wholeMinute(const QDateTime& dateTime)
    {
        return {dateTime.date(), QTime(dateTime.time().hour(), dateTime.time().minute())};
    }
} // namespace

namespace IstarCountdown
{
    QTime workEnd(QTime start, int pauseMinutes, int targetMinutes)
    {
        return start.addSecs((pauseMinutes + targetMinutes) * 60);
    }

    int workedMinutes(QTime start, QTime end, int pauseMinutes)
    {
        const int span = ((start.secsTo(end) / 60) + MinutesPerDay) % MinutesPerDay;

        return qMax(span - pauseMinutes, 0);
    }

    int effectivePauseMinutes(int fixedMinutes, int clockedMinutes)
    {
        return qMax(fixedMinutes, clockedMinutes);
    }

    int clockedMinutes(const QList<QDateTime>& stamps)
    {
        qint64 seconds = 0;

        // An end set before its begin while correcting a pause counts as nothing, not as time worked.
        for (qsizetype index = 0; index + 1 < stamps.size(); index += 2)
        {
            seconds += qMax<qint64>(stamps.at(index).secsTo(stamps.at(index + 1)), 0);
        }

        return static_cast<int>(seconds / 60);
    }

    WorkTime::WorkTime(QObject* parent) : QObject(parent), m_start(wholeMinute(QDateTime::currentDateTime()))
    {
        const KConfigGroup group = settings();

        m_pauseMinutes = group.readEntry(PauseKey, m_pauseMinutes);
        m_targetMinutes = group.readEntry(TargetKey, m_targetMinutes);

        // ponytail: a day is "today" by its start's date, so a night shift loses its stamps when the program
        // restarts after midnight; key the day by the clock-in instead if that ever matters.
        if (const QDateTime dayStart = group.readEntry(DayStartKey, QDateTime());
            dayStart.isValid() && dayStart.date() == QDate::currentDate())
        {
            m_start = dayStart;

            for (const QString& stamp : group.readEntry(PauseStampsKey, QStringList()))
            {
                m_pauseStamps.append(QDateTime::fromString(stamp, Qt::ISODate));
            }
        }

        m_end = {m_start.date(), workEnd(m_start.time(), pauseToDeduct(), m_targetMinutes)};
    }

    QDateTime WorkTime::start() const
    {
        return m_start;
    }

    QDateTime WorkTime::end() const
    {
        return m_end;
    }

    int WorkTime::pauseMinutes() const
    {
        return m_pauseMinutes;
    }

    int WorkTime::targetMinutes() const
    {
        return m_targetMinutes;
    }

    QString WorkTime::resultText() const
    {
        const int minutes = workedMinutes(m_start.time(), m_end.time(), pauseToDeduct());

        // i18n: %1 is the hours and %2 the minutes of the time worked, e.g. "7 h 45 min"
        return i18nc("worked time", "%1 h %2 min", minutes / 60, minutes % 60);
    }

    bool WorkTime::paused() const
    {
        return m_pauseStamps.size() % 2 == 1;
    }

    QVariantList WorkTime::pauseStamps() const
    {
        QVariantList stamps;

        for (const QDateTime& stamp : m_pauseStamps)
        {
            stamps.append(stamp);
        }

        return stamps;
    }

    void WorkTime::addPause()
    {
        const QDateTime now = wholeMinute(QDateTime::currentDateTime());

        // A running pause has to stay the last stamp, or begins and ends would swap places.
        m_pauseStamps.insert(m_pauseStamps.size() - (paused() ? 1 : 0), 2, now);
        pausesEdited();
    }

    void WorkTime::setPauseStamp(int index, const QDateTime& stamp)
    {
        const QDateTime value = wholeMinute(stamp);

        if (index < 0 || index >= m_pauseStamps.size() || m_pauseStamps.at(index) == value)
        {
            return;
        }

        m_pauseStamps.replace(index, value);
        pausesEdited();
    }

    void WorkTime::removePause(int pause)
    {
        const qsizetype begin = static_cast<qsizetype>(pause) * 2;

        if (pause < 0 || begin >= m_pauseStamps.size())
        {
            return;
        }

        m_pauseStamps.remove(begin, qMin<qsizetype>(2, m_pauseStamps.size() - begin));
        pausesEdited();
    }

    void WorkTime::pausesEdited()
    {
        rememberDay();
        recalculateEnd();
        Q_EMIT pausesChanged();
    }

    void WorkTime::setStart(const QDateTime& start)
    {
        const QDateTime value = wholeMinute(start);

        if (value == m_start)
        {
            return;
        }

        m_start = value;
        rememberDay();
        Q_EMIT startChanged();
        recalculateEnd();
    }

    void WorkTime::setEnd(const QDateTime& end)
    {
        const QDateTime value = wholeMinute(end);

        if (value == m_end)
        {
            return;
        }

        m_end = value;
        Q_EMIT endChanged();
        Q_EMIT resultTextChanged();
    }

    void WorkTime::setPauseMinutes(int minutes)
    {
        if (minutes == m_pauseMinutes)
        {
            return;
        }

        m_pauseMinutes = minutes;
        remember(PauseKey, minutes);
        Q_EMIT pauseMinutesChanged();
        recalculateEnd();
    }

    void WorkTime::setTargetMinutes(int minutes)
    {
        if (minutes == m_targetMinutes)
        {
            return;
        }

        m_targetMinutes = minutes;
        remember(TargetKey, minutes);
        Q_EMIT targetMinutesChanged();
        recalculateEnd();
    }

    void WorkTime::clockIn()
    {
        if (!m_pauseStamps.isEmpty())
        {
            m_pauseStamps.clear();
            Q_EMIT pausesChanged();
        }

        setStart(QDateTime::currentDateTime());
        rememberDay();
        recalculateEnd();
    }

    void WorkTime::togglePause()
    {
        m_pauseStamps.append(wholeMinute(QDateTime::currentDateTime()));
        rememberDay();

        // A running pause only counts once it is over, so only its end moves the end of the day - and that
        // before pausesChanged, so whoever resumes on it already sees the new end.
        if (!paused())
        {
            recalculateEnd();
        }

        Q_EMIT pausesChanged();
    }

    QDateTime WorkTime::endTarget() const
    {
        // A day that runs past midnight ends tomorrow. If the shown end is already behind us the
        // timer reports it as over, which is what it is.
        const QDate day = m_end.time() < m_start.time() ? QDate::currentDate().addDays(1) : QDate::currentDate();

        return {day, m_end.time()};
    }

    QDateTime WorkTime::startTarget() const
    {
        const int span = ((m_start.time().secsTo(m_end.time()) / 60) + MinutesPerDay) % MinutesPerDay;

        return endTarget().addSecs(-static_cast<qint64>(span) * 60);
    }

    void WorkTime::recalculateEnd()
    {
        setEnd({m_start.date(), workEnd(m_start.time(), pauseToDeduct(), m_targetMinutes)});
    }

    void WorkTime::rememberDay() const
    {
        KConfigGroup group = settings();
        QStringList stamps;

        for (const QDateTime& stamp : m_pauseStamps)
        {
            stamps.append(stamp.toString(Qt::ISODate));
        }

        group.writeEntry(DayStartKey, m_start);
        group.writeEntry(PauseStampsKey, stamps);
        group.sync();
    }

    int WorkTime::pauseToDeduct() const
    {
        return effectivePauseMinutes(m_pauseMinutes, clockedMinutes(m_pauseStamps));
    }
} // namespace IstarCountdown
