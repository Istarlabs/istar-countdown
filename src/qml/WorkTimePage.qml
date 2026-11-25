// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import net.kurokyuu.istarcountdown

Kirigami.ScrollablePage {
    id: page

    required property WorkTime workTime
    // The working day's own timer, not the one of the Timer page.
    required property CountdownTimer countdownTimer

    title: i18n("Working Time")

    actions: [
        Kirigami.Action {
            text: i18n("Clock In")
            icon.name: "chronometer-reset"
            onTriggered: page.workTime.clockIn()
        },
        Kirigami.Action {
            text: page.workTime.paused ? i18n("End Pause") : i18n("Begin Pause")
            icon.name: page.workTime.paused ? "media-playback-start" : "media-playback-pause"
            onTriggered: page.workTime.togglePause()
        },
        Kirigami.Action {
            text: i18n("Count Down to the End")
            icon.name: "chronometer-start"
            visible: !page.countdownTimer.running && !page.workTime.paused
            onTriggered: page.countdownTimer.startAt(page.workTime.endTarget(), page.workTime.startTarget())
        },
        Kirigami.Action {
            text: i18n("Stop")
            icon.name: "chronometer-pause"
            visible: page.countdownTimer.running
            onTriggered: page.countdownTimer.stop()
        }
    ]

    // The delegates write back to the model on every edit, which breaks a plain binding; a Binding
    // keeps them in step when the end is recalculated from the start.
    Binding {
        target: startField
        property: "value"
        value: page.workTime.start
    }

    Binding {
        target: endField
        property: "value"
        value: page.workTime.end
    }

    function hoursAndMinutes(minutes) {
        return i18nc("a length of time as hours:minutes", "%1:%2", Math.floor(minutes / 60), String(minutes % 60).padStart(2, "0"));
    }

    ColumnLayout {
        spacing: Kirigami.Units.gridUnit

        GradientText {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 6
            text: page.workTime.paused ? i18n("Pause") : page.countdownTimer.text
            startColor: page.countdownTimer.startColor
            endColor: page.countdownTimer.endColor
            font.pixelSize: Kirigami.Units.gridUnit * 3
            font.bold: true
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            FormCard.FormDateTimeDelegate {
                id: startField

                text: i18n("Working start")
                dateTimeDisplay: FormCard.FormDateTimeDelegate.DateTimeDisplay.Time
                onValueChanged: page.workTime.start = value
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormSpinBoxDelegate {
                label: i18n("Pause (minutes)")
                from: 0
                to: 180
                stepSize: 15
                value: page.workTime.pauseMinutes
                onValueChanged: page.workTime.pauseMinutes = value
            }

            FormCard.FormDelegateSeparator {}

            // The fixed pause is always deducted; clocked pauses only count for what goes beyond it.
            FormCard.FormTextDelegate {
                text: i18n("Clocked pauses")
                description: page.workTime.pausesText
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormSpinBoxDelegate {
                label: i18n("Target time (hours:minutes)")
                from: 0
                to: 600
                stepSize: 15
                value: page.workTime.targetMinutes
                textFromValue: (value, locale) => page.hoursAndMinutes(value)
                onValueChanged: page.workTime.targetMinutes = value
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormDateTimeDelegate {
                id: endField

                text: i18n("Working end")
                dateTimeDisplay: FormCard.FormDateTimeDelegate.DateTimeDisplay.Time
                onValueChanged: page.workTime.end = value
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Result")
                description: page.workTime.resultText
            }
        }
    }
}
