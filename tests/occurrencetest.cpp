// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "occurrence.h"

#include <QTest>

using namespace IstarCountdown;

namespace
{
    QDateTime at(int year, int month, int day, int hour = 20, int minute = 0)
    {
        return {QDate(year, month, day), QTime(hour, minute)};
    }
} // namespace

class OccurrenceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void next_data()
    {
        QTest::addColumn<QDateTime>("start");
        QTest::addColumn<int>("occurrence");
        QTest::addColumn<QDateTime>("now");
        QTest::addColumn<QDateTime>("expected");

        const auto row = [](const char* name, const QDateTime& start, Occurrence occurrence, const QDateTime& now,
                            const QDateTime& expected)
        { QTest::newRow(name) << start << static_cast<int>(occurrence) << now << expected; };

        row("once, still ahead", at(2030, 1, 1), Occurrence::Once, at(2026, 5, 5), at(2030, 1, 1));
        row("once, already over stays over", at(2020, 1, 1), Occurrence::Once, at(2026, 5, 5), at(2020, 1, 1));
        row("series that has not begun", at(2030, 1, 1), Occurrence::EveryWeek, at(2026, 5, 5), at(2030, 1, 1));

        // 2026-05-05 is a Tuesday; the series starts on Monday 2026-01-05.
        row("weekly, later this week", at(2026, 1, 5), Occurrence::EveryWeek, at(2026, 5, 5), at(2026, 5, 11));
        row("weekly, same day before the hour", at(2026, 1, 6), Occurrence::EveryWeek, at(2026, 5, 5, 10),
            at(2026, 5, 5));
        row("weekly, same day after the hour", at(2026, 1, 6), Occurrence::EveryWeek, at(2026, 5, 5, 21),
            at(2026, 5, 12));
        row("every second week keeps its rhythm", at(2026, 1, 5), Occurrence::EverySecondWeek, at(2026, 1, 20),
            at(2026, 2, 2));

        row("monthly", at(2026, 1, 10), Occurrence::EveryMonth, at(2026, 5, 20), at(2026, 6, 10));
        row("monthly across a year", at(2025, 11, 10), Occurrence::EveryMonth, at(2025, 12, 20), at(2026, 1, 10));
        row("monthly on the 31st is clamped, not drifted", at(2026, 1, 31), Occurrence::EveryMonth, at(2026, 2, 28, 21),
            at(2026, 3, 31));

        row("yearly", at(2020, 3, 1), Occurrence::EveryYear, at(2026, 5, 5), at(2027, 3, 1));
        row("birthday this year still ahead", at(1991, 12, 24), Occurrence::Birthday, at(2026, 5, 5), at(2026, 12, 24));
        row("birthday on a leap day", at(2024, 2, 29), Occurrence::Birthday, at(2026, 5, 5), at(2027, 2, 28));
    }

    void next()
    {
        QFETCH(QDateTime, start);
        QFETCH(int, occurrence);
        QFETCH(QDateTime, now);
        QFETCH(QDateTime, expected);

        QCOMPARE(nextOccurrence(start, static_cast<Occurrence>(occurrence), now), expected);
    }
};

QTEST_APPLESS_MAIN(OccurrenceTest)
#include "occurrencetest.moc"
