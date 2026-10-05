import QtQuick
import "Theme.js" as Theme

Canvas {
    id: icon
    property string name: "home"
    property color stroke: Theme.muted
    implicitWidth: 24
    implicitHeight: 24
    onNameChanged: requestPaint()
    onStrokeChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        const c = getContext("2d");
        c.reset(); c.scale(width / 24, height / 24);
        c.strokeStyle = stroke; c.lineWidth = 1.55;
        c.lineCap = "round"; c.lineJoin = "round";
        function line(points) {
            c.beginPath(); c.moveTo(points[0], points[1]);
            for (let i = 2; i < points.length; i += 2) c.lineTo(points[i], points[i+1]);
            c.stroke();
        }
        function circle(x,y,r) { c.beginPath(); c.arc(x,y,r,0,Math.PI*2); c.stroke(); }
        switch (name) {
        case "home": line([2,10,12,2,22,10]); line([5,9,5,22,10,22,10,15,14,15,14,22,19,22,19,9]); break;
        case "person": circle(12,7,4); c.beginPath(); c.arc(12,21,8,Math.PI,Math.PI*2); c.stroke(); line([4,21,20,21]); break;
        case "light": circle(12,9,5); line([9,14,9,18,15,18,15,14]); line([10,21,14,21]);
            line([12,0,12,1]); line([3,3,5,5]); line([0,10,2,10]); line([19,5,21,3]); line([22,10,24,10]); break;
        case "door": line([4,22,4,4,15,1,15,22,4,22]); line([17,4,21,4,21,22,17,22]); circle(11,12,0.6); break;
        case "shield": line([12,2,21,6,20,14,17,19,12,23,7,19,4,14,3,6,12,2]); break;
        case "chip": c.strokeRect(5,5,14,14); c.strokeRect(8,8,8,8);
            for (let n=8;n<=16;n+=4) {line([n,2,n,5]);line([n,19,n,22]);line([2,n,5,n]);line([19,n,22,n]);} break;
        case "server": c.strokeRect(3,3,18,7); c.strokeRect(3,14,18,7); line([6,6,7,6]);line([6,17,7,17]);line([14,7,18,7]);line([14,18,18,18]); break;
        case "monitor": c.strokeRect(2,3,20,14); line([12,17,12,22]);line([7,22,17,22]); break;
        case "events": c.strokeRect(3,5,18,17);line([3,10,21,10]);line([7,2,7,7]);line([17,2,17,7]); break;
        case "check": line([5,12,10,17,20,6]); break;
        case "close": line([5,5,19,19]);line([19,5,5,19]); break;
        case "minimize": line([5,12,19,12]); break;
        case "maximize": c.strokeRect(5,5,14,14); break;
        case "restore": c.strokeRect(4,8,12,12);line([8,8,8,4,20,4,20,16,16,16]); break;
        }
    }
}
