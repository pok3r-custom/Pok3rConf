import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    id: win
//    color: "orange"

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
//        color: 'blue'

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

    Component.onCompleted: {
        var keyWidthList = keymapper.getKeyWidth()
        var keyReprList = keymapper.getKeyRepr()

        var x = 0
        var y = 0
        var largestWidth = 0

        for(var key in keyWidthList) {
            var num = keyWidthList[key]

            if (num === -1) {
                y++;
                x = 0;
                continue;
            }

            var keyX = 8 * x
            var keyY = 25 * y
            var keyWidth = 8 * num

            var keycap = "
                import QtQuick 2.0
                import QtQuick.Controls 2.0
                Button {
                    id: keycap
                    x: " + keyX + "
                    y: " + keyY + "
                    width: " + (keyWidth - 4) + "
                    height: 20
                    text: qsTr(\"" + keyReprList[key - y] + "\")
                    onClicked: keymapper.customize(" + (key - y) + ")
                    padding: 0
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
                        elide: Text.ElideNone
                    }
                }"

            var item = Qt.createQmlObject(keycap, keyboard, "dynamicItem")
            keyboard.keys.push(item)

            x += num

            if (keyX + keyWidth > largestWidth) {
                largestWidth = keyX + keyWidth
            }
        }

        keyboard.width = largestWidth + 16
        keyboard.height = y * 25
    }
}
