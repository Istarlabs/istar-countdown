// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

// A time of day in a form card: typed on a desktop ("8:30", "08.30" or "0830"), where the picker's wheel is slower
// than the keyboard, and picked on a phone. It does not change `value` itself; the page writes what `edited` hands
// it back to the model, and the binding brings it here again.
ColumnLayout {
    id: root

    required property string label
    property date value

    signal edited(date time)

    Layout.fillWidth: true
    spacing: 0

    // The typed time on the day `value` is on, or null for text that is no time.
    function parse(text) {
        const match = /^([01]?\d|2[0-3])[:.]?([0-5]\d)$/.exec(text.trim());

        if (match === null) {
            return null;
        }

        const time = new Date(root.value);
        time.setHours(Number(match[1]), Number(match[2]), 0, 0);

        return time;
    }

    function show() {
        typed.text = Qt.formatTime(root.value, "hh:mm");
    }

    onValueChanged: show()
    Component.onCompleted: show()

    FormCard.FormTextFieldDelegate {
        id: typed

        visible: !Kirigami.Settings.isMobile
        label: root.label
        inputMethodHints: Qt.ImhPreferNumbers

        // Text that is no time goes back to the last valid one.
        onEditingFinished: {
            const time = root.parse(text);

            if (time !== null) {
                root.edited(time);
            }

            root.show();
        }
    }

    FormCard.FormDateTimeDelegate {
        id: picked

        visible: Kirigami.Settings.isMobile
        text: root.label
        dateTimeDisplay: FormCard.FormDateTimeDelegate.DateTimeDisplay.Time
        onValueChanged: {
            if (value.getTime() !== root.value.getTime()) {
                root.edited(value);
            }
        }
    }

    // The picker writes its own value, which breaks a plain binding.
    Binding {
        target: picked
        property: "value"
        value: root.value
    }
}
