// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "countdownmodel.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <utility>

using namespace Qt::StringLiterals;

using namespace IstarCountdown;

namespace
{
    QString defaultFile()
    {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/countdowns.json"_s;
    }

    /// Anything outside the enum - a hand-edited file, a newer version's value - reads as a one-off.
    Occurrence occurrenceFrom(int value)
    {
        return value >= 0 && value < OccurrenceCount ? static_cast<Occurrence>(value) : Occurrence::Once;
    }
} // namespace

namespace IstarCountdown
{
    CountdownModel::CountdownModel(QObject* parent) : CountdownModel(defaultFile(), parent)
    {
    }

    CountdownModel::CountdownModel(QString file, QObject* parent) : QAbstractListModel(parent), m_file(std::move(file))
    {
        load();
    }

    int CountdownModel::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(m_entries.size());
    }

    QVariant CountdownModel::data(const QModelIndex& index, int role) const
    {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        {
            return {};
        }

        const Entry& entry = m_entries.at(index.row());

        switch (role)
        {
            case NameRole:
                return entry.name;
            case DateTimeRole:
                return entry.dateTime;
            case OccurrenceRole:
                return static_cast<int>(entry.occurrence);
            case OccurrenceTextRole:
                return occurrenceLabel(entry.occurrence);
            case IconRole:
                return occurrenceIcon(entry.occurrence);
            case MessageRole:
                return entry.message;
            default:
                return {};
        }
    }

    QHash<int, QByteArray> CountdownModel::roleNames() const
    {
        return {
            {NameRole, "name"},
            {DateTimeRole, "dateTime"},
            {OccurrenceRole, "occurrence"},
            {OccurrenceTextRole, "occurrenceText"},
            {IconRole, "iconName"},
            {MessageRole, "message"},
        };
    }

    QStringList CountdownModel::occurrenceLabels() const
    {
        QStringList labels;

        for (int i = 0; i < OccurrenceCount; ++i)
        {
            labels.append(occurrenceLabel(static_cast<Occurrence>(i)));
        }
        return labels;
    }

    CountdownModel::Entry CountdownModel::makeEntry(const QString& name, const QDateTime& dateTime, int occurrence,
                                                    const QString& message)
    {
        // The pickers have no seconds, so a countdown is to a whole minute: the seconds of the moment it
        // was entered are not part of what anyone chose, and would show up in the list and the clock.
        QDateTime wholeMinute = dateTime;

        if (wholeMinute.isValid())
        {
            wholeMinute.setTime(QTime(dateTime.time().hour(), dateTime.time().minute()));
        }

        return {.name = name, .dateTime = wholeMinute, .occurrence = occurrenceFrom(occurrence), .message = message};
    }

    void CountdownModel::add(const QString& name, const QDateTime& dateTime, int occurrence, const QString& message)
    {
        const int row = static_cast<int>(m_entries.size());

        beginInsertRows({}, row, row);
        m_entries.append(makeEntry(name, dateTime, occurrence, message));
        endInsertRows();
        save();
    }

    void CountdownModel::update(int row, const QString& name, const QDateTime& dateTime, int occurrence,
                                const QString& message)
    {
        if (row < 0 || row >= m_entries.size())
        {
            return;
        }

        m_entries.replace(row, makeEntry(name, dateTime, occurrence, message));
        Q_EMIT dataChanged(index(row), index(row));
        save();
    }

    void CountdownModel::remove(int row)
    {
        if (row < 0 || row >= m_entries.size())
        {
            return;
        }

        beginRemoveRows({}, row, row);
        m_entries.removeAt(row);
        endRemoveRows();
        save();
    }

    void CountdownModel::load()
    {
        QFile file(m_file);

        if (!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        // A file that does not parse is left alone and treated as empty; the next change overwrites it,
        // which loses a broken file rather than a working one.
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());

        for (const QJsonValue value : document.array())
        {
            const QJsonObject object = value.toObject();

            m_entries.append(makeEntry(object.value(u"name"_s).toString(),
                                       QDateTime::fromString(object.value(u"dateTime"_s).toString(), Qt::ISODate),
                                       object.value(u"occurrence"_s).toInt(), object.value(u"message"_s).toString()));
        }
    }

    void CountdownModel::save() const
    {
        QJsonArray array;

        for (const Entry& entry : m_entries)
        {
            array.append(QJsonObject{
                {u"name"_s, entry.name},
                {u"dateTime"_s, entry.dateTime.toString(Qt::ISODate)},
                {u"occurrence"_s, static_cast<int>(entry.occurrence)},
                {u"message"_s, entry.message},
            });
        }

        QDir().mkpath(QFileInfo(m_file).absolutePath());

        QSaveFile file(m_file);

        if (file.open(QIODevice::WriteOnly))
        {
            file.write(QJsonDocument(array).toJson());
            file.commit();
        }
    }
} // namespace IstarCountdown
