import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RoundButton {
    id: helpButton
    visible: modelData.helpUrl?.length > 0 || configCategory.length > 0
    display: AbstractButton.IconOnly
    text: qsTr("Open help")
    hoverEnabled: true
    Layout.preferredWidth: 36
    Layout.preferredHeight: 36
    ToolTip.visible: hovered || pressed
    ToolTip.text: text
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    icon.source: app.faUrlBase + "question"
    icon.width: 20
    icon.height: 20
    onClicked: helpButton.desc.length > 0 ? helpDlg.open() : helpButton.openSyncthingDocs()

    Dialog {
        id: helpDlg
        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        title: modelData.label ?? helpButton.key
        modal: true
        width: parent.width - 20
        contentItem: Label {
            text: helpButton.desc
            wrapMode: Text.WordWrap
        }
        footer: DialogButtonBox {
            Button {
                text: qsTr("Close")
                flat: true
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            Button {
                text: qsTr("Details")
                flat: true
                DialogButtonBox.buttonRole: DialogButtonBox.HelpRole
            }
        }
        onHelpRequested: helpButton.openSyncthingDocs()
    }

    property string key: modelData.key
    property string desc: modelData.desc
    property string url: modelData.helpUrl ?? `https://docs.syncthing.net/users/config#${helpButton.configCategory}.${helpButton.key.toLowerCase()}`
    property string configCategory

    function openSyncthingDocs() {
        Qt.openUrlExternally(helpButton.url);
    }
}
