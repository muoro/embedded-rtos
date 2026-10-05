import QtQuick
import QtQuick.Controls

// Keep mouse-wheel scrolling consistent across desktop pages.
ScrollView {
    id: page
    contentWidth: availableWidth
    clip: true
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    WheelHandler {
        target: null
        onWheel: event => {
            const delta = event.pixelDelta.y || event.angleDelta.y / 3;
            const maximum = Math.max(0, page.contentHeight - page.availableHeight);
            page.contentItem.contentY = Math.max(0, Math.min(maximum, page.contentItem.contentY - delta));
            event.accepted = true;
        }
    }
}
