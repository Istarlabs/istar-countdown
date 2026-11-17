// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#pragma once

#include "occurrence.h"

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QStringList>
#include <QtQmlIntegration>

namespace IstarCountdown
{
    /// The user's countdowns, saved to countdowns.json after every change.
    class CountdownModel : public QAbstractListModel
    {
        Q_OBJECT
        QML_ELEMENT

        /// Indexed by the `occurrence` role and by the `occurrence` argument of add() and update().
        Q_PROPERTY(QStringList occurrenceLabels READ occurrenceLabels CONSTANT)

    public:
        enum Role
        {
            NameRole = Qt::UserRole + 1,
            DateTimeRole,
            OccurrenceRole,
            OccurrenceTextRole,
            IconRole,
            MessageRole,
        };

        explicit CountdownModel(QObject* parent = nullptr);
        explicit CountdownModel(QString file, QObject* parent = nullptr);

        [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
        [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

        [[nodiscard]] QStringList occurrenceLabels() const;

        Q_INVOKABLE void add(const QString& name, const QDateTime& dateTime, int occurrence, const QString& message);
        Q_INVOKABLE void update(int row, const QString& name, const QDateTime& dateTime, int occurrence,
                                const QString& message);
        Q_INVOKABLE void remove(int row);

    private:
        struct Entry
        {
            QString name;
            QDateTime dateTime;
            Occurrence occurrence = Occurrence::Once;
            QString message;
        };

        static Entry makeEntry(const QString& name, const QDateTime& dateTime, int occurrence, const QString& message);
        void load();
        void save() const;

        QString m_file;
        QList<Entry> m_entries;
    };
} // namespace IstarCountdown
