// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#pragma once

#include <QColor>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QtQmlIntegration>

namespace IstarCountdown
{
    /// "03:25:09", or "2 days 03:25:09" from a day up. Negative input reads as zero.
    QString formatRemaining(qint64 seconds);

    /// The big ticking text. Its finished message and gradient are kept in the settings file.
    class CountdownTimer : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(QString text READ text NOTIFY textChanged)
        Q_PROPERTY(bool running READ running NOTIFY runningChanged)
        Q_PROPERTY(QString finishedText READ finishedText WRITE setFinishedText NOTIFY finishedTextChanged)
        Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
        Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)

    public:
        explicit CountdownTimer(QObject* parent = nullptr);

        [[nodiscard]] QString text() const;
        [[nodiscard]] bool running() const;
        [[nodiscard]] QString finishedText() const;
        [[nodiscard]] QColor startColor() const;
        [[nodiscard]] QColor endColor() const;

        /// The share of the running countdown that is left, from 1 at the start down to 0.
        [[nodiscard]] double progress() const;

        void setFinishedText(const QString& text);
        void setStartColor(const QColor& color);
        void setEndColor(const QColor& color);

        /// Counts down to the next occurrence of a saved countdown, and shows its message at the end.
        Q_INVOKABLE void startCountdown(const QDateTime& start, int occurrence, const QString& message);

        /// Counts down to `target`, keeping the finished text as it is. `from` is what the task manager's
        /// progress bar counts as 100 %; without it, the moment of this call is.
        Q_INVOKABLE void startAt(const QDateTime& target, const QDateTime& from = {});

        Q_INVOKABLE void stop();

    Q_SIGNALS:
        void textChanged();
        void runningChanged();
        void finishedTextChanged();
        void startColorChanged();
        void endColorChanged();

        /// A countdown that was running has reached its end; not sent for one that was over when it started.
        void finished();

    private:
        void tick();
        void setText(const QString& text);

        QTimer m_ticker;
        QDateTime m_start;
        QDateTime m_target;
        QString m_text;
        QString m_finishedText;
        QColor m_startColor;
        QColor m_endColor;
    };
} // namespace IstarCountdown
