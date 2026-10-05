import QtQuick
import QtQuick.Layouts
import "Theme.js" as Theme

Rectangle {
    id: pill
    property string label
    property color tone: Theme.success
    property bool outlined: false
    implicitHeight: 30
    implicitWidth: content.implicitWidth + 24
    radius: height / 2
    color: outlined ? Qt.rgba(tone.r,tone.g,tone.b,0.06) : "transparent"
    border.color: outlined ? tone : Theme.border
    RowLayout {
        id: content
        anchors.centerIn: parent
        spacing: 9
        Rectangle { width: 8; height: 8; radius: 4; color: pill.tone }
        Text { text: pill.label; color: pill.outlined ? pill.tone : Theme.text; font.pixelSize: 12 }
    }
}
