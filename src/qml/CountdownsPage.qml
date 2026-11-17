// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import net.kurokyuu.istarcountdown

Kirigami.ScrollablePage {
    id: page

    required property CountdownModel countdowns
    required property CountdownTimer countdownTimer

    signal timerStarted

    title: i18n("Countdowns")

    // The locale's short date with a four-digit year (a birthday as "02.11.91" says nothing), then its
    // time. Not Qt.formatDateTime(date, Locale.ShortFormat): a number there is read as a Qt::DateFormat,
    // and ShortFormat is 1, which is Qt::ISODate - "1991-11-02T00:00:00".
    function dateText(dateTime) {
        const locale = Qt.locale();
        let dateFormat = locale.dateFormat(Locale.ShortFormat);

        if (dateFormat.indexOf("yyyy") < 0) {
            dateFormat = dateFormat.replace("yy", "yyyy");
        }

        return Qt.formatDateTime(dateTime, dateFormat + " " + locale.timeFormat(Locale.ShortFormat));
    }

    actions: [
        Kirigami.Action {
            text: i18n("Add Countdown")
            icon.name: "list-add"
            onTriggered: dialog.openNew()
        }
    ]

    ListView {
        id: list

        model: page.countdowns

        delegate: Kirigami.SwipeListItem {
            id: item

            required property int index
            required property string name
            required property date dateTime
            required property int occurrence
            required property string occurrenceText
            required property string iconName
            required property string message

            function start() {
                page.countdownTimer.startCountdown(dateTime, occurrence, message);
                page.timerStarted();
            }

            onClicked: start()

            contentItem: Kirigami.IconTitleSubtitle {
                icon.name: item.iconName
                title: item.name
                subtitle: i18nc("date and time, then how often it repeats", "%1 · %2",
                                page.dateText(item.dateTime), item.occurrenceText)
            }

            actions: [
                Kirigami.Action {
                    text: i18n("Start")
                    icon.name: "chronometer-start"
                    onTriggered: item.start()
                },
                Kirigami.Action {
                    text: i18n("Edit")
                    icon.name: "document-edit"
                    onTriggered: dialog.openEdit(item.index, item.name, item.dateTime, item.occurrence, item.message)
                },
                Kirigami.Action {
                    text: i18n("Delete")
                    icon.name: "edit-delete"
                    onTriggered: {
                        confirmDelete.row = item.index;
                        confirmDelete.open();
                    }
                }
            ]
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
            visible: list.count === 0
            icon.name: "view-calendar"
            text: i18n("No countdowns yet")
            helpfulAction: page.actions[0]
        }
    }

    CountdownDialog {
        id: dialog

        countdowns: page.countdowns
    }

    Kirigami.PromptDialog {
        id: confirmDelete

        property int row: -1

        title: i18n("Delete Countdown")
        subtitle: i18n("Do you really want to remove the selected item from your countdown list?")
        standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.No
        onAccepted: page.countdowns.remove(row)
    }
}
