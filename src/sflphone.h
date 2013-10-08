/***************************************************************************
 *   Copyright (C) 2009-2013 by Savoir-Faire Linux                         *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>         *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>*
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

#ifndef SFLPHONE_H
#define SFLPHONE_H

#include <KXmlGuiWindow>

//Qt
class QString;
class QLabel;
class QTreeWidgetItem;
class QActionGroup;
class QToolButton;

//KDE
class KAction;
class KComboBox;

//SFLPhone
class Call;
class ContactDock;
class BookmarkDock;
class VideoDock;
class SFLPhoneTray;
class SFLPhoneView;
class HistoryDock;
class CallTreeItem;
class VideoRenderer;
class ExtendedAction;
class AccountListNoCheckProxyModel;
class Account;


/**
 * This class represents the SFLphone main window
 * It implements the methods relative to windowing
 * (status, menus, toolbars, notifications...).
 * It uses a view which implements the real functionning
 * and features of the phone.
 * The display of the window is according to the state of the view,
 * so the view sends some signals to ask for changes on the window
 * that the window has to take into account.
 *
 * @short Main window
 * @author Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>
 * @author Emmanuel Lepage <emmanuel.lepage@savoirfairelinux.com>
 * @version 1.2.3
**/
class SFLPhone : public KXmlGuiWindow
{
Q_OBJECT

public:
enum CallAction {
        Accept            ,
        Refuse            ,
        Hold              ,
        Transfer          ,
        Record            ,
        Mailbox           ,
        NumberOfCallActions
};

private:
   //Attributes
   bool   m_pInitialized;
//    ExtendedAction* action_accept         ;
//    ExtendedAction* action_refuse         ;
//    ExtendedAction* action_hold           ;
//    ExtendedAction* action_transfer       ;
//    ExtendedAction* action_record         ;
//    ExtendedAction* action_mute           ;
//    ExtendedAction* action_hangup         ;
//    ExtendedAction* action_unhold         ;
//    ExtendedAction* action_pickup         ;
//    //Video actions
//    #ifdef ENABLE_VIDEO
//    ExtendedAction* action_video_rotate_left    ;
//    ExtendedAction* action_video_rotate_right   ;
//    ExtendedAction* action_video_flip_horizontal;
//    ExtendedAction* action_video_flip_vertical  ;
//    ExtendedAction* action_video_mute           ;
//    ExtendedAction* action_video_preview        ;
//    #endif
//    KAction* action_mailBox               ;
//    KAction* action_close                 ;
//    KAction* action_quit                  ;
//    KAction* action_displayVolumeControls ;
//    KAction* action_displayDialpad        ;
//    KAction* action_displayMessageBox     ;
//    KAction* action_configureSflPhone     ;
//    KAction* action_configureShortcut     ;
//    KAction* action_accountCreationWizard ;
//    KAction* action_pastenumber           ;
//    KAction* action_showContactDock       ;
//    KAction* action_showHistoryDock       ;
//    KAction* action_showBookmarkDock      ;
//    KAction* action_editToolBar           ;
//    QActionGroup* action_screen           ;

   SFLPhoneView*  m_pView            ;
   bool           m_pIconChanged     ;
   SFLPhoneTray*  m_pTrayIcon        ;
   QLabel*        m_pStatusBarWidget ;
   ContactDock*   m_pContactCD       ;
   QDockWidget*   m_pCentralDW       ;
   HistoryDock*   m_pHistoryDW       ;
   BookmarkDock*  m_pBookmarkDW      ;
   KComboBox*     m_pAccountStatus   ;
   #ifdef ENABLE_VIDEO
   VideoDock*     m_pVideoDW         ;
   #endif
   QToolButton*   m_pPresent         ;
   QDockWidget*   m_pPresenceDock    ;

   static SFLPhone*            m_sApp;
   AccountListNoCheckProxyModel* m_pAccountModel;

   //Setters
   void setObjectNames();

protected:
   virtual bool  queryClose (                )      ;
   virtual void  changeEvent( QEvent * event )      ;
   virtual QSize sizeHint   (                ) const;


public:
   explicit SFLPhone(QWidget *parent = 0);
   ~SFLPhone                       ();
   bool             initialize     ();
//    void             setupActions   ();
   void             trayIconSignal ();
//    QList<QAction *> callActions    ();

   friend class SFLPhoneView;

   static SFLPhone*            app   ();
   static SFLPhoneView*        view  ();

   ContactDock*  contactDock ();
   HistoryDock*  historyDock ();
   BookmarkDock* bookmarkDock();

//    ExtendedAction* holdAction    () { return action_hold;     }
//    ExtendedAction* recordAction  () { return action_record;   }
//    ExtendedAction* refuseAction  () { return action_refuse;   }
//    ExtendedAction* muteAction    () { return action_mute;     }
//    ExtendedAction* hangupAction  () { return action_hangup;   }
//    ExtendedAction* unholdAction  () { return action_unhold;   }
//    ExtendedAction* transferAction() { return action_transfer; }
//    ExtendedAction* pickupAction  () { return action_pickup;   }
//    ExtendedAction* acceptAction  () { return action_accept;   }
// 
//    //Video actions
//    #ifdef ENABLE_VIDEO
//    ExtendedAction* videoRotateLeftAction     () { return action_video_rotate_left    ;}
//    ExtendedAction* videoRotateRightAction    () { return action_video_rotate_right   ;}
//    ExtendedAction* videoFlipHorizontalAction () { return action_video_flip_horizontal;}
//    ExtendedAction* videoFlipVerticalAction   () { return action_video_flip_vertical  ;}
//    ExtendedAction* videoMuteAction           () { return action_video_mute           ;}
//    ExtendedAction* videoPreviewAction        () { return action_video_preview        ;}
//    #endif

private Q_SLOTS:
   void on_m_pView_statusMessageChangeAsked      ( const QString& message               );
   void on_m_pView_windowTitleChangeAsked        ( const QString& message               );
   void on_m_pView_enabledActionsChangeAsked     ( const bool*    enabledActions        );
   void on_m_pView_actionIconsChangeAsked        ( const QString* actionIcons           );
   void on_m_pView_actionTextsChangeAsked        ( const QString* actionTexts           );
   void on_m_pView_transferCheckStateChangeAsked ( bool  transferCheckState             );
   void on_m_pView_recordCheckStateChangeAsked   ( bool  recordCheckState               );
   void on_m_pView_incomingCall                  ( const Call*    call                  );
   void currentAccountIndexChanged               ( int newIndex                         );
   void currentPriorAccountChanged               ( Account* newPrior                    );
   void quitButton                               (                                      );
   void updateTabIcons                           (                                      );
   void updatePresence                           ( const QString& status                );
   void hidePresenceDock                         (                                      );
   #ifdef ENABLE_VIDEO
   void displayVideoDock                         ( VideoRenderer* r                     );
   #endif

public Q_SLOTS:
   void timeout                                  (                                      );
};

#endif
