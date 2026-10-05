import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as Theme

ApplicationWindow {
    id: window
    width: Math.min(1200, Screen.desktopAvailableWidth * 0.92)
    height: Math.min(800, Screen.desktopAvailableHeight * 0.90)
    minimumWidth: 640
    minimumHeight: 440
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint
    title: "Smart Room · Embedded Linux & RTOS"
    color: Theme.background
    font.family: "Segoe UI"
    palette.window: Theme.background
    palette.windowText: Theme.text
    palette.text: Theme.text
    palette.base: Theme.panel
    palette.highlight: Theme.accent
    property int page: 0
    readonly property bool compact: width < 1000
    readonly property bool live: gateway.connected && gateway.online && gateway.valid
    readonly property string connectionText: !gateway.connected ? "Gateway offline" : !gateway.online ? "Device offline" : !gateway.valid ? "Waiting for state" : "Device online"
    header: TitleBar { window: window }
    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.preferredWidth: window.compact ? 64 : 186
            Layout.fillHeight: true
            color: Theme.sidebar
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.border }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 8; spacing: 6
                Repeater {
                    model: [{label: "Overview", icon: "home"}, {label: "Devices", icon: "chip"}, {label: "Events", icon: "events"}]
                    Button {
                        id: nav
                        required property var modelData
                        required property int index
                        objectName: "nav" + index
                        Layout.fillWidth: true; Layout.topMargin: index === 0 ? 6 : 0
                        implicitHeight: 48
                        Accessible.name: modelData.label
                        onClicked: window.page = index
                        ToolTip.visible: hovered && window.compact
                        ToolTip.text: modelData.label
                        contentItem: RowLayout {
                            spacing: 16
                            LineIcon { Layout.leftMargin: window.compact ? 4 : 12; name: nav.modelData.icon; stroke: window.page === nav.index ? Theme.accent : Theme.muted }
                            Text { visible: !window.compact; Layout.fillWidth: true; text: nav.modelData.label; color: window.page === nav.index ? "#b3f6f8" : Theme.muted; font.pixelSize: 14; font.weight: window.page === nav.index ? Font.DemiBold : Font.Normal }
                        }
                        background: Rectangle {
                            radius: 7; color: window.page === nav.index ? Theme.selected : nav.hovered ? Theme.panel : "transparent"
                            border.width: nav.activeFocus ? 1 : 0; border.color: Theme.accent
                            Rectangle { visible: window.page === nav.index; anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; width: 4; height: 32; radius: 2; color: Theme.accent }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                RowLayout {
                    Layout.leftMargin: window.compact ? 12 : 16; Layout.bottomMargin: 18; spacing: 10
                    LineIcon { name: "monitor"; width: 18; height: 18 }
                    Rectangle { visible: !window.compact; width: 6; height: 6; radius: 3; color: Theme.accent }
                    Text { visible: !window.compact; text: "Local workspace"; color: Theme.muted; font.pixelSize: 11 }
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.margins: window.width < 800 ? 14 : 24
            spacing: 18
            Item {
                id: pageHeader
                Layout.fillWidth: true
                implicitHeight: heading.implicitHeight + (window.width < 800 ? 38 : 0)
                ColumnLayout {
                    id: heading
                    anchors.left: parent.left
                    anchors.top: parent.top
                    spacing: 4
                    Text { text: ["Room overview", "Connected devices", "Recent events"][window.page]; color: Theme.text; font.pixelSize: window.width < 800 ? 24 : 29; font.weight: Font.Bold }
                    Text { text: ["Live state from the nRF52832", "From the physical device to your screen", "Device changes and command outcomes"][window.page]; color: Theme.muted; font.pixelSize: 14 }
                }
                StatusPill {
                    objectName: "connectionStatus"
                    x: window.width < 800 ? 0 : pageHeader.width - width
                    y: window.width < 800 ? heading.implicitHeight + 8 : (heading.implicitHeight - height) / 2
                    label: window.connectionText
                    tone: window.live ? Theme.success : Theme.warning
                }
            }
            StackLayout {
                Layout.fillWidth: true; Layout.fillHeight: true; Layout.minimumHeight: 0
                currentIndex: window.page
                OverviewPage { objectName: "overviewPage"; model: gateway; onViewAllEvents: window.page = 2 }
                FlowPage { model: gateway }
                EventPanel { model: gateway }
            }
        }
    }
    Rectangle { anchors.fill: parent; color: "transparent"; border.color: Theme.border }
    ResizeHandles { anchors.fill: parent; window: window; z: 100 }
}
