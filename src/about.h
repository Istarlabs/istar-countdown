// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#pragma once

#include <KAboutData>

#include <QObject>
#include <QtQmlIntegration>

namespace IstarCountdown
{
    /// The application's KAboutData for the about page. A singleton of the module rather than a
    /// qmlRegisterSingletonType() call: registering by hand under the module's own URI makes Qt treat
    /// the module as hand-registered and its other types are no longer found.
    class About : public QObject
    {
        Q_OBJECT
        QML_ELEMENT
        QML_SINGLETON

        Q_PROPERTY(KAboutData data READ data CONSTANT)

    public:
        explicit About(QObject* parent = nullptr) : QObject(parent)
        {
        }

        [[nodiscard]] static KAboutData data()
        {
            return KAboutData::applicationData();
        }
    };
} // namespace IstarCountdown
