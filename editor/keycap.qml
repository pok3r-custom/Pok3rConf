import QtQuick 2.0
import QtQuick.Controls 2.0

Button {
    id: keycap
    property int keyIndex: 0
    onClicked: mainwindow.customizeKey(keyIndex)
    padding: 1

    background: Rectangle {
        color: keycap.down ? "#d6d6d6" : "#f6f6f6"
        border.color: "#aaaaaa"
        border.width: 1
        radius: 4
    }

    contentItem: Text {
        text: keycap.text
        color: "#404244"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: "monospace"
        // If text goes out of bounds, don't overflow, because the background does not get repainted
        clip: true
        // Sets a custom font size based on the container height/width
        fontSizeMode: Text.Fit
        // Needed for fontSizeMode
        minimumPointSize: 1
        // Don't put ellipsis for text larger than container (if we don't want to use fontSizeMode)
        //elide: Text.ElideNone
        // Disable desktop font size (if we don't want to use fontSizeMode)
        //font.pointSize: 8
    }
}
