/*
 *   Copyright 2018 Fabian Riethmayer
 *   Copyright 2019 Emmanuel Lepage <emmanuel.lepage@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 3, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.6 as Kirigami
import net.lvindustries.ringqtquick.media 1.0 as RingQtMedia
import net.lvindustries.ringqtquick 1.0 as RingQtQuick
import org.kde.ringkde.basicview 1.0 as BasicView
import org.kde.ringkde.jamicontactview 1.0 as JamiContactView
import org.kde.ringkde.jamiwizard 1.0 as JamiWizard
import org.kde.ringkde.jamikdeintegration 1.0 as JamiKDEIntegration

Kirigami.ApplicationWindow {
    /*
     * Default to 4:3 because it has a nice balance between landscape and
     * portrait aspect ratios.
     */
    width: 1024; height: 768
    pageStack.defaultColumnWidth: width < 320 ? width : 320

    /*
     * The call page is only in the stack when it is needed.
     */
    pageStack.initialPage: [list, chat]

    /*
     * Always use the toolbar mode because otherwise the chatbox and call
     * toolbars gets covered by the Kirigami navigation widgets.
     */
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
    pageStack.globalToolBar.preferredHeight: Kirigami.Units.gridUnit * 3

    // Localization is currently broken with the Material theme
    //TODO check if the theme is Material before "fixing" i18n
    function i18n(t) {return t;}

    function showCallPage() {
        callpage.visible = true

        if (pageStack.currentItem == callpage)
            return

        showChat()

        for (var i = 0; i < pageStack.depth; i++) {
            if (pageStack.get(i) == callpage) {
                pageStack.currentIndex = i
                return
            }
        }
        pageStack.push(callpage)
    }

    function hideCall() {
        for (var i = 0; i < pageStack.depth; i++) {
            if (pageStack.get(i) == callpage) {
                pageStack.pop(callpage)
                pageStack.currentIndex = 0
                return
            }
        }
    }

    function showChat() {
        if (pageStack.currentItem == callpage)
            return

        for (var i = 0; i < pageStack.depth; i++) {
            if (pageStack.get(i) == chat) {
                pageStack.currentIndex = Kirigami.Settings.isMobile ? 1 : 0
                return
            }
        }

        pageStack.push(chat)
        pageStack.currentIndex = Kirigami.Settings.isMobile ? 1 : 0
    }

    /*
     * Track the network status and other information sources to ensure
     * actions *can* work at any given time
     */
    RingQtMedia.AvailabilityTracker {
        id: availabilityTracker
        individual: workflow.currentIndividual
    }

    /*
     * This implements the workflow of having a single "current" Individual
     * (abstract contact) at once. It hold some references to all the shared
     * pointers to ensure QtQuick don't accidentally let them be freed.
     */
    RingQtQuick.SharedModelLocker {
        id: workflow

        onCallChanged: {
            if (!call)
                hideCall()
            else
                showCallPage()
        }

        onIndividualChanged: {
            list.currentIndex = RingSession.peersTimelineModel.individualIndex(
                currentIndividual
            ).row
        }
    }

    /*
     * This is the QtQuick action collection, there is also:
     *
     *          org.kde.ringkde.jamikdeintegration.actioncollection
     *
     * The difference is that this one has the actions that can be implemented
     * in QML easily while the other one has the ones where the check and
     * enabled state depends on the backend state and thus are better
     * implemented in C++.
     */
    BasicView.ActionCollection {
        id: actionCollection
    }

    /*
     * This is a fixed non-expanded column on the left side with a list.
     *
     * Currently, it always display the timeline, but maybe eventually the other
     * will be supported too:
     *
     *  * Peers timeline
     *  * Contacts
     *  * History
     *  * Bookmarks
     *  * Active calls and conferences
     *
     */
    BasicView.ListPage {
        id: list
    }

    /*
     * This page holds the chat and "per individual" timeline.
     *
     * It also has a sidebar with some actions and statistics to perform on
     * the individual (abstract contact).
     */
    BasicView.ChatPage {
        id: chat
    }

    /*
     * This is the multimedia page.
     *
     * It is used for audio, video and screencast communication. It is only in
     * the PageRow stack when explicitly selected using a QmlAction or when
     * there is an incoming (or active) communication.
     */
    BasicView.CallPage {
        id: callpage
        visible: false
    }

    // Each page provide their actions
    contextDrawer: Kirigami.ContextDrawer {}

    globalDrawer: BasicView.GlobalDrawer {
        /*
         * Without this the handle might cover the "back" button or allow to
         * open the wizard from within the wizard, which "works" but is
         * obviously bad.
         */
        handleVisible: !wizardLoader.active
    }

    /*
     * This is a "wormhole" object where the signals sent by any instance,
     * including in C++, ends up here.
     *
     * This is used as an abstraction to integrate the C++ backend with the QML
     * frontend without any setContextProperty.
     */
    JamiKDEIntegration.WindowEvent {
        id: events

        function showDialog(path) {
            var component = Qt.createComponent(path)
            if (component.status == Component.Ready) {
                var window = component.createObject(applicationWindow().contentItem)
                window.open()
            }
            else
                console.log("ERROR", component.status, component.errorString())
        }

        // Dialogs and overlays
        onRequestsConfigureAccounts: showDialog("qrc:/account/qml/accountdialog.qml")
        onRequestsVideo: showDialog("qrc:/jamivideoview/qml/settingpopup.qml")
        onRequestsWizard: wizardLoader.activate()

        // Window events
        onRequestsHideWindow: hide()
    }

    /*
     * This object tracks when showing the wizard is required. Unlike Ring-KDE,
     * Banji cannot work without an account and all component blindly expect
     * one to exist.
     *
     * In Ring-KDE, having no account was requested (and implemented), but this
     * made everything harder to maintain with long QML expressions checking
     * everything, everywhere. It was a mistake and Banji will *never* support
     * having no account.
     */
    JamiWizard.Policies {
        id: wizardPolicies
    }

    /*
     * Do common operations in a wizard instead of the settings.
     *
     * It is less error prone.
     */
    JamiWizard.Wizard {
        id: wizardLoader
        anchors.fill: parent

        onActiveChanged: {
            if (list.displayWelcome && !active)
                list.search()
        }
    }

    /*
     * View the current individual information.
     *
     * This isn't intended as a full contact manager, only an overview of the
     * relevant information. It is partially redundant with the sidebar.
     *
     * Please note that the sidebar only exists in wide mode with enough room to
     * "afford" it. So this information needs to be available as an overlay too.
     */
    BasicView.IndividualDetails {
        id: viewContact
    }

    /*
     * Edit the current individual details.
     *
     * Like the `viewContact` above, this is partially redundant with the
     * sidebar. As with `viewContact`, this is due to the fact that the sidebar
     * isn't always there.
     *
     * It is also important to note that this will create a "real" contact
     * (LibRingQt.Person) object. For as long as possible, the peer will exist
     * as an LibRingQt.Individual object. It is a lower level abstraction and
     * it is easier to synchronize because it doesn't have a custom vCard.
     */
    BasicView.IndividualEditor {
        id: editContact
    }

    /*
     * Delay showing the wizard or welcome page until the next event loop
     * iteration. This avoids having to handle a whole bunch of corner cases.
     *
     * It is important to keep in mind that net.lvindustries.ringqtquick is
     * loaded by the QML files, not ahead of time. So during the first iteration,
     * it isn't fully ready yet.
     */
    Timer {
        interval: 0
        running: true
        repeat: false
        onTriggered: {
            if (wizardPolicies.displayWizard)
                wizardLoader.activate()
            else if (list.displayWelcome)
                list.search()
        }
    }

}
