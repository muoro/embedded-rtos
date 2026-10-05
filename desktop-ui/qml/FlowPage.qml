import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: page
    required property var model
    contentWidth: availableWidth
    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ColumnLayout {
        width: page.availableWidth
        spacing: 14
        FlowPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: 280
            model: page.model
            detailed: true
        }
        Panel {
            Layout.fillWidth: true
            implicitHeight: health.implicitHeight + 32
            ColumnLayout {
                id: health
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 16
                spacing: 10
                Text {
                    text: "Connection health"
                    color: "#213849"
                    font.pixelSize: 17
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Gateway TCP: " + (page.model.connected ? "Connected" : "Reconnecting every 2 seconds")
                    color: "#677e8b"
                    font.pixelSize: 13
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Device: " + (!page.model.connected ? "Unknown while gateway is disconnected" : page.model.online ? "Responding to gateway" : "Offline") + "  ·  State: " + (page.model.connected && page.model.online && page.model.valid ? "Current" : "Unavailable or stale")
                    color: "#677e8b"
                    font.pixelSize: 13
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Heartbeat every 2 seconds · device offline after 6 seconds without a response"
                    color: "#677e8b"
                    font.pixelSize: 11
                }
            }
        }
    }
}
