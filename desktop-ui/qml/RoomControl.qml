import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as Theme

Panel {
    id: panel
    required property var model
    readonly property bool fresh: model.connected && model.online && model.valid
    readonly property string outcome: model.result.split(" — ")[0]
    readonly property color resultColor: model.pending ? Theme.warning : outcome === "Confirmed" ? Theme.success : outcome === "No command sent" ? Theme.muted : Theme.warning
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 10
        Text { text: "Light control"; color: Theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
        RowLayout {
            Layout.fillWidth: true; Layout.topMargin: 10
            ColumnLayout {
                spacing: 7
                Text { text: "Ceiling light"; color: Theme.text; font.pixelSize: 14 }
                Text { text: panel.model.room.light_on === undefined ? "Unknown" : panel.model.room.light_on ? "On" : "Off"; color: panel.fresh ? Theme.text : Theme.faint; font.pixelSize: 14 }
            }
            Item { Layout.fillWidth: true }
            AbstractButton {
                id: lightButton
                objectName: "lightSwitch"
                implicitWidth: 64; implicitHeight: 36
                enabled: panel.model.ready
                checked: panel.model.room.light_on === true
                Accessible.role: Accessible.CheckBox
                Accessible.name: "Ceiling light"
                Accessible.checked: checked
                // Do not let a local click pretend that the device accepted a command.
                onClicked: panel.model.setLight(!checked)
                ToolTip.visible: hovered
                ToolTip.text: panel.model.pending ? "Waiting for device confirmation" : !panel.fresh ? "A current device state is required" : "Request light " + (checked ? "off" : "on")
                background: Rectangle {
                    radius: 18
                    color: lightButton.checked && panel.fresh ? Theme.accent : Theme.border
                    opacity: lightButton.enabled ? 1 : 0.45
                    border.width: lightButton.activeFocus ? 2 : 0; border.color: Theme.text
                    Rectangle {
                        width: 28; height: 28; radius: 14; y: 4
                        x: lightButton.checked ? parent.width - width - 4 : 4
                        color: Theme.text
                        Behavior on x { NumberAnimation { duration: 140 } }
                    }
                }
            }
        }
        StatusPill {
            objectName: "commandResult"
            label: panel.model.pending ? "Pending" : panel.outcome
            tone: panel.resultColor; outlined: true
        }
        Text {
            Layout.fillWidth: true; wrapMode: Text.WordWrap
            text: panel.model.pending ? "Waiting for the device reply" : !panel.fresh ? "Controls unlock when fresh state arrives" : panel.outcome === "Confirmed" ? "Command confirmed by device" : panel.model.result
            color: Theme.muted; font.pixelSize: 11
        }
        Item { Layout.fillHeight: true; Layout.minimumHeight: 0 }
        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
        Text { text: "Connection"; color: Theme.text; font.pixelSize: 15; font.weight: Font.DemiBold; Layout.topMargin: 3 }
        GridLayout {
            Layout.fillWidth: true; columns: 2; columnSpacing: 10; rowSpacing: 7
            Text { text: "UART"; color: Theme.muted; font.pixelSize: 12 }
            Text { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: "/dev/ttyAMA1"; color: Theme.muted; font.pixelSize: 12 }
            Text { text: "Baud rate"; color: Theme.muted; font.pixelSize: 12 }
            Text { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: "115200 · 8N1"; color: Theme.muted; font.pixelSize: 12 }
            Text { text: "Gateway"; color: Theme.muted; font.pixelSize: 12 }
            Text { Layout.fillWidth: true; horizontalAlignment: Text.AlignRight; text: "127.0.0.1:5556"; color: Theme.muted; font.pixelSize: 12 }
        }
    }
}
