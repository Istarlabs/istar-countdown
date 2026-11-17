// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#pragma once

#include <QDateTime>
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

    /// A working-day calculator: start, pause and target length give the end, and the end that is
    /// shown gives the length actually worked. Only the pause and the target are remembered.
    class WorkTime : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(QDateTime start READ start WRITE setStart NOTIFY startChanged)
        Q_PROPERTY(QDateTime end READ end WRITE setEnd NOTIFY endChanged)
        Q_PROPERTY(int pauseMinutes READ pauseMinutes WRITE setPauseMinutes NOTIFY pauseMinutesChanged)
        Q_PROPERTY(int targetMinutes READ targetMinutes WRITE setTargetMinutes NOTIFY targetMinutesChanged)
        Q_PROPERTY(QString resultText READ resultText NOTIFY resultTextChanged)

    public:
        explicit WorkTime(QObject* parent = nullptr);

        [[nodiscard]] QDateTime start() const;
        [[nodiscard]] QDateTime end() const;
        [[nodiscard]] int pauseMinutes() const;
        [[nodiscard]] int targetMinutes() const;
        [[nodiscard]] QString resultText() const;

        void setStart(const QDateTime& start);
        void setEnd(const QDateTime& end);
        void setPauseMinutes(int minutes);
        void setTargetMinutes(int minutes);

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

    private:
        void recalculateEnd();

        QDateTime m_start;
        QDateTime m_end;
        int m_pauseMinutes = 45;
        int m_targetMinutes = 465;
    };
} // namespace IstarCountdown
