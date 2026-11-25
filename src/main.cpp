// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

#include "countdowntimer.h"

#include <KAboutData>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>

#ifndef Q_OS_ANDROID
#include <KDBusService>
#include <KNotification>
#include <KStatusNotifierItem>
#include <KWindowSystem>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QPainter>
#include <QPixmap>

#include <memory>
#endif

using namespace Qt::StringLiterals;

#ifndef Q_OS_ANDROID
namespace
{
    /// A circle in `color`, filled clockwise from twelve o'clock with the share `left` (1 to 0) of the time.
    QIcon remainingIcon(double left, const QColor& color)
    {
        constexpr int Size = 64;
        constexpr int Margin = 4;

        QPixmap pixmap(Size, Size);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRectF circle(Margin, Margin, Size - (2 * Margin), Size - (2 * Margin));
        QColor faint = color;

        faint.setAlphaF(0.25);
        painter.setPen(QPen(color, Margin));
        painter.setBrush(faint);
        painter.drawEllipse(circle);

        // Angles are in 1/16 degree, counter-clockwise from three o'clock; a negative span runs clockwise.
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPie(circle, 90 * 16, -qRound(left * 360 * 16));
        painter.end();

        return QIcon(pixmap);
    }

    /// Sets the progress bar on this program's task manager entry. Plasma listens for the Unity launcher
    /// API, so this works minimized too and needs no KDE library.
    void publishProgress(double progress, bool visible)
    {
        QDBusMessage message = QDBusMessage::createSignal(u"/net/kurokyuu/istar_countdown"_s,
                                                          u"com.canonical.Unity.LauncherEntry"_s, u"Update"_s);

        message << u"application://net.kurokyuu.istar-countdown.desktop"_s
                << QVariantMap{{u"progress"_s, progress}, {u"progress-visible"_s, visible}};
        QDBusConnection::sessionBus().send(message);
    }

    /// The tray icon: closing the window leaves the program running there, a click brings the window back,
    /// and while a countdown runs the icon and the task manager entry show how much of it is left. There is
    /// one of each for both timers, so the working-time countdown has them while it runs: it is the one
    /// that runs all day.
    void addTrayIcon(QQuickWindow* window, const IstarCountdown::CountdownTimer* mainTimer,
                     const IstarCountdown::CountdownTimer* workTimer)
    {
        auto* tray = new KStatusNotifierItem(window);

        tray->setTitle(i18n("Istar Countdown"));
        tray->setCategory(KStatusNotifierItem::ApplicationStatus);
        tray->setStatus(KStatusNotifierItem::Active);
        tray->setAssociatedWindow(window);
        tray->setIconByName(u"chronometer"_s);
        tray->setToolTip(u"chronometer"_s, i18n("Istar Countdown"), {});

        // ponytail: without a tray host (no Plasma, no SNI watcher) a closed window leaves a process that only a
        // second launch brings back; ask the tray whether it is shown if that ever comes up.
        QGuiApplication::setQuitOnLastWindowClosed(false);

        if (mainTimer == nullptr || workTimer == nullptr)
        {
            return;
        }

        // The icon goes over D-Bus as a pixmap, so it is redrawn per whole percent, not every second.
        auto shownPercent = std::make_shared<int>(-1);
        const auto refresh = [tray, mainTimer, workTimer, shownPercent]
        {
            const auto* timer = workTimer->running() ? workTimer : mainTimer;

            if (!timer->running())
            {
                *shownPercent = -1;
                tray->setIconByName(u"chronometer"_s);
                tray->setToolTip(u"chronometer"_s, i18n("Istar Countdown"), {});
                publishProgress(0, false);
                return;
            }

            const double left = timer->progress();

            if (const int percent = qRound(left * 100); percent != *shownPercent)
            {
                *shownPercent = percent;
                tray->setIconByPixmap(remainingIcon(left, timer->startColor()));
            }

            tray->setToolTip(u"chronometer"_s, i18n("Istar Countdown"), timer->text());
            publishProgress(left, true);
        };

        for (const auto* timer : {mainTimer, workTimer})
        {
            QObject::connect(timer, &IstarCountdown::CountdownTimer::textChanged, tray, refresh);
            QObject::connect(timer, &IstarCountdown::CountdownTimer::runningChanged, tray, refresh);
        }
    }

    /// A notification when a countdown is over; clicking it brings the window back, from the tray too.
    void notifyWhenFinished(QQuickWindow* window, const IstarCountdown::CountdownTimer* timer)
    {
        QObject::connect(timer, &IstarCountdown::CountdownTimer::finished, window,
                         [window, timer]
                         {
                             // Deletes itself once it is closed.
                             auto* notification = new KNotification(u"countdownFinished"_s);

                             notification->setTitle(i18n("Countdown finished"));
                             notification->setText(timer->finishedText());
                             notification->setIconName(u"chronometer"_s);

                             const KNotificationAction* show = notification->addDefaultAction(i18n("Show"));

                             QObject::connect(show, &KNotificationAction::activated, window,
                                              [window, notification]
                                              {
                                                  // On Wayland, raising needs the token the click hands over.
                                                  KWindowSystem::setCurrentXdgActivationToken(
                                                      notification->xdgActivationToken());
                                                  window->show();
                                                  KWindowSystem::activateWindow(window);
                                              });
                             notification->sendEvent();
                         });
    }
} // namespace

// The tray icon's context menu is a QMenu, so the desktop needs the widgets application.
using Application = QApplication;
#else
using Application = QGuiApplication;
#endif

int main(int argc, char* argv[])
{
    const Application app(argc, argv);

    // The domain the .mo files are installed under; it has to match i18n.domain in Scripts/project.toml,
    // and it has to be set before the first i18n() call - the about data below makes one.
    KLocalizedString::setApplicationDomain("istar-countdown");

    KAboutData about(u"istar-countdown"_s, i18n("Istar Countdown"), QStringLiteral(ISTAR_COUNTDOWN_VERSION),
                     i18n("Countdowns, a timer and a working-time calculator"), KAboutLicense::MIT,
                     u"© 2026 Pascal Klose"_s);
    about.setDesktopFileName(u"net.kurokyuu.istar-countdown"_s);
    about.setOrganizationDomain("kurokyuu.net");
    about.addAuthor(u"Pascal Klose"_s, {}, u"istarlabs_dev@proton.me"_s,
                    u"https://github.com/Istarlabs/istar-countdown"_s);
    about.setBugAddress(QByteArrayLiteral("https://github.com/Istarlabs/istar-countdown/issues"));
    KAboutData::setApplicationData(about);

#ifndef Q_OS_ANDROID
    // One window per session: a second launch exits right here and brings the running window back instead.
    const KDBusService service(KDBusService::Unique);
#endif

#ifdef Q_OS_ANDROID
    // Android takes the launcher icon from the APK. org.kde.desktop does not exist there; the Breeze
    // style is what the KDE apps on Android use, and it follows the system's light and dark setting -
    // without it Kirigami falls back to its built-in light colours whatever the phone is set to.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE"))
    {
        QQuickStyle::setStyle(u"org.kde.breeze"_s);
    }
#else
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"chronometer"_s));

    // Without this a session that is not Plasma gets Fusion; with it Kirigami's controls look like
    // the rest of the desktop either way.
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE"))
    {
        QQuickStyle::setStyle(u"org.kde.desktop"_s);
    }
#endif

    // Vulkan instead of the OpenGL default; has to be set before the first window exists.
    // QSG_RHI_BACKEND in the environment still wins, e.g. for a machine without a Vulkan driver.
    if (qEnvironmentVariableIsEmpty("QSG_RHI_BACKEND"))
    {
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);
    }

    QQmlApplicationEngine engine;
    auto* localized = new KLocalizedQmlContext(&engine);
    engine.rootContext()->setContextObject(localized);
    QQmlEngine::setContextForObject(localized, engine.rootContext());
    engine.loadFromModule("net.kurokyuu.istarcountdown", "Main");

    if (engine.rootObjects().isEmpty())
    {
        return 1;
    }

#ifndef Q_OS_ANDROID
    if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first()))
    {
        const auto* mainTimer = window->findChild<IstarCountdown::CountdownTimer*>(u"mainTimer"_s);
        const auto* workTimer = window->findChild<IstarCountdown::CountdownTimer*>(u"workTimer"_s);

        addTrayIcon(window, mainTimer, workTimer);

        for (const auto* timer : {mainTimer, workTimer})
        {
            if (timer != nullptr)
            {
                notifyWhenFinished(window, timer);
            }
        }

        QObject::connect(&service, &KDBusService::activateRequested, window,
                         [window]
                         {
                             // On Wayland, raising needs the activation token the second launch passed along.
                             window->show();
                             KWindowSystem::updateStartupId(window);
                             KWindowSystem::activateWindow(window);
                         });
    }
#endif

    return QGuiApplication::exec();
}
