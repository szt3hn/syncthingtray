import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

ApplicationWindow {
    id: window
    visible: true
    width: 700
    height: 500
    title: qsTr("Syncthing App")
    Material.theme: app.darkmodeEnabled ? Material.Dark : Material.Light
    Material.accent: Material.LightBlue
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: flickable.anchors.leftMargin
            ToolButton {
                visible: !backButton.visible
                icon.source: app.faUrlBase + "bars"
                icon.width: iconSize
                icon.height: iconSize
                onClicked: drawer.visible ? drawer.close() : drawer.open()
                ToolTip.text: qsTr("Toggle menu")
                ToolTip.visible: hovered || pressed
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            }
            ToolButton {
                id: backButton
                visible: pageStack.currentDepth > 1
                icon.source: app.faUrlBase + "chevron-left"
                icon.width: iconSize
                icon.height: iconSize
                onClicked: pageStack.pop()
                ToolTip.text: qsTr("Back")
                ToolTip.visible: hovered || pressed
                ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
            }
            Label {
                text: pageStack.currentPage.title
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            Repeater {
                model: pageStack.currentActions
                ToolButton {
                    required property Action modelData
                    enabled: modelData.enabled
                    ToolTip.visible: hovered || pressed
                    ToolTip.text: modelData.text
                    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
                    icon.source: modelData.icon.source
                    icon.width: iconSize
                    icon.height: iconSize
                    onClicked: modelData.trigger()
                }
            }
            ToolButton {
                visible: pageStack.currentExtraActions.length > 0
                icon.source: app.faUrlBase + "bars"
                icon.width: iconSize
                icon.height: iconSize
            }
        }
    }

    readonly property bool inPortrait: window.width < window.height
    readonly property int spacing: 7
    readonly property int iconSize: 16

    AboutDialog {
        id: aboutDialog
    }

    Drawer {
        id: drawer
        width: Math.min(0.66 * window.width, 200)
        height: window.height
        interactive: inPortrait || window.width < 600
        modal: interactive
        position: initialPosition
        visible: !interactive

        readonly property double initialPosition: interactive ? 0 : 1
        readonly property int effectiveWidth: !interactive ? width : 0

        ListView {
            id: drawerListView
            anchors.fill: parent
            footer: ItemDelegate {
                width: parent.width
                text: Qt.application.version
                icon.source: app.faUrlBase + "info-circle"
                icon.width: iconSize
                icon.height: iconSize
                onClicked: aboutDialog.visible = true
            }
            model: ListModel {
                ListElement {
                    name: qsTr("Folders")
                    iconName: "folder"
                }
                ListElement {
                    name: qsTr("Devices")
                    iconName: "sitemap"
                }
                ListElement {
                    name: qsTr("Recent changes")
                    iconName: "history"
                }
                ListElement {
                    name: qsTr("Syncthing")
                    iconName: "syncthing"
                }
                ListElement {
                    name: qsTr("Advanced")
                    iconName: "cogs"
                }
                ListElement {
                    name: qsTr("App settings")
                    iconName: "cog"
                }
            }
            delegate: ItemDelegate {
                text: name
                icon.source: app.faUrlBase + iconName
                icon.width: iconSize
                icon.height: iconSize
                width: parent.width
                onClicked: {
                    drawerListView.currentIndex = index
                    drawer.position = drawer.initialPosition
                }
            }
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.leftMargin: drawer.visible ? drawer.effectiveWidth : 0

        StackLayout {
            id: pageStack
            anchors.fill: parent
            currentIndex: drawerListView.currentIndex
            DirsPage {
            }
            DevsPage {
            }
            ChangesPage {
            }
            WebViewPage {
            }
            AdvancedPage {
            }
            SettingsPage {
            }

            readonly property var currentPage: {
                const currentChild = children[currentIndex];
                return currentChild.currentItem ?? currentChild;
            }
            readonly property var currentDepth: children[currentIndex]?.depth ?? 1
            readonly property var currentActions: currentPage.actions ?? []
            readonly property var currentExtraActions: currentPage.extraActions ?? []
            function pop() { children[currentIndex].pop?.() }
        }
    }

    ToolTip {
        anchors.centerIn: Overlay.overlay
        id: errorToolTip
        timeout: 5000
    }
    Connections {
        target: app.connection
        function onError(message) {
            showError(message);
         }
    }
    Connections {
        target: app.notifier
        function onDisconnected() {
            showError(qsTr("UI disconnected from Syncthing backend"));
         }
    }
    function showError(message) {
        errorToolTip.text = message;
        errorToolTip.open()
    }
}
