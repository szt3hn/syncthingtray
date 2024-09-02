import QtQuick
import QtQuick.Controls

Label {
    anchors.fill: parent
    text: qsTr("The app has not been built with web view support so this page is not available.")
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    wrapMode: Text.WordWrap
    property list<Action> actions
}
