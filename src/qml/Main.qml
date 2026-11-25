// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import org.kde.kirigami as Kirigami
import net.kurokyuu.istarcountdown

Kirigami.ApplicationWindow {
    id: root

    property string currentPage: ""

    title: i18n("Istar Countdown")
    width: Kirigami.Units.gridUnit * 48
    height: Kirigami.Units.gridUnit * 32
    minimumWidth: Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Units.gridUnit * 20

    // They live here, not in the pages, so that the timer keeps ticking while another page is shown.
    CountdownModel {
        id: allCountdowns
    }

    CountdownTimer {
        id: mainTimer
        // main.cpp finds it by this name for the tray icon.
        objectName: "mainTimer"
    }

    // The working day's own countdown, so counting down to its end leaves a running event countdown alone.
    CountdownTimer {
        id: workTimer
        // main.cpp finds it by this name for the tray icon.
        objectName: "workTimer"
        settingsGroup: "WorkTimer"
    }

    WorkTime {
        id: mainWorkTime

        // Kept here, not on the page, so it holds while another page is shown.
        property bool resumeAfterPause: false

        // The countdown holds during a pause and goes on to the end the pause has moved.
        onPausedChanged: {
            if (paused) {
                resumeAfterPause = workTimer.running;
                workTimer.stop();
            } else if (resumeAfterPause) {
                resumeAfterPause = false;
                workTimer.startAt(endTarget(), startTarget());
            }
        }

        // A changed start, pause or target moves a countdown that is already running.
        onEndChanged: {
            if (workTimer.running) {
                workTimer.startAt(endTarget(), startTarget());
            }
        }
    }

    function showCountdowns() {
        currentPage = "countdowns";
        pageStack.clear();
        const page = pageStack.push(Qt.resolvedUrl("CountdownsPage.qml"), {
            countdowns: allCountdowns,
            countdownTimer: mainTimer
        });
        page.timerStarted.connect(showTimer);
    }

    function showTimer() {
        currentPage = "timer";
        pageStack.clear();
        pageStack.push(Qt.resolvedUrl("TimerPage.qml"), {
            countdownTimer: mainTimer
        });
    }

    function showWorkTime() {
        currentPage = "worktime";
        pageStack.clear();
        pageStack.push(Qt.resolvedUrl("WorkTimePage.qml"), {
            workTime: mainWorkTime,
            countdownTimer: workTimer
        });
    }

    // A page of the stack like the others, not a dialog layer: a layer covers the drawer, so on a phone
    // there was no way off it but the Back button, which closes the app.
    function showAbout() {
        currentPage = "about";
        pageStack.clear();
        pageStack.push(Qt.resolvedUrl("AboutAppPage.qml"));
    }

    globalDrawer: Kirigami.GlobalDrawer {
        isMenu: false
        actions: [
            Kirigami.Action {
                text: i18n("Countdowns")
                icon.name: "view-calendar"
                checked: root.currentPage === "countdowns"
                onTriggered: root.showCountdowns()
            },
            Kirigami.Action {
                text: i18n("Timer")
                icon.name: "chronometer"
                checked: root.currentPage === "timer"
                onTriggered: root.showTimer()
            },
            Kirigami.Action {
                text: i18n("Working Time")
                icon.name: "appointment-new"
                checked: root.currentPage === "worktime"
                onTriggered: root.showWorkTime()
            },
            Kirigami.Action {
                text: i18n("About Istar Countdown")
                icon.name: "help-about"
                checked: root.currentPage === "about"
                onTriggered: root.showAbout()
            }
        ]
    }

    pageStack.columnView.columnResizeMode: Kirigami.ColumnView.SingleColumn

    Component.onCompleted: showCountdowns()
}
