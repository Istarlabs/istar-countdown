// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#pragma once

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QTime>
#include <QtQmlIntegration>

namespace IstarCountdown
{
    /// When the day ends if it starts at `start`, pauses for `pauseMinutes` and is to be
    /// `targetMinutes` long. Wraps past midnight.
    QTime workEnd(QTime start, int pauseMinutes, int targetMinutes);

    /// Minutes between `start` and `end` without the pause. An end before the start is the next day's.
    int workedMinutes(QTime start, QTime end, int pauseMinutes);

    /// The pause that is deducted: the fixed one always, and clocked pauses only as far as they go beyond it,
    /// as a time clock like Loga3 counts them.
    int effectivePauseMinutes(int fixedMinutes, int clockedMinutes);

    /// The whole minutes of the finished pauses in `stamps`, which alternate begin, end, begin, ...;
    /// a pause still running at the end is left out.
    int clockedMinutes(const QList<QDateTime>& stamps);

    /// A working-day calculator: start, pause and target length give the end, and the end that is
    /// shown gives the length actually worked. Pauses can be clocked as they happen. The pause and the
    /// target are remembered, and so are today's start and clocked pauses.
    class WorkTime : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(QDateTime start READ start WRITE setStart NOTIFY startChanged)
        Q_PROPERTY(QDateTime end READ end WRITE setEnd NOTIFY endChanged)
        Q_PROPERTY(int pauseMinutes READ pauseMinutes WRITE setPauseMinutes NOTIFY pauseMinutesChanged)
        Q_PROPERTY(int targetMinutes READ targetMinutes WRITE setTargetMinutes NOTIFY targetMinutesChanged)
        Q_PROPERTY(QString resultText READ resultText NOTIFY resultTextChanged)
        Q_PROPERTY(bool paused READ paused NOTIFY pausesChanged)
        Q_PROPERTY(QString pausesText READ pausesText NOTIFY pausesChanged)

    public:
        explicit WorkTime(QObject* parent = nullptr);

        [[nodiscard]] QDateTime start() const;
        [[nodiscard]] QDateTime end() const;
        [[nodiscard]] int pauseMinutes() const;
        [[nodiscard]] int targetMinutes() const;
        [[nodiscard]] QString resultText() const;
        [[nodiscard]] bool paused() const;

        /// "11:00 – 13:00, since 14:10", or "None".
        [[nodiscard]] QString pausesText() const;

        void setStart(const QDateTime& start);
        void setEnd(const QDateTime& end);
        void setPauseMinutes(int minutes);
        void setTargetMinutes(int minutes);

        /// A new working day: it starts now, without clocked pauses.
        Q_INVOKABLE void clockIn();

        /// Begins a pause now, or ends the running one; the end moves by what the pause adds.
        Q_INVOKABLE void togglePause();

        /// The next moment the day ends at the shown time, for the timer to count down to.
        [[nodiscard]] Q_INVOKABLE QDateTime endTarget() const;

        /// When the day that ends at endTarget() began: the length of the whole day (work and pause)
        /// before it, so the progress bar covers the working day and not just the time since the timer ran.
        [[nodiscard]] Q_INVOKABLE QDateTime startTarget() const;

    Q_SIGNALS:
        void startChanged();
        void endChanged();
        void pauseMinutesChanged();
        void targetMinutesChanged();
        void resultTextChanged();
        void pausesChanged();

    private:
        void recalculateEnd();
        void rememberDay() const;
        [[nodiscard]] int pauseToDeduct() const;

        QDateTime m_start;
        QDateTime m_end;
        int m_pauseMinutes = 45;
        int m_targetMinutes = 465;
        QList<QDateTime> m_pauseStamps;
    };
} // namespace IstarCountdown
