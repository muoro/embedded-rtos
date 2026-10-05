import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Panel {
    id: panel
    required property var model
    Layout.minimumHeight: 410
    readonly property bool fresh: model.connected && model.online && model.valid
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 8
        Text {
            text: "Room control"
            color: "#213849"
            font.pixelSize: 17
            font.weight: Font.DemiBold
        }
        Text {
            text: "nRF52832 · Room 01"
            color: "#677e8b"
            font.pixelSize: 12
            Layout.bottomMargin: 12
        }
        Repeater {
            model: [
                {
                    label: "Occupancy",
                    value: panel.model.room.occupied === undefined ? "Unknown" : panel.model.room.occupied ? "Occupied" : "Vacant"
                },
                {
                    label: "Light",
                    value: panel.model.room.light_on === undefined ? "Unknown" : panel.model.room.light_on ? "On" : "Off"
                },
                {
                    label: "Contact",
                    value: panel.model.room.contact_open === undefined ? "Unknown" : panel.model.room.contact_open ? "Open" : "Closed"
                },
                {
                    label: "Alarm",
                    value: panel.model.room.alarm === undefined ? "Unknown" : panel.model.room.alarm === "none" ? "Clear" : panel.model.room.alarm === "warning" ? "Warning" : "Alarm"
                }
            ]
            ColumnLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: modelData.label
                        color: "#677e8b"
                        font.pixelSize: 13
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Text {
                        text: modelData.value
                        color: panel.fresh ? "#213849" : "#8b969e"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 1
                    color: "#edf2f3"
                }
            }
        }
        Button {
            id: lightButton
            objectName: "lightSwitch"
            Layout.fillWidth: true
            Layout.topMargin: 12
            implicitHeight: 44
            enabled: panel.model.ready
            checked: panel.model.room.light_on === true
            text: panel.model.pending ? "Awaiting device…" : checked ? "Turn light off" : "Turn light on"
            onClicked: panel.model.setLight(!checked)
            contentItem: Text {
                text: lightButton.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 13
                font.weight: Font.DemiBold
            }
            background: Rectangle {
                radius: 8
                color: !lightButton.enabled ? "#a4b9b6" : lightButton.down ? "#116c62" : lightButton.hovered ? "#117d70" : "#168e80"
                border.width: lightButton.activeFocus ? 2 : 0
                border.color: "#213849"
            }
        }
        Text {
            objectName: "commandResult"
            Layout.fillWidth: true
            Layout.minimumHeight: 38
            text: panel.model.result
            color: "#677e8b"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
        Item {
            Layout.fillHeight: true
        }
        Text {
            Layout.fillWidth: true
            text: "Occupancy or an open contact keeps the light on."
            color: "#677e8b"
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }
}
