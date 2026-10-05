import QtQuick
import QtQuick.Layouts

Panel {
    id: panel
    required property var model
    property bool detailed: false
    property string lastMessage: "Waiting for gateway traffic"
    property bool outbound: false
    property real progress: 0
    Connections {
        target: panel.model
        function onTrafficObserved(line, outbound) {
            panel.lastMessage = line;
            panel.outbound = outbound;
            pulse.restart();
        }
    }
    NumberAnimation {
        id: pulse
        target: panel
        property: "progress"
        from: 0
        to: 1
        duration: 650
    }
    component Node: Rectangle {
        required property string label
        required property string detail
        property bool active: false
        Layout.fillWidth: true
        implicitHeight: 62
        radius: 9
        color: active ? "#eaf5f1" : "#f3f6f7"
        Column {
            anchors.centerIn: parent
            spacing: 5
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: label
                color: "#213849"
                font.pixelSize: panel.width < 600 ? 11 : 13
                font.weight: Font.DemiBold
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: detail
                color: "#677e8b"
                font.pixelSize: 10
            }
        }
    }
    component Wire: Item {
        required property string label
        Layout.preferredWidth: panel.width < 600 ? 26 : panel.detailed ? 90 : 55
        implicitHeight: 40
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 0
            text: label
            color: "#677e8b"
            font.pixelSize: 10
        }
        Rectangle {
            y: 25
            width: parent.width
            height: 2
            color: "#c9dcd9"
        }
        Rectangle {
            y: 22
            width: 8
            height: 8
            radius: 4
            color: "#168e80"
            visible: pulse.running
            x: (parent.width - width) * (panel.outbound ? 1 - panel.progress : panel.progress)
        }
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        Text {
            text: panel.detailed ? "From the room to your screen" : "System activity"
            color: "#213849"
            font.pixelSize: 16
            font.weight: Font.DemiBold
        }
        Text {
            visible: panel.detailed
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "Commands travel toward the device; state returns to the screen."
            color: "#677e8b"
            font.pixelSize: 12
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Node {
                label: "nRF52832"
                detail: "Room state"
                active: panel.model.connected && panel.model.online
            }
            Wire {
                label: "UART"
            }
            Node {
                label: "Linux gateway"
                detail: panel.detailed ? "QEMU · ttyAMA1" : "Buildroot"
                active: panel.model.connected
            }
            Wire {
                label: "TCP"
            }
            Node {
                label: "Qt / QML"
                detail: "Room control"
                active: panel.model.connected
            }
        }
        Text {
            Layout.fillWidth: true
            text: panel.lastMessage
            color: "#677e8b"
            font.family: "Consolas"
            font.pixelSize: 10
            elide: Text.ElideRight
        }
        Text {
            visible: panel.detailed
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "UART 115200 8N1   ·   TCP 127.0.0.1:5556\nAnimation follows messages observed by Qt; it is not a UART packet trace."
            color: "#677e8b"
            font.pixelSize: 11
            lineHeight: 1.5
        }
    }
}
