import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    id: win
    color: "transparent"

    function setSize(width, height) {
        win.width = width
        win.height = height
    }

    function updateRepr(index, value) {
        keyboard.keys[index].text = qsTr(value)
    }

    Rectangle {
        id: keyboard
        width: 600
        height: 200
        color: "transparent"

        transform: [
            Scale {
                id: scale
                xScale: yScale
                yScale: Math.min(win.width / keyboard.width, win.height / keyboard.height)
            },
            Translate {
                x: (win.width - keyboard.width * scale.xScale) / 2
                y: (win.height - keyboard.height * scale.yScale) / 2
            }
        ]

        property variant keys: []
    }

    function updateLayout(layer) {
        var keyWidthList = keymapper.getKeyLayout()
        var keyReprList = keymapper.getKeyLayer(layer)

        for (var key in keyboard.keys) {
            keyboard.keys[key].destroy()
        }

        keyboard.keys = []

        var x = 0
        var y = 0
        var spacers = 0
        var largestWidth = 0

        // constants
        var keyHeight = 20
        var keyWidth = 8
        var keyMargin = 4

        for(key in keyWidthList) {
            var size = keyWidthList[key]

            // new row
            if (size === -1) {
                y++
                x = 0
                continue
            }

            // spacer
            if (size > 100) {
                x += size - 100
                spacers++
                continue
            }

            var keyX = keyWidth * x
            var keyY = (keyHeight + keyMargin) * y
            var localKeyWidth = (keyWidth * size) - keyMargin
            var keyReprIndex = key - y - spacers

            var keycap = "
                import QtQuick 2.0
                import QtQuick.Controls 2.0
                Button {
                    id: keycap
                    x: " + keyX + "
                    y: " + keyY + "
                    width: " + localKeyWidth + "
                    height: " + keyHeight + "
                    text: qsTr(\"" + keyReprList[keyReprIndex] + "\")
                    onClicked: keymapper.customize(" + keyReprIndex + ")
                    padding: 1
                    background: Rectangle {
                        color: keycap.down ? \"#d6d6d6\" : \"#f6f6f6\"
                        border.color: \"#aaa\"
                        border.width: 1
                        radius: 4
                    }
                    contentItem: Text {
                        text: keycap.text
                        color: \"#404244\"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.family: \"monospace\"
                        // If text goes out of bounds, don't overflow, because the background does not get repainted
                        clip: true
                        // Sets a custom font size based on the container height/width
                        fontSizeMode: Text.Fit
                        // Needed for fontSizeMode
                        minimumPointSize: 1
                        // Don't put ellipsis for text larger than container (if we don't want to use fontSizeMode)
//                        elide: Text.ElideNone
                        // Disable desktop font size (if we don't want to use fontSizeMode)
//                        font.pointSize: 8
                    }
                }"

            var item = Qt.createQmlObject(keycap, keyboard, "dynamicItem")
            keyboard.keys.push(item)

            x += size

            if (keyX + localKeyWidth > largestWidth) {
                largestWidth = keyX + localKeyWidth
            }
        }

        // TODO remove the 16 magic margin number once dynamic width works
        keyboard.width = largestWidth + 16
        keyboard.height = y * (keyHeight + keyMargin) + 16
    }
}
