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

    property var keyWidthList: []

    function setKeyLayout(layout) {
        keyWidthList = layout;
    }

    function updateLayer(keyReprList) {
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

            var comp = Qt.createComponent("keycap.qml")
            var item = comp.createObject(keyboard, {
                x: keyX,
                y: keyY,
                width: localKeyWidth,
                height: keyHeight,
                text: qsTr(keyReprList[keyReprIndex]),
                keyIndex: keyReprIndex,
            })
            keyboard.keys.push(item)

            x += size

            if (keyX + localKeyWidth > largestWidth) {
                largestWidth = keyX + localKeyWidth
            }
        }

        // TODO remove the 16 magic margin number once dynamic width works
        keyboard.width = largestWidth
        keyboard.height = y * (keyHeight + keyMargin)
    }
}
