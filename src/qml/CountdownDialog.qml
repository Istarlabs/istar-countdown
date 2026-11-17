// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigamiaddons.formcard as FormCard
import net.kurokyuu.istarcountdown

FormCard.FormCardDialog {
    id: dialog

    required property CountdownModel countdowns

    // -1 while a new countdown is being entered.
    property int row: -1

    function openNew() {
        row = -1;
        nameField.text = "";
        dateField.value = new Date();
        occurrenceField.currentIndex = 0;
        messageField.text = i18n("Let's go!");
        open();
    }

    function openEdit(index, name, dateTime, occurrence, message) {
        row = index;
        nameField.text = name;
        dateField.value = dateTime;
        occurrenceField.currentIndex = occurrence;
        messageField.text = message;
        open();
    }

    title: row < 0 ? i18n("Add Countdown") : i18n("Edit Countdown")
    standardButtons: QQC2.Dialog.Save | QQC2.Dialog.Cancel

    onAccepted: {
        if (row < 0) {
            countdowns.add(nameField.text, dateField.value, occurrenceField.currentIndex, messageField.text);
        } else {
            countdowns.update(row, nameField.text, dateField.value, occurrenceField.currentIndex, messageField.text);
        }
    }

    Component.onCompleted: standardButton(QQC2.Dialog.Save).enabled = Qt.binding(() => nameField.text.length > 0)

    FormCard.FormTextFieldDelegate {
        id: nameField

        label: i18n("Countdown name")
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormDateTimeDelegate {
        id: dateField

        text: i18n("Date and time")
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormComboBoxDelegate {
        id: occurrenceField

        text: i18n("Occurrence")
        model: dialog.countdowns.occurrenceLabels
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: messageField

        label: i18n("Message when it is over")
    }
}
