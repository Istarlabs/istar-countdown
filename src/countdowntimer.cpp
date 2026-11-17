// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "countdowntimer.h"

#include "occurrence.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QTime>

#include <algorithm>

#ifndef Q_OS_ANDROID
#include <QDBusConnection>
#include <QDBusMessage>
#endif

using namespace Qt::StringLiterals;

namespace
{
    /// Sets the progress bar on this program's task manager entry. Plasma listens for the Unity launcher
    /// API, so this works minimized too and needs no KDE library.
    void publishProgress([[maybe_unused]] double progress, [[maybe_unused]] bool visible)
    {
#ifndef Q_OS_ANDROID
        QDBusMessage message = QDBusMessage::createSignal(u"/net/kurokyuu/istar_countdown"_s,
                                                          u"com.canonical.Unity.LauncherEntry"_s, u"Update"_s);

        message << u"application://net.kurokyuu.istar-countdown.desktop"_s
                << QVariantMap{{u"progress"_s, progress}, {u"progress-visible"_s, visible}};
        QDBusConnection::sessionBus().send(message);
#endif
    }

    constexpr qint64 SecondsPerDay = 24LL * 60 * 60;

    constexpr auto FinishedTextKey = "FinishedText"_L1;
    constexpr auto StartColorKey = "StartColor"_L1;
    constexpr auto EndColorKey = "EndColor"_L1;

    /// The [Timer] group of ~/.config/istar-countdownrc.
    KConfigGroup settings()
    {
        return KSharedConfig::openConfig()->group(u"Timer"_s);
    }

    template <typename Value>
    void remember(QLatin1StringView key, const Value& value)
    {
        KConfigGroup group = settings();

        group.writeEntry(key, value);
        group.sync();
    }
} // namespace

namespace IstarCountdown
{
    QString formatRemaining(qint64 seconds)
    {
        seconds = qMax<qint64>(seconds, 0);

        QString clock = QTime(0, 0).addSecs(static_cast<int>(seconds % SecondsPerDay)).toString(u"hh:mm:ss"_s);
        const qint64 days = seconds / SecondsPerDay;

        if (days == 0)
        {
            return clock;
        }

        // i18n: %1 is a number of days, %2 the remaining time of day as hh:mm:ss
        return i18np("%1 day %2", "%1 days %2", days, clock);
    }

    CountdownTimer::CountdownTimer(QObject* parent) : QObject(parent), m_text(formatRemaining(0))
    {
        const KConfigGroup group = settings();

        // The defaults are the colours and the message the WinUI version shipped with.
        m_finishedText = group.readEntry(FinishedTextKey, i18n("Let's go!"));
        m_startColor = group.readEntry(StartColorKey, QColor(0x3C, 0xA1, 0xDD));
        m_endColor = group.readEntry(EndColorKey, QColor(0x6B, 0x8C, 0xDF));

        m_ticker.setInterval(1000);
        connect(&m_ticker, &QTimer::timeout, this, &CountdownTimer::tick);
    }

    QString CountdownTimer::text() const
    {
        return m_text;
    }

    bool CountdownTimer::running() const
    {
        return m_ticker.isActive();
    }

    QString CountdownTimer::finishedText() const
    {
        return m_finishedText;
    }

    QColor CountdownTimer::startColor() const
    {
        return m_startColor;
    }

    QColor CountdownTimer::endColor() const
    {
        return m_endColor;
    }

    double CountdownTimer::progress() const
    {
        const qint64 total = m_start.msecsTo(m_target);

        if (total <= 0)
        {
            return 0;
        }

        const auto left = static_cast<double>(QDateTime::currentDateTime().msecsTo(m_target));

        return std::clamp(left / static_cast<double>(total), 0.0, 1.0);
    }

    void CountdownTimer::setFinishedText(const QString& text)
    {
        if (text == m_finishedText)
        {
            return;
        }

        m_finishedText = text;
        remember(FinishedTextKey, text);
        Q_EMIT finishedTextChanged();
    }

    void CountdownTimer::setStartColor(const QColor& color)
    {
        if (color == m_startColor)
        {
            return;
        }

        m_startColor = color;
        remember(StartColorKey, color);
        Q_EMIT startColorChanged();
    }

    void CountdownTimer::setEndColor(const QColor& color)
    {
        if (color == m_endColor)
        {
            return;
        }

        m_endColor = color;
        remember(EndColorKey, color);
        Q_EMIT endColorChanged();
    }

    void CountdownTimer::startCountdown(const QDateTime& start, int occurrence, const QString& message)
    {
        const bool known = occurrence >= 0 && occurrence < OccurrenceCount;

        setFinishedText(message);
        startAt(nextOccurrence(start, known ? static_cast<Occurrence>(occurrence) : Occurrence::Once,
                               QDateTime::currentDateTime()));
    }

    void CountdownTimer::startAt(const QDateTime& target, const QDateTime& from)
    {
        m_start = from.isValid() ? from : QDateTime::currentDateTime();
        m_target = target;
        m_ticker.start();
        Q_EMIT runningChanged();
        tick();
    }

    void CountdownTimer::stop()
    {
        if (!m_ticker.isActive())
        {
            return;
        }

        m_ticker.stop();
        publishProgress(0, false);
        Q_EMIT runningChanged();
    }

    void CountdownTimer::tick()
    {
        const qint64 remaining = QDateTime::currentDateTime().secsTo(m_target);

        if (remaining < 0)
        {
            setText(m_finishedText);
            stop();

            // A one-off started after its date is over at once; that is no news worth a popup.
            if (m_start < m_target)
            {
                Q_EMIT finished();
            }

            return;
        }

        setText(formatRemaining(remaining));
        publishProgress(progress(), true);
    }

    void CountdownTimer::setText(const QString& text)
    {
        if (text == m_text)
        {
            return;
        }

        m_text = text;
        Q_EMIT textChanged();
    }
} // namespace IstarCountdown
