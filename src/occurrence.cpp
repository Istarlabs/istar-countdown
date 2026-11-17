// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "occurrence.h"

#include <KLocalizedString>

using IstarCountdown::Occurrence;

namespace
{
    /// `at(n)` is the n-th repetition counted from the start, always computed from the start rather
    /// than from the previous one - January 31st plus one month is February 28th, plus two months
    /// March 31st, not March 28th. `estimate` is never above the answer, so only a step or two is left.
    template <typename At>
    QDateTime firstAfter(const QDateTime& now, qint64 estimate, At at)
    {
        qint64 n = qMax<qint64>(estimate, 0);

        while (at(n) <= now)
        {
            ++n;
        }
        return at(n);
    }

    qint64 monthsBetween(const QDate& from, const QDate& to)
    {
        return ((to.year() - from.year()) * 12LL) + (to.month() - from.month());
    }
} // namespace

namespace IstarCountdown
{
    QDateTime nextOccurrence(const QDateTime& start, Occurrence occurrence, const QDateTime& now)
    {
        if (occurrence == Occurrence::Once || start > now)
        {
            return start;
        }

        switch (occurrence)
        {
            case Occurrence::EveryWeek:
            case Occurrence::EverySecondWeek:
            {
                const int days = occurrence == Occurrence::EveryWeek ? 7 : 14;
                return firstAfter(now, start.daysTo(now) / days, [&](qint64 n) { return start.addDays(n * days); });
            }
            case Occurrence::EveryMonth:
                return firstAfter(now, monthsBetween(start.date(), now.date()),
                                  [&](qint64 n) { return start.addMonths(static_cast<int>(n)); });
            case Occurrence::EveryYear:
            case Occurrence::Birthday:
                return firstAfter(now, now.date().year() - start.date().year(),
                                  [&](qint64 n) { return start.addYears(static_cast<int>(n)); });
            case Occurrence::Once:
                break;
        }
        return start;
    }

    QString occurrenceLabel(Occurrence occurrence)
    {
        switch (occurrence)
        {
            case Occurrence::Once:
                return i18n("Once");
            case Occurrence::EveryWeek:
                return i18n("Every week");
            case Occurrence::EverySecondWeek:
                return i18n("Every second week");
            case Occurrence::EveryMonth:
                return i18n("Every month");
            case Occurrence::EveryYear:
                return i18n("Every year");
            case Occurrence::Birthday:
                return i18n("Birthday");
        }
        return {};
    }

    QString occurrenceIcon(Occurrence occurrence)
    {
        switch (occurrence)
        {
            case Occurrence::Once:
                return QStringLiteral("view-calendar-day");
            case Occurrence::EveryWeek:
            case Occurrence::EverySecondWeek:
                return QStringLiteral("view-calendar-week");
            case Occurrence::EveryMonth:
                return QStringLiteral("view-calendar-month");
            case Occurrence::EveryYear:
                return QStringLiteral("view-calendar");
            case Occurrence::Birthday:
                return QStringLiteral("view-calendar-birthday");
        }
        return {};
    }
} // namespace IstarCountdown
