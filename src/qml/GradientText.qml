// SPDX-FileCopyrightText: 2026 Pascal Klose
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Effects

/// A line of text filled with a vertical gradient. Text itself can only be one colour, so the
/// gradient is drawn on a rectangle and the text is the mask that cuts it out.
Item {
    id: root

    property alias text: mask.text
    property alias font: mask.font
    property color startColor: "white"
    property color endColor: "white"

    implicitWidth: mask.implicitWidth
    implicitHeight: mask.implicitHeight

    Rectangle {
        id: fill

        anchors.fill: parent
        visible: false
        layer.enabled: true
        layer.smooth: true
        gradient: Gradient {
            GradientStop {
                position: 0
                color: root.startColor
            }
            GradientStop {
                position: 1
                color: root.endColor
            }
        }
    }

    Text {
        id: mask

        anchors.fill: parent
        visible: false
        layer.enabled: true
        // Without smooth the mask texture is sampled unfiltered and the glyph edges come out as hard steps.
        layer.smooth: true
        layer.samples: 4
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        // "364 days 23:25:11" is wider than a phone at the size the page asks for; shrink to the width
        // instead of cutting the ends off.
        fontSizeMode: Text.Fit
        minimumPixelSize: 12
        color: "black"
    }

    MultiEffect {
        anchors.fill: parent
        source: fill
        maskEnabled: true
        maskSource: mask
    }
}
