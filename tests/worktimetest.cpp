// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "worktime.h"

#include <QTest>

using namespace IstarCountdown;

class WorkTimeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void end()
    {
        // The WinUI defaults: a 45 minute pause and 7.75 hours.
        QCOMPARE(workEnd(QTime(8, 0), 45, 465), QTime(16, 30));
        QCOMPARE(workEnd(QTime(17, 0), 30, 480), QTime(1, 30));
    }

    void worked()
    {
        QCOMPARE(workedMinutes(QTime(8, 0), QTime(16, 0), 45), 435);
        QCOMPARE(workedMinutes(QTime(22, 0), QTime(6, 0), 30), 450);
        QCOMPARE(workedMinutes(QTime(8, 0), QTime(8, 10), 45), 0);
    }

    void startTargetIsTheStartOfTheDayThatEndsAtEndTarget()
    {
        // The setters used here do not touch the settings file; the pause and target would.
        WorkTime day;
        const QDate today = QDate::currentDate();

        day.setStart({today, QTime(7, 21)});
        day.setEnd({today, QTime(15, 21)});
        QCOMPARE(day.startTarget(), QDateTime(today, QTime(7, 21)));

        day.setStart({today, QTime(22, 0)});
        day.setEnd({today, QTime(6, 0)});
        QCOMPARE(day.startTarget(), QDateTime(today, QTime(22, 0)));
    }
};

QTEST_APPLESS_MAIN(WorkTimeTest)
#include "worktimetest.moc"
