// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "countdowntimer.h"

#include <QTest>

using namespace IstarCountdown;
using namespace Qt::StringLiterals;

class CountdownTimerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void format_data()
    {
        QTest::addColumn<qint64>("seconds");
        QTest::addColumn<QString>("expected");

        QTest::newRow("zero") << qint64{0} << u"00:00:00"_s;
        QTest::newRow("negative reads as zero") << qint64{-5} << u"00:00:00"_s;
        QTest::newRow("clock only") << qint64{(3 * 3600) + (25 * 60) + 9} << u"03:25:09"_s;
        QTest::newRow("just under a day") << qint64{86399} << u"23:59:59"_s;
        QTest::newRow("one day") << qint64{86400 + 61} << u"1 day 00:01:01"_s;
        QTest::newRow("several days") << qint64{(3 * 86400) + 3661} << u"3 days 01:01:01"_s;
    }

    void format()
    {
        QFETCH(qint64, seconds);
        QFETCH(QString, expected);

        QCOMPARE(formatRemaining(seconds), expected);
    }

    void progressStartsFullAndStopsAtZero()
    {
        CountdownTimer timer;

        QCOMPARE(timer.progress(), 0.0);

        timer.startAt(QDateTime::currentDateTime().addSecs(100));
        QVERIFY(timer.progress() > 0.99);

        // A day that began before the timer was started: halfway through it.
        const QDateTime now = QDateTime::currentDateTime();

        timer.startAt(now.addSecs(100), now.addSecs(-100));
        QVERIFY(qAbs(timer.progress() - 0.5) < 0.01);

        timer.startAt(QDateTime::currentDateTime().addSecs(-1));
        QCOMPARE(timer.progress(), 0.0);
        QVERIFY(!timer.running());
    }
};

QTEST_APPLESS_MAIN(CountdownTimerTest)
#include "countdowntimertest.moc"
