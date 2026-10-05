import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Panel {
    id: panel
    required property var model
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12
        Text {
            text: "Event timeline"
            color: "#213849"
            font.pixelSize: 17
            font.weight: Font.DemiBold
        }
        Text {
            text: "Device changes and command outcomes · newest first"
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: "#677e8b"
            font.pixelSize: 12
        }
        Text {
            visible: panel.model.events.length === 0
            text: "Waiting for the gateway…"
            color: "#677e8b"
            font.pixelSize: 13
        }
        ListView {
            objectName: "eventList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: panel.model.events
            delegate: Item {
                required property string modelData
                width: ListView.view.width
                height: eventText.implicitHeight + 26
                Text {
                    x: 0
                    y: 13
                    text: modelData.slice(0, 8)
                    color: "#677e8b"
                    font.family: "Consolas"
                    font.pixelSize: 12
                }
                Text {
                    id: eventText
                    x: 84
                    y: 13
                    width: parent.width - 100
                    text: modelData.slice(10)
                    color: "#213849"
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#edf2f3"
                }
            }
            ScrollBar.vertical: ScrollBar {}
        }
    }
}
