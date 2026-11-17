// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "worktime.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

using namespace Qt::StringLiterals;

namespace
{
    constexpr int MinutesPerDay = 24 * 60;

    constexpr auto PauseKey = "PauseMinutes"_L1;
    constexpr auto TargetKey = "TargetMinutes"_L1;

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

    WorkTime::WorkTime(QObject* parent) : QObject(parent), m_start(wholeMinute(QDateTime::currentDateTime()))
    {
        const KConfigGroup group = settings();

        m_pauseMinutes = group.readEntry(PauseKey, m_pauseMinutes);
        m_targetMinutes = group.readEntry(TargetKey, m_targetMinutes);
        m_end = {m_start.date(), workEnd(m_start.time(), m_pauseMinutes, m_targetMinutes)};
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
        const int minutes = workedMinutes(m_start.time(), m_end.time(), m_pauseMinutes);

        // i18n: %1 is the hours and %2 the minutes of the time worked, e.g. "7 h 45 min"
        return i18nc("worked time", "%1 h %2 min", minutes / 60, minutes % 60);
    }

    void WorkTime::setStart(const QDateTime& start)
    {
        const QDateTime value = wholeMinute(start);

        if (value == m_start)
        {
            return;
        }

        m_start = value;
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
        setEnd({m_start.date(), workEnd(m_start.time(), m_pauseMinutes, m_targetMinutes)});
    }
} // namespace IstarCountdown
