// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#pragma once

#include <QDateTime>
#include <QString>

namespace IstarCountdown
{
    /// The order is the order of the combo box in the dialog and of the values in countdowns.json.
    enum class Occurrence
    {
        Once,
        EveryWeek,
        EverySecondWeek,
        EveryMonth,
        EveryYear,
        Birthday,
    };

    inline constexpr int OccurrenceCount = 6;

    /// The first moment of the series that starts at `start` and lies after `now`. A series that has
    /// not begun yet, and a one-off, return `start` itself - for the latter that may be in the past,
    /// which is how a countdown that is already over is told apart from one that repeats.
    QDateTime nextOccurrence(const QDateTime& start, Occurrence occurrence, const QDateTime& now);

    QString occurrenceLabel(Occurrence occurrence);

    /// A Breeze icon name.
    QString occurrenceIcon(Occurrence occurrence);
} // namespace IstarCountdown
