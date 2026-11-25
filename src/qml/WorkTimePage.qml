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

            TimeField {
                label: i18n("Working start")
                value: page.workTime.start
                onEdited: time => page.workTime.start = time
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

            TimeField {
                label: i18n("Working end")
                value: page.workTime.end
                onEdited: time => page.workTime.end = time
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormTextDelegate {
                text: i18n("Result")
                description: page.workTime.resultText
            }
        }

        // The fixed pause above is always deducted; clocked pauses only count for what goes beyond it.
        FormCard.FormHeader {
            Layout.fillWidth: true
            title: i18n("Clocked pauses")
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            Repeater {
                // The number of pauses, not the stamps: an edit then keeps the delegates, and with them the focus
                // that Tab just moved from a pause's begin to its end.
                model: Math.ceil(page.workTime.pauseStamps.length / 2)

                delegate: ColumnLayout {
                    id: pause

                    required property int index
                    readonly property var begin: page.workTime.pauseStamps[index * 2]
                    // Undefined while the pause is still running.
                    readonly property var end: page.workTime.pauseStamps[index * 2 + 1]

                    Layout.fillWidth: true
                    spacing: 0

                    TimeField {
                        label: i18nc("%1 is the number of the pause", "Pause %1 began", pause.index + 1)
                        value: pause.begin
                        onEdited: time => page.workTime.setPauseStamp(pause.index * 2, time)
                    }

                    TimeField {
                        visible: pause.end !== undefined
                        label: i18nc("%1 is the number of the pause", "Pause %1 ended", pause.index + 1)
                        value: pause.end ?? pause.begin
                        onEdited: time => page.workTime.setPauseStamp(pause.index * 2 + 1, time)
                    }

                    FormCard.FormTextDelegate {
                        visible: pause.end === undefined
                        text: i18nc("%1 is the number of the pause", "Pause %1 ended", pause.index + 1)
                        description: i18n("Still running")
                    }

                    FormCard.FormButtonDelegate {
                        text: i18nc("%1 is the number of the pause", "Remove Pause %1", pause.index + 1)
                        icon.name: "edit-delete"
                        onClicked: page.workTime.removePause(pause.index)
                    }

                    FormCard.FormDelegateSeparator {}
                }
            }

            FormCard.FormButtonDelegate {
                text: i18n("Add Pause")
                icon.name: "list-add"
                onClicked: page.workTime.addPause()
            }
        }
    }
}
