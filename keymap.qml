import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    id: win
//    color: "orange"
    function setSize(width, height) {
        win.width = width;
        win.height = height;
    }
    Rectangle {
        id: keyboard;
        width: 600;
        height: 200;
//        color: 'blue';

        transform: [
            Scale {
                id: scale;
                xScale: yScale;
                yScale: Math.min(win.width / keyboard.width, win.height / keyboard.height);
            },
            Translate {
                x: (win.width - keyboard.width * scale.xScale) / 2;
                y: (win.height - keyboard.height * scale.yScale) / 2;
            }
        ]

        ListModel {
            id: objectsModel
        }
    }

    Component.onCompleted: {
        var keymap = keymapper.getKeymap()

        var x = 0
        var y = 0
        var largestWidth = 0;

        for(var key in keymap) {
            var num = keymap[key]

            if (num === -1) {
                y++;
                x = 0;
                continue;
            }

            var keyX = 8 * x
            var keyY = 25 * y
            var keyWidth = 8 * num

            var item = Qt.createQmlObject("import QtQuick.Controls 2.3; Button { x: " + keyX + "; y: " + keyY + "; width: " + (keyWidth - 4) + "; height: 20; text: qsTr(\"" + key + "\") }", keyboard, "dynamicItem");

            objectsModel.append(item);
            x += num;

            if (keyX + keyWidth > largestWidth) {
                largestWidth = keyX + keyWidth;
            }
        }

        keyboard.width = largestWidth + 8;
        keyboard.height = y * 25;

//        for(var i=0; i < objectsModel.count; ++i) {
//        }
    }
}
