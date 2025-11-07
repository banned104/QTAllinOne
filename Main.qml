import QtQuick
import HuskarUI.Basic
import lib.OpenGLItem 1.0
import "src/QML_Files/Buttons"

HusWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Rectangle{
        id: rect1;
        anchors.fill: parent;
        color: "grey";
        HusButton {
            id: mainButton;
            width: 200; height: 50;
            anchors.centerIn: parent;
            text: "HuskarUI";
        }


        Rectangle {
            border.width: 2;
            border.color: "red";
            width: 100; height: 100;
            anchors.top: mainButton.bottom;
            anchors.margins: 10;
            anchors.horizontalCenter: mainButton.horizontalCenter;
            OpenGLItem {
                visible: true;
                anchors.fill: parent;
            }
        }
    }

    ThreeDSwitch {
        anchors.top: rect1.bottom;
        anchors.topMargin: 10;
    }

}
