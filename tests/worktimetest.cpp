// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "worktime.h"

#include <QStandardPaths>
#include <QTest>

using namespace IstarCountdown;

class WorkTimeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // The start setter keeps today's day in the settings file; not in the real one.
        QStandardPaths::setTestModeEnabled(true);
    }

    void clockedPausesCountOnlyBeyondTheFixedOne()
    {
        const QDate today(2026, 11, 25);
        const QList<QDateTime> twoHours{{today, QTime(11, 0)}, {today, QTime(13, 0)}};

        // The Loga3 day: in at 08:00, 7:45 h, 45 min fixed pause.
        QCOMPARE(workEnd(QTime(8, 0), effectivePauseMinutes(45, clockedMinutes({})), 465), QTime(16, 30));
        QCOMPARE(workEnd(QTime(8, 0), effectivePauseMinutes(45, clockedMinutes(twoHours)), 465), QTime(17, 45));

        // A short clocked pause is inside the fixed one.
        QCOMPARE(effectivePauseMinutes(45, 20), 45);
    }

    void clockedMinutesLeavesOutARunningPause()
    {
        const QDate today(2026, 11, 25);

        QCOMPARE(clockedMinutes({{today, QTime(11, 0)}, {today, QTime(11, 30)}, {today, QTime(14, 0)}}), 30);
        QCOMPARE(clockedMinutes({{today, QTime(23, 30)}, {today.addDays(1), QTime(0, 15)}}), 45);
    }

    void pausesEnteredAfterwardsMoveTheEnd()
    {
        WorkTime day;
        const QDate today = QDate::currentDate();

        day.clockIn();
        day.setStart({today, QTime(8, 0)});
        day.setPauseMinutes(45);
        day.setTargetMinutes(465);
        QCOMPARE(day.end().time(), QTime(16, 30));

        day.addPause();
        day.setPauseStamp(0, {today, QTime(11, 0)});
        day.setPauseStamp(1, {today, QTime(13, 0)});
        QCOMPARE(day.end().time(), QTime(17, 45));

        // An end before its begin while typing counts as no pause, so only the fixed one is left.
        day.setPauseStamp(1, {today, QTime(10, 0)});
        QCOMPARE(day.end().time(), QTime(16, 30));

        day.setPauseStamp(1, {today, QTime(13, 0)});
        day.removePause(0);
        QVERIFY(day.pauseStamps().isEmpty());
        QCOMPARE(day.end().time(), QTime(16, 30));
    }

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
