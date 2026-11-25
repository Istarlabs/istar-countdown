// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Dialogs // FormColorDelegate needs it; on Android only a module named in our own QML is packed into the APK
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import net.kurokyuu.istarcountdown

Kirigami.ScrollablePage {
    id: page

    required property CountdownTimer countdownTimer

    title: i18n("Timer")

    actions: [
        Kirigami.Action {
            text: i18n("Stop")
            icon.name: "chronometer-pause"
            visible: page.countdownTimer.running
            onTriggered: page.countdownTimer.stop()
        }
    ]

    // Today at the chosen hour and minute; a time that has already passed today means tomorrow.
    function countDownUntil(time) {
        const now = new Date();
        const target = new Date(now.getFullYear(), now.getMonth(), now.getDate(), time.getHours(), time.getMinutes());

        if (target <= now) {
            target.setDate(target.getDate() + 1);
        }

        page.countdownTimer.startAt(target);
    }

    ColumnLayout {
        spacing: Kirigami.Units.gridUnit

        GradientText {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 8
            text: page.countdownTimer.text
            startColor: page.countdownTimer.startColor
            endColor: page.countdownTimer.endColor
            font.pixelSize: Kirigami.Units.gridUnit * 4
            font.bold: true
        }

        // A quick countdown without saving one first.
        FormCard.FormCard {
            Layout.fillWidth: true

            TimeField {
                id: untilField

                label: i18n("Count down until")
                value: new Date()
                onEdited: time => value = time
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormButtonDelegate {
                text: i18n("Start")
                icon.name: "chronometer-start"
                onClicked: page.countDownUntil(untilField.value)
            }
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            FormCard.FormTextFieldDelegate {
                label: i18n("Message when it is over")
                text: page.countdownTimer.finishedText
                onTextChanged: page.countdownTimer.finishedText = text
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormColorDelegate {
                text: i18n("Gradient start")
                color: page.countdownTimer.startColor
                onColorChanged: page.countdownTimer.startColor = color
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormColorDelegate {
                text: i18n("Gradient end")
                color: page.countdownTimer.endColor
                onColorChanged: page.countdownTimer.endColor = color
            }
        }
    }
}
