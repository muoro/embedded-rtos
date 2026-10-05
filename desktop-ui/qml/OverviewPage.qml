import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Theme.js" as Theme

ScrollablePage {
    id: page
    required property var model
    signal viewAllEvents()
    readonly property bool live: model.connected && model.online && model.valid
    readonly property bool wide: availableWidth >= 760
    contentHeight: overviewContent.implicitHeight
    ColumnLayout {
        id: overviewContent
        width: page.availableWidth
        spacing: 14
        TopologyStrip { Layout.fillWidth: true; Layout.preferredHeight: 44; model: page.model }
        GridLayout {
            Layout.fillWidth: true; columns: page.availableWidth >= 660 ? 4 : 2; columnSpacing: 14; rowSpacing: 14
            StatCard { Layout.fillWidth: true; Layout.preferredWidth: 180; label: "Occupancy"; icon: "person"; live: page.live; value: page.model.room.occupied === undefined ? "Unknown" : page.model.room.occupied ? "Occupied" : "Vacant" }
            StatCard { Layout.fillWidth: true; Layout.preferredWidth: 180; label: "Light"; icon: "light"; live: page.live; highlighted: page.model.room.light_on === true; value: page.model.room.light_on === undefined ? "Unknown" : page.model.room.light_on ? "On" : "Off" }
            StatCard { Layout.fillWidth: true; Layout.preferredWidth: 180; label: "Contact"; icon: "door"; live: page.live; value: page.model.room.contact_open === undefined ? "Unknown" : page.model.room.contact_open ? "Open" : "Closed" }
            StatCard { Layout.fillWidth: true; Layout.preferredWidth: 180; label: "Alarm"; icon: "shield"; live: page.live; value: page.model.room.alarm === undefined ? "Unknown" : page.model.room.alarm === "none" ? "Clear" : page.model.room.alarm === "warning" ? "Warning" : "Alarm" }
        }
        GridLayout {
            Layout.fillWidth: true; columns: page.wide ? 2 : 1; columnSpacing: 14; rowSpacing: 14
            Panel {
                Layout.fillWidth: true
                Layout.preferredWidth: page.availableWidth * 0.61
                Layout.preferredHeight: page.wide ? Math.max(310, page.availableHeight - 324) : 340
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 10
                    Text { text: "Living room"; color: Theme.text; font.pixelSize: 17; font.weight: Font.DemiBold }
                    RoomMap { objectName: "roomMap"; Layout.fillWidth: true; Layout.fillHeight: true; room: page.model.room; live: page.live }
                    RowLayout {
                        spacing: 8
                        Rectangle { width: 8; height: 8; radius: 4; color: page.live ? Theme.success : Theme.warning }
                        Text { text: page.live ? "State synchronized" : Object.keys(page.model.room).length ? "Cached state · not current" : "Waiting for device state"; color: Theme.muted; font.pixelSize: 11 }
                    }
                }
            }
            RoomControl {
                Layout.fillWidth: true; Layout.preferredWidth: page.availableWidth * 0.39
                Layout.preferredHeight: page.wide ? Math.max(310, page.availableHeight - 324) : 320
                model: page.model
            }
        }
        EventPanel { Layout.fillWidth: true; Layout.preferredHeight: 158; model: page.model; compact: true; onViewAll: page.viewAllEvents() }
    }
}
