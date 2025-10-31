import QtQuick
import HuskarUI.Basic

HusWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    HusButton {
        id: mainButton;
        width: 200; height: 50;
        anchors.centerIn: parent;
        text: "HuskarUI";
    }

    // HusCard{
    //     width: 200; height: 50;
    //     anchors.top: mainButton.bottom;
    //     anchors.topMargin: 10;
    // }

}
