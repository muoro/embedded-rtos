import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as Theme

Panel {
    id: panel
    required property var model
    property bool compact: false
    signal viewAll()
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 18; spacing: 7
        RowLayout {
            Layout.fillWidth: true
            Text { Layout.fillWidth: true; text: "Recent events"; color: Theme.text; font.pixelSize: 16; font.weight: Font.DemiBold }
            Button {
                visible: panel.compact; text: "View all →"; onClicked: panel.viewAll()
                contentItem: Text { text: parent.text; color: Theme.accent; font.pixelSize: 12 }
                background: Rectangle { color: "transparent"; border.width: parent.activeFocus ? 1 : 0; border.color: Theme.accent; radius: 3 }
            }
        }
        Text { visible: panel.model.events.count === 0; Layout.fillWidth: true; text: "Waiting for gateway events…"; color: Theme.muted; font.pixelSize: 12 }
        ListView {
            objectName: panel.compact ? "recentEventList" : "eventList"
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true; model: panel.model.events
            interactive: !panel.compact
            delegate: Item {
                id: row
                required property string timestamp
                required property string category
                required property string description
                width: ListView.view.width
                height: panel.compact ? 30 : Math.max(42, eventText.implicitHeight + 22)
                readonly property color tone: category === "ACK" ? Theme.success : category === "LINK" ? "#73bbe5" : category === "WARN" ? Theme.warning : Theme.muted
                Rectangle { width: parent.width; height: 1; color: Theme.border; opacity: 0.6 }
                RowLayout {
                    anchors.fill: parent; spacing: 14
                    Text { Layout.preferredWidth: 62; text: row.timestamp; color: Theme.muted; font.pixelSize: 11 }
                    Rectangle {
                        Layout.preferredWidth: 48; implicitHeight: 21; radius: 4
                        color: Qt.rgba(row.tone.r,row.tone.g,row.tone.b,0.12)
                        Text { anchors.centerIn: parent; text: row.category; color: row.tone; font.pixelSize: 10; font.weight: Font.DemiBold }
                    }
                    Text { id: eventText; Layout.fillWidth: true; text: row.description; color: Theme.text; font.pixelSize: 12; wrapMode: panel.compact ? Text.NoWrap : Text.WordWrap; elide: panel.compact ? Text.ElideRight : Text.ElideNone }
                }
            }
            ScrollBar.vertical: ScrollBar { policy: panel.compact ? ScrollBar.AlwaysOff : ScrollBar.AsNeeded }
        }
    }
}
