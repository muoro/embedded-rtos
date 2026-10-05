import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: Math.min(980, Screen.desktopAvailableWidth * 0.90)
    height: Math.min(680, Screen.desktopAvailableHeight * 0.85)
    minimumWidth: 640
    minimumHeight: 440
    visible: true
    title: "Smart Room · Command Center"
    color: "#f3f7f8"
    font.family: "Segoe UI"
    property int page: 0
    readonly property bool live: gateway.connected && gateway.online && gateway.valid
    readonly property string connectionText: !gateway.connected ? "Gateway disconnected" : !gateway.online ? "Device offline" : !gateway.valid ? "Waiting for state" : "Device connected"
    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            Layout.preferredWidth: window.width < 800 ? 132 : 148
            Layout.fillHeight: true
            color: "white"
            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: "#dae6e8"
            }
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8
                Text {
                    text: "Smart Room"
                    color: "#168e80"
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                    Layout.topMargin: 14
                    Layout.bottomMargin: 30
                }
                Repeater {
                    model: ["Overview", "Live Flow", "Events"]
                    Button {
                        required property string modelData
                        required property int index
                        objectName: "nav" + index
                        Layout.fillWidth: true
                        implicitHeight: 42
                        text: modelData
                        onClicked: window.page = index
                        contentItem: Text {
                            text: parent.text
                            leftPadding: 12
                            verticalAlignment: Text.AlignVCenter
                            color: window.page === index ? "#168e80" : "#677e8b"
                            font.pixelSize: 14
                            font.weight: window.page === index ? Font.DemiBold : Font.Normal
                        }
                        background: Rectangle {
                            radius: 8
                            color: window.page === index ? "#e4f4ef" : parent.hovered ? "#f3f7f8" : "transparent"
                            border.width: parent.activeFocus ? 2 : 0
                            border.color: "#168e80"
                        }
                    }
                }
                Text {
                    visible: window.height >= 620
                    text: "SYSTEM"
                    color: "#677e8b"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    Layout.topMargin: 30
                    Layout.leftMargin: 10
                }
                Text {
                    visible: window.height >= 620
                    text: "nRF52832 DK\nRoom controller"
                    color: "#677e8b"
                    font.pixelSize: 12
                    lineHeight: 1.5
                    Layout.leftMargin: 10
                    Layout.topMargin: 8
                }
                Text {
                    visible: window.height >= 620
                    text: "Linux gateway\nBuildroot · ARM64"
                    color: "#677e8b"
                    font.pixelSize: 12
                    lineHeight: 1.5
                    Layout.leftMargin: 10
                    Layout.topMargin: 8
                }
                Item {
                    Layout.fillHeight: true
                }
                Text {
                    text: "ROOM 01\nWindows · Qt / QML"
                    color: "#677e8b"
                    font.pixelSize: 11
                    lineHeight: 1.6
                    Layout.leftMargin: 10
                    Layout.bottomMargin: 12
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16
            spacing: 18
            GridLayout {
                Layout.fillWidth: true
                columns: window.width >= 900 ? 3 : 1
                ColumnLayout {
                    spacing: 5
                    Text {
                        text: ["Room overview", "Live system flow", "Recent events"][window.page]
                        color: "#213849"
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                    }
                    Text {
                        text: "One room. Every state and connection in view."
                        color: "#677e8b"
                        font.pixelSize: 12
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                Rectangle {
                    implicitWidth: statusText.implicitWidth + 30
                    implicitHeight: 32
                    radius: 16
                    color: window.live ? "#e4f4ef" : "#fff0d9"
                    Text {
                        id: statusText
                        objectName: "connectionStatus"
                        anchors.centerIn: parent
                        text: window.connectionText
                        color: window.live ? "#168e80" : "#996523"
                        font.pixelSize: 12
                    }
                }
            }
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 0
                currentIndex: window.page
                OverviewPage {
                    objectName: "overviewPage"
                    model: gateway
                }
                FlowPage {
                    model: gateway
                }
                EventPanel {
                    model: gateway
                }
            }
            Text {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: window.live ? "LIVE HARDWARE  ·  Room state is owned by the nRF controller" : "Controls unlock after a fresh, valid device state arrives"
                color: "#677e8b"
                font.pixelSize: 11
            }
        }
    }
}
