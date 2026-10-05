import QtQuick
import QtQuick.Layouts
import "Theme.js" as Theme

Panel {
    id: card
    property string label
    property string value
    property string icon
    property bool live: false
    property bool highlighted: false
    implicitHeight: 80
    Accessible.role: Accessible.StaticText
    Accessible.name: label + ": " + value + (live ? "" : ", not current")
    RowLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 14
        LineIcon { name: card.icon; Layout.preferredWidth: 32; Layout.preferredHeight: 32; stroke: card.live && card.highlighted ? Theme.accent : Theme.muted }
        ColumnLayout {
            Layout.fillWidth: true; spacing: 4
            Text { Layout.fillWidth: true; text: card.label; color: Theme.muted; font.pixelSize: 12; elide: Text.ElideRight }
            Text { Layout.fillWidth: true; text: card.value; color: card.live ? Theme.text : Theme.faint; font.pixelSize: 21; font.weight: Font.DemiBold; elide: Text.ElideRight }
        }
    }
}
