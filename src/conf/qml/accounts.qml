/***************************************************************************
*   Copyright (C) 2017 by Bluesystems                                     *
*   Author : Emmanuel Lepage Vallee <elv1313@gmail.com>                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
**************************************************************************/
import QtQuick 2.8
import org.kde.kirigami 2.5 as Kirigami
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import org.kde.playground.kquickview 1.0 as KQuickItemViews

Rectangle {
    property var profileModel: null
    property QtObject selectedAccount: nullptr

    SystemPalette {
        id: activePalette
        colorGroup: SystemPalette.Active
    }

    color: activePalette.window

    Kirigami.CardsListView {
        id: gridView

        Component {
            id: codecDelegate
            Kirigami.Card {
                id: card
                width: gridView.width
                height: content.height + 20
                ColumnLayout {
                    id: content
                    height: implicitHeight
                    width: parent.width
                    Kirigami.Heading {
                        text: display
                        level:4
                    }

                    ListView {
                        id: profileList
                        interactive: false
                        model: profileModel.profilesForPerson(object)
                        implicitHeight: contentHeight
                        height: contentHeight
                        width: parent.width

                        delegate: Kirigami.SwipeListItem {
                            id: listItem
                            width: parent.width
                            background: Rectangle {
                                id: background
                                property var listItem: parent

                                color: listItem.checked ||
                                    listItem.highlighted || (
                                        listItem.supportsMouseEvents
                                        && listItem.pressed
                                        && !listItem.checked
                                        && !listItem.sectionDelegate
                                    ) ? listItem.activeBackgroundColor : listItem.backgroundColor

                                visible: listItem.ListView.view ? listItem.ListView.view.highlight === null : true

                                Rectangle {
                                    id: internal
                                    property bool indicateActiveFocus: listItem.pressed || listItem.activeFocus || (listItem.ListView.view ? listItem.ListView.view.activeFocus : false)
                                    anchors.fill: parent
                                    visible: !listItem.supportsMouseEvents
                                    color: listItem.activeBackgroundColor
                                    opacity: (listItem.hovered || listItem.highlighted || listItem.activeFocus) && !listItem.pressed ? 0.5 : 0
                                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration } }
                                }



                                Rectangle {
                                    opacity: 0.9
                                    anchors.fill: parent
                                    color: registrationStateColor
                                }

                                KQuickItemViews.DecorationAdapter {
                                    width: check.implicitHeight
                                    height: check.implicitHeight
                                    anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    pixmap: securityLevelIcon
                                }

                                readonly property bool __separatorVisible: listItem.separatorVisible

                                on__SeparatorVisibleChanged: {
                                    if (__separatorVisible) {
                                        var newObject = Qt.createQmlObject('import QtQuick 2.0; import org.kde.kirigami 2.4; Separator {anchors {left: parent.left; right: parent.right; bottom: parent.top} visible: listItem.separatorVisible}',
                                                            background);
                                        newObject = Qt.createQmlObject('import QtQuick 2.0; import org.kde.kirigami 2.4; Separator {anchors {left: parent.left; right: parent.right; bottom: parent.bottom} visible: listItem.separatorVisible}',
                                                            background);
                                    }
                                }
                            }
                            RowLayout {
                                width: parent.width
                                height: check.implicitHeight*2
                                CheckBox {
                                    id: check
                                    width: height
                                    checked: model.enabled
                                }

                                KQuickItemViews.DecorationAdapter {
                                    height: check.implicitHeight
                                    width: check.implicitHeight
                                    pixmap: decoration
                                }

                                Label {
                                    text: alias
                                    Layout.fillWidth: true
                                }
                            }
                            actions: [
                                Kirigami.Action {
                                    text: i18n("Remove")
                                    iconName: "list-remove"
                                    onTriggered: {}
                                }
                            ]

                            onPressedChanged: {
                                selectedAccount = object
                            }
                        }
                    }
                }
            }
        }

        property var foo: test

        footer: OutlineButton {
            height: 50
            label: i18n("Add an account")
            width: parent.width
        }

        header: KQuickItemViews.IndexView {
            id: test
            modelIndex: selectedAccount ? selectedAccount.index : undefined
            height: 100
            width: parent.width

            delegate: Rectangle {
                color: "red"
                height: 70
                width: 200//parent.width
                ColumnLayout {
                    anchors.fill: parent
                    Text {
                        color: "blue"
                        text: alias
                    }
                    KQuickItemViews.QModelIndexBinder {
                        height: aliasTxt.implicitHeight
                        width: parent.width
                        modelRole: "alias"
                        objectProperty: "text"
                        TextField {
                            id: aliasTxt
                            width: parent.width
                        }
                    }
                }
            }
        }

        anchors.fill: parent
        model: profileModel
        delegate: codecDelegate
    }
}
