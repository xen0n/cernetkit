import QtQuick 2.2
import QtQuick.Window 2.1
import QtGraphicalEffects 1.0


Window {
    id: rootWindow

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    color: "#00000000"

    Item {
        id: backgroundContainer
        anchors.fill: parent
        // visible: false

        Rectangle {
            anchors.fill: parent
            anchors.margins: 8

            border.color: "#666666"
            border.width: 1.0
            color: "#eedddddd"
            radius: 4

            MouseArea {
                anchors.fill: parent

                property point clickPos: Qt.point(1, 1)

                onPressed: {
                    clickPos = Qt.point(mouse.x, mouse.y);
                }

                onPositionChanged: {
                    var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y);

                    rootWindow.x += delta.x;
                    rootWindow.y += delta.y;
                }
            }

            Text {
                id: gwDisplay
                text: gw
                anchors.centerIn: parent
            }

            Rectangle {
                id: btnExit
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: gwDisplay.bottom
                anchors.topMargin: 8

                width: 100
                height: 30
                radius: 4

                color: "#999999"
                border.color: "#666666"
                border.width: 1.0

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onEntered: {
                        parent.color = "#666666";
                    }

                    onExited: {
                        parent.color = "#999999";
                    }

                    onClicked: {
                        Qt.quit();
                    }
                }

                Text {
                    anchors.centerIn: parent
                    font.pointSize: 14
                    color: "#ffffff"

                    text: qsTr("Exit")
                }
            }
        }
    }

    DropShadow {
        anchors.fill: backgroundContainer

        color: "#66000000"
        horizontalOffset: 2
        verticalOffset: 2
        radius: 8.0
        samples: 16

        source: backgroundContainer
    }
}
