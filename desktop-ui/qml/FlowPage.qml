import "Theme.js" as Theme
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id: page
    required property var model
    contentHeight: deviceContent.implicitHeight
    ColumnLayout {
        id: deviceContent
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
                    color: Theme.text
                    font.pixelSize: 17
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Gateway TCP: " + (page.model.connected ? "Connected" : "Reconnecting every 2 seconds")
                    color: Theme.muted
                    font.pixelSize: 13
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Device: " + (!page.model.connected ? "Unknown while gateway is disconnected" : page.model.online ? "Responding to gateway" : "Offline") + "  ·  State: " + (page.model.connected && page.model.online && page.model.valid ? "Current" : "Unavailable or stale")
                    color: Theme.muted
                    font.pixelSize: 13
                }
                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: "Heartbeat every 2 seconds · device offline after 6 seconds without a response"
                    color: Theme.muted
                    font.pixelSize: 11
                }
            }
        }
    }
}
