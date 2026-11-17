// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "countdownmodel.h"

#include <QTemporaryDir>
#include <QTest>

using namespace IstarCountdown;
using namespace Qt::StringLiterals;

class CountdownModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void survivesARestart()
    {
        const QTemporaryDir directory;
        const QString file = directory.filePath(u"sub/countdowns.json"_s);
        const QDateTime when(QDate(2026, 12, 24), QTime(18, 30));

        {
            CountdownModel model(file);
            model.add(u"Dinner"_s, when, static_cast<int>(Occurrence::EveryYear), u"Eat!"_s);
            model.add(u"Gone"_s, when, 0, QString());
            model.remove(1);
            model.update(0, u"Dinner 2"_s, when.addDays(1), static_cast<int>(Occurrence::Birthday), u"Yum"_s);
        }

        const CountdownModel reloaded(file);

        QCOMPARE(reloaded.rowCount(), 1);
        QCOMPARE(reloaded.data(reloaded.index(0), CountdownModel::NameRole).toString(), u"Dinner 2"_s);
        QCOMPARE(reloaded.data(reloaded.index(0), CountdownModel::DateTimeRole).toDateTime(), when.addDays(1));
        QCOMPARE(reloaded.data(reloaded.index(0), CountdownModel::OccurrenceRole).toInt(),
                 static_cast<int>(Occurrence::Birthday));
        QCOMPARE(reloaded.data(reloaded.index(0), CountdownModel::MessageRole).toString(), u"Yum"_s);
    }

    void unknownOccurrenceIsAOneOff()
    {
        const QTemporaryDir directory;
        CountdownModel model(directory.filePath(u"c.json"_s));

        model.add(u"x"_s, QDateTime::currentDateTime(), 99, QString());

        QCOMPARE(model.data(model.index(0), CountdownModel::OccurrenceRole).toInt(), 0);
    }

    void secondsAreDropped()
    {
        const QTemporaryDir directory;
        const QString file = directory.filePath(u"c.json"_s);
        const QDateTime withSeconds(QDate(2026, 12, 24), QTime(18, 30, 49, 123));
        const QDateTime wholeMinute(QDate(2026, 12, 24), QTime(18, 30));

        {
            CountdownModel model(file);
            model.add(u"x"_s, withSeconds, 0, QString());
            model.add(u"y"_s, wholeMinute.addDays(1), 0, QString());
            model.update(1, u"y"_s, withSeconds.addDays(1), 0, QString());

            QCOMPARE(model.data(model.index(0), CountdownModel::DateTimeRole).toDateTime(), wholeMinute);
            QCOMPARE(model.data(model.index(1), CountdownModel::DateTimeRole).toDateTime(), wholeMinute.addDays(1));
        }

        // What was saved with seconds before this, is read back without them.
        const CountdownModel reloaded(file);

        QCOMPARE(reloaded.data(reloaded.index(0), CountdownModel::DateTimeRole).toDateTime(), wholeMinute);
    }

    void badRowsAreIgnored()
    {
        const QTemporaryDir directory;
        CountdownModel model(directory.filePath(u"c.json"_s));

        model.remove(3);
        model.update(-1, QString(), QDateTime(), 0, QString());

        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_GUILESS_MAIN(CountdownModelTest)
#include "countdownmodeltest.moc"
