import QtQuick
import QtQuick.Layouts
import "Theme.js" as Theme

RowLayout {
    id: strip
    required property var model
    property bool compact: width < 680
    spacing: 10
    implicitHeight: 44
    component Node: Rectangle {
        property string label
        property string icon
        property bool active
        Layout.fillWidth: true
        Layout.preferredWidth: 180
        implicitHeight: 44
        color: "transparent"; radius: 6; border.color: Theme.border
        RowLayout {
            anchors.centerIn: parent; spacing: 12
            LineIcon { name: parent.parent.icon; stroke: parent.parent.active ? Theme.muted : Theme.faint; visible: !strip.compact }
            Text { text: parent.parent.label; color: parent.parent.active ? Theme.text : Theme.faint; font.pixelSize: strip.compact ? 11 : 12 }
        }
    }
    component Wire: Item {
        property string label
        Layout.preferredWidth: strip.compact ? 34 : 62
        implicitHeight: 42
        Text { anchors.horizontalCenter: parent.horizontalCenter; y: 0; text: parent.label; color: Theme.muted; font.pixelSize: 10 }
        Canvas {
            anchors.left: parent.left; anchors.right: parent.right; y: 20; height: 14
            onWidthChanged: requestPaint()
            onPaint: {
                const c=getContext("2d"); c.reset(); c.strokeStyle=Theme.muted; c.lineWidth=1.2;
                c.beginPath(); c.moveTo(1,7); c.lineTo(width-1,7);
                c.moveTo(6,2); c.lineTo(1,7); c.lineTo(6,12);
                c.moveTo(width-6,2); c.lineTo(width-1,7); c.lineTo(width-6,12); c.stroke();
            }
        }
    }
    Node { label: strip.compact ? "nRF52832" : "nRF52832 / Zephyr"; icon: "chip"; active: strip.model.connected && strip.model.online }
    Wire { label: "UART" }
    Node { label: strip.compact ? "ARM64 Linux" : "ARM64 / Buildroot"; icon: "server"; active: strip.model.connected }
    Wire { label: "TCP" }
    Node { label: "Qt / QML"; icon: "monitor"; active: true }
}
