import QtQuick

Item {
    id: map
    required property var room
    property bool live: false
    readonly property bool known: room.occupied !== undefined
    readonly property bool occupied: room.occupied === true
    readonly property bool lightOn: room.light_on === true
    readonly property bool contactOpen: room.contact_open === true
    readonly property string alarm: room.alarm === undefined ? "unknown" : room.alarm
    Accessible.role: Accessible.Graphic
    Accessible.name: !known ? "Room state unknown" : "Room " + (occupied ? "occupied" : "vacant") + ", light " + (lightOn ? "on" : "off") + ", contact " + (contactOpen ? "open" : "closed") + ", alarm " + alarm + (live ? "" : ", cached state")
    onRoomChanged: drawing.requestPaint()
    onLiveChanged: drawing.requestPaint()
    Canvas {
        id: drawing
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        // Normalized coordinates keep the plan readable as the window resizes.
        onPaint: {
            var c = getContext("2d");
            c.reset();
            c.scale(width / 480, height / 310);
            c.strokeStyle = "#edf2f3";
            c.lineWidth = 1;
            for (var x = 0; x < 480; x += 25) {
                c.beginPath();
                c.moveTo(x, 0);
                c.lineTo(x, 310);
                c.stroke();
            }
            for (var y = 0; y < 310; y += 25) {
                c.beginPath();
                c.moveTo(0, y);
                c.lineTo(480, y);
                c.stroke();
            }
            c.globalAlpha = map.live ? 1 : 0.5;
            c.fillStyle = map.lightOn ? "#fff5cd" : "#f5f8f7";
            c.fillRect(27, 25, 423, 250);
            c.strokeStyle = "#aebfc6";
            c.lineWidth = 7;
            c.strokeRect(27, 25, 423, 250);
            c.fillStyle = "#9dd9de";
            c.fillRect(148, 21, 140, 8);
            // Sofa and coffee table.
            c.fillStyle = "#d7e5e5";
            c.strokeStyle = "#bbcdce";
            c.lineWidth = 2;
            c.fillRect(75, 100, 94, 115);
            c.strokeRect(75, 100, 94, 115);
            c.beginPath();
            c.moveTo(95, 100);
            c.lineTo(95, 215);
            c.moveTo(95, 157);
            c.lineTo(169, 157);
            c.stroke();
            c.fillStyle = "#e9ded0";
            c.strokeStyle = "#d6c7b5";
            c.beginPath();
            c.ellipse(202, 125, 58, 78);
            c.fill();
            c.stroke();
            // Contact state opens the door into the room.
            c.clearRect(446, 181, 9, 61);
            c.strokeStyle = map.known ? "#168e80" : "#aebfc6";
            c.lineWidth = 6;
            c.beginPath();
            c.moveTo(450, 242);
            c.lineTo(map.contactOpen ? 395 : 450, map.contactOpen ? 222 : 182);
            c.stroke();
            // Lamp icon.
            c.fillStyle = map.lightOn ? "#ffebad" : "#edf2f3";
            c.beginPath();
            c.arc(335, 88, 27, 0, Math.PI * 2);
            c.fill();
            c.strokeStyle = map.lightOn ? "#b48114" : "#96a6ad";
            c.lineWidth = 2.5;
            c.beginPath();
            c.arc(335, 84, 10, 0, Math.PI * 2);
            c.stroke();
            c.beginPath();
            c.moveTo(330, 94);
            c.lineTo(330, 101);
            c.lineTo(340, 101);
            c.lineTo(340, 94);
            c.moveTo(332, 105);
            c.lineTo(338, 105);
            c.stroke();
            // Presence icon.
            c.globalAlpha = map.occupied ? (map.live ? 1 : 0.5) : 0.2;
            c.fillStyle = "#e4f4ef";
            c.beginPath();
            c.arc(337, 192, 28, 0, Math.PI * 2);
            c.fill();
            c.strokeStyle = "#168e80";
            c.lineWidth = 3;
            c.lineCap = "round";
            c.beginPath();
            c.arc(337, 177, 5, 0, Math.PI * 2);
            c.stroke();
            c.beginPath();
            c.moveTo(337, 186);
            c.lineTo(337, 198);
            c.moveTo(325, 187);
            c.lineTo(349, 187);
            c.moveTo(330, 209);
            c.lineTo(337, 198);
            c.lineTo(344, 209);
            c.stroke();
            c.globalAlpha = 1;
        }
    }
    Text {
        x: parent.width * 0.08
        y: parent.height * 0.79
        text: "ROOM 01"
        font.pixelSize: 10
        font.letterSpacing: 1.5
        color: "#677e8b"
    }
    Text {
        anchors.right: parent.right
        anchors.rightMargin: 16
        y: 5
        text: alarm === "alarm" ? "ALARM" : alarm === "warning" ? "WARNING" : ""
        color: live ? "#ad5750" : "#8b969e"
        font.pixelSize: 11
        font.bold: true
    }
    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: !known ? "Unknown · waiting for state" : (occupied ? "Occupied" : "Vacant") + "   ·   " + (contactOpen ? "Contact open" : "Contact closed") + "   ·   " + (alarm === "none" ? "Alarm clear" : alarm)
        color: "#677e8b"
        font.pixelSize: 11
    }
}
