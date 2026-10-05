import QtQuick

// Native system resizing preserves normal mouse behavior for our dark title bar.
Item {
    id: handles
    required property var window
    enabled: window.visibility !== Window.Maximized
    Repeater {
        model: [Qt.LeftEdge, Qt.RightEdge, Qt.TopEdge, Qt.BottomEdge,
                Qt.LeftEdge | Qt.TopEdge, Qt.RightEdge | Qt.TopEdge,
                Qt.LeftEdge | Qt.BottomEdge, Qt.RightEdge | Qt.BottomEdge]
        MouseArea {
            required property int modelData
            readonly property bool leftEdge: (modelData & Qt.LeftEdge) !== 0
            readonly property bool rightEdge: (modelData & Qt.RightEdge) !== 0
            readonly property bool topEdge: (modelData & Qt.TopEdge) !== 0
            readonly property bool bottomEdge: (modelData & Qt.BottomEdge) !== 0
            readonly property bool corner: (leftEdge || rightEdge) && (topEdge || bottomEdge)
            width: corner ? 10 : (leftEdge || rightEdge) ? 5 : handles.width - 20
            height: corner ? 10 : (topEdge || bottomEdge) ? 5 : handles.height - 20
            x: leftEdge ? 0 : rightEdge ? handles.width - width : 10
            y: topEdge ? 0 : bottomEdge ? handles.height - height : 10
            cursorShape: corner ? (leftEdge === topEdge ? Qt.SizeFDiagCursor : Qt.SizeBDiagCursor) : (leftEdge || rightEdge) ? Qt.SizeHorCursor : Qt.SizeVerCursor
            onPressed: handles.window.startSystemResize(modelData)
        }
    }
}
