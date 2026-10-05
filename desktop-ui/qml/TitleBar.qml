import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as Theme

Rectangle {
    id: bar
    required property var window
    implicitHeight: 40
    color: Theme.background
    function toggleMaximized() { if (window.visibility === Window.Maximized) window.showNormal(); else window.showMaximized(); }
    MouseArea {
        anchors.fill: parent; anchors.rightMargin: 126
        onPressed: bar.window.startSystemMove()
        onDoubleClicked: bar.toggleMaximized()
    }
    RowLayout {
        anchors.left: parent.left; anchors.leftMargin: 24; anchors.verticalCenter: parent.verticalCenter; spacing: 14
        LineIcon { name: "home"; stroke: Theme.accent; Layout.preferredWidth: 20; Layout.preferredHeight: 20 }
        Text { text: "Smart Room"; color: Theme.text; font.pixelSize: 14; font.weight: Font.DemiBold }
        Rectangle { visible: bar.width > 750; width: 1; height: 16; color: Theme.border }
        Text { visible: bar.width > 750; text: "Embedded Linux & RTOS"; color: Theme.muted; font.pixelSize: 13 }
    }
    Row {
        anchors.right: parent.right; height: parent.height
        Repeater {
            model: ["minimize", "maximize", "close"]
            Button {
                required property string modelData
                width: 42; height: bar.height
                Accessible.name: modelData
                onClicked: { if (modelData === "close") bar.window.close(); else if (modelData === "minimize") bar.window.showMinimized(); else bar.toggleMaximized(); }
                background: Rectangle { color: parent.hovered ? (parent.modelData === "close" ? "#b34450" : Theme.border) : "transparent" }
                contentItem: Item { LineIcon { anchors.centerIn: parent; width: 16; height: 16; stroke: Theme.text; name: parent.parent.modelData === "maximize" && bar.window.visibility === Window.Maximized ? "restore" : parent.parent.modelData } }
            }
        }
    }
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.border }
}
