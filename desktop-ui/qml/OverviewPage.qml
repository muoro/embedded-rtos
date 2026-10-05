import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: page
    required property var model
    readonly property bool live: model.connected && model.online && model.valid
    contentWidth: availableWidth
    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ColumnLayout {
        width: page.availableWidth
        spacing: 14
        GridLayout {
            Layout.fillWidth: true
            columns: page.availableWidth >= 650 ? 2 : 1
            columnSpacing: 14
            rowSpacing: 14
            Panel {
                Layout.fillWidth: true
                Layout.preferredHeight: 410
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10
                    Text {
                        text: "Living room"
                        color: "#213849"
                        font.pixelSize: 17
                        font.weight: Font.DemiBold
                    }
                    Text {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: page.live ? "Current device state" : Object.keys(page.model.room).length ? "Cached state · values are not current" : "Waiting for the first device state"
                        color: page.live ? "#677e8b" : "#996523"
                        font.pixelSize: 12
                    }
                    RoomMap {
                        objectName: "roomMap"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        room: page.model.room
                        live: page.live
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Physical board buttons\nB1  Occupancy    B2  Light    B3  Contact    B4  Alarm"
                        color: "#677e8b"
                        font.pixelSize: 11
                        lineHeight: 1.6
                        wrapMode: Text.WordWrap
                    }
                }
            }
            RoomControl {
                Layout.fillWidth: true
                Layout.preferredWidth: 250
                Layout.maximumWidth: parent.columns === 2 ? 250 : Infinity
                Layout.preferredHeight: 410
                model: page.model
            }
        }
        FlowPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: 174
            model: page.model
        }
    }
}
