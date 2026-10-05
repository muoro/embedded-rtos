import QtQuick
import "Theme.js" as Theme

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
        opacity: map.live ? 1 : 0.38
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            const c = getContext("2d"); c.reset();
            const scale = Math.min(width / 500, height / 310);
            c.translate((width - 500*scale)/2, (height - 310*scale)/2);
            c.scale(scale,scale);
            c.lineJoin = "round"; c.lineCap = "round";
            function path(points, color, thickness) {
                c.strokeStyle = color || "#aebdca"; c.lineWidth = thickness || 1.3;
                c.beginPath(); c.moveTo(points[0],points[1]);
                for (let i=2; i<points.length; i+=2) c.lineTo(points[i],points[i+1]);
                c.stroke();
            }
            function box(x,y,w,h,color) { c.strokeStyle=color || "#8799a7"; c.lineWidth=1.2; c.strokeRect(x,y,w,h); }
            function circle(x,y,r,color) { c.strokeStyle=color; c.beginPath(); c.arc(x,y,r,0,Math.PI*2); c.stroke(); }
            if (map.lightOn && map.known) {
                const glow = c.createRadialGradient(252,106,4,252,106,116);
                glow.addColorStop(0,"#665537"); glow.addColorStop(0.4,"#393831"); glow.addColorStop(1,"#1d2830");
                c.fillStyle=glow; c.fillRect(124,3,256,244);
            }
            // Floor-plan geometry stays independent of window size.
            path([12,191,12,12,488,12,488,298,12,298,12,265],"#b8c7d3",3);
            path([20,191,20,20,480,20,480,290,20,290,20,265],"#687d8d",1);
            box(142,10,185,10, map.contactOpen ? Theme.warning : "#bccbd5");
            if (map.contactOpen) path([146,10,146,-1,323,-1],Theme.warning,2);
            path([12,192,88,192],"#b8c7d3");
            c.beginPath(); c.arc(12,192,75,0,Math.PI/2); c.stroke();
            box(25,49,29,77); box(445,54,22,103); box(451,86,9,44);
            // Sofa and cushions.
            box(142,239,164,44); box(136,234,14,51);box(298,234,14,51);
            box(155,238,63,30);box(224,238,64,30);path([150,275,298,275]);
            // Coffee table and chair.
            box(164,158,125,62); box(203,178,51,26);
            c.save();c.translate(366,199);c.rotate(-0.62);box(-21,-29,42,55);box(-25,-22,8,46);box(17,-22,8,46);path([-17,14,17,14]);c.restore();
            // Plant: an outline accent rather than extra device hardware.
            for(let p=0;p<5;p++) { c.save();c.translate(439,266);c.rotate(p*Math.PI*2/5);c.beginPath();c.moveTo(0,0);c.quadraticCurveTo(-18,-15,0,-24);c.quadraticCurveTo(15,-12,0,0);c.stroke();c.restore(); }
            circle(439,266,5,"#91a5b5");
            circle(252,106,16,map.lightOn ? "#edc66f" : "#7d91a0");
            c.fillStyle=map.lightOn ? "#edc66f" : "#7d91a0";c.beginPath();c.arc(252,106,5,0,Math.PI*2);c.fill();
            if(map.occupied && map.known) {
                circle(103,110,9,Theme.accent);
                c.beginPath();c.arc(103,140,16,Math.PI,Math.PI*2);c.stroke();
                path([87,140,119,140],Theme.accent);
            }
            if(map.alarm === "warning" || map.alarm === "alarm") {
                c.fillStyle=map.alarm === "alarm" ? Theme.danger : Theme.warning;
                c.font="bold 12px sans-serif";c.fillText(map.alarm.toUpperCase(),359,42);
            }
        }
    }
}
