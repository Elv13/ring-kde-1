/************************************** ************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                         *
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

//Parent
#include "mainwindow.h"

//System
#include <unistd.h>

//Qt
#include <QtCore/QString>
#include <QtWidgets/QLabel>
#include <QtCore/QTimer>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QComboBox>

//KDE
#include <KNotification>
#include <KToolBar>
#include <kmessagebox.h>

#include <klocalizedstring.h>
#include <QIcon>
#include <QStandardPaths>

//Models
#include <accountmodel.h>
#include <profilemodel.h>
#include <certificatemodel.h>
#include <availableaccountmodel.h>
#include <numbercategorymodel.h>
#include <media/recordingmodel.h>
#include <presencestatusmodel.h>
#include <personmodel.h>
#include <macromodel.h>
#include <categorizedbookmarkmodel.h>
// #include <recentmodel.h>

//Collections
#include <foldercertificatecollection.h>
#include <fallbackpersoncollection.h>
#include <localhistorycollection.h>
#include <localbookmarkcollection.h>
#include <localrecordingcollection.h>

//Configurators
#include "configurator/localhistoryconfigurator.h"
#include "configurator/audiorecordingconfigurator.h"
#include "configurator/fallbackpersonconfigurator.h"

//Delegates
#include <delegates/kdepixmapmanipulation.h>
#include <interfaces/itemmodelstateserializeri.h>
#include "klib/itemmodelserialization.h"
#include "extensions/presencecollectionextension.h"
#include "klib/kdeprofilepersistor.h"

//Ring
#include "klib/kcfg_settings.h"
#include "icons/icons.h"
#include "view.h"
#include "dock.h"
#include <globalinstances.h>
#include "conf/account/widgets/autocombobox.h"
#include "actioncollection.h"
#include "widgets/systray.h"
#include "widgets/presence.h"
#include "accessibility.h"
#include "errormessage.h"
#include <video/renderer.h>
#include "ringapplication.h"
#include "widgets/dockbase.h"
#ifdef ENABLE_VIDEO
#include "widgets/videodock.h"
#endif

MainWindow* MainWindow::m_sApp = nullptr;

static void loadNumberCategories()
{
//    QList<int> list = ConfigurationSkeleton::phoneTypeList();
//    const bool isEmpty = !list.size();
// #define IS_ENABLED(name) (list.indexOf(name) != -1) || isEmpty
   auto& model = NumberCategoryModel::instance();
#define ICN(name) QPixmap(QString(":/mini/icons/miniicons/%1.png").arg(name))
   model.addCategory(i18n("Home")     ,ICN("home")     , 1 /*KABC::PhoneNumber::Home */);
   model.addCategory(i18n("Work")     ,ICN("work")     , 2 /*KABC::PhoneNumber::Work */);
   model.addCategory(i18n("Msg")      ,ICN("mail")     , 3 /*KABC::PhoneNumber::Msg  */);
   model.addCategory(i18n("Pref")     ,ICN("call")     , 4 /*KABC::PhoneNumber::Pref */);
   model.addCategory(i18n("Voice")    ,ICN("video")    , 5 /*KABC::PhoneNumber::Voice*/);
   model.addCategory(i18n("Fax")      ,ICN("call")     , 6 /*KABC::PhoneNumber::Fax  */);
   model.addCategory(i18n("Cell")     ,ICN("mobile")   , 7 /*KABC::PhoneNumber::Cell */);
   model.addCategory(i18n("Video")    ,ICN("call")     , 8 /*KABC::PhoneNumber::Video*/);
   model.addCategory(i18n("Bbs")      ,ICN("call")     , 9 /*KABC::PhoneNumber::Bbs  */);
   model.addCategory(i18n("Modem")    ,ICN("call")     , 10/*KABC::PhoneNumber::Modem*/);
   model.addCategory(i18n("Car")      ,ICN("car")      , 11/*KABC::PhoneNumber::Car  */);
   model.addCategory(i18n("Isdn")     ,ICN("call")     , 12/*KABC::PhoneNumber::Isdn */);
   model.addCategory(i18n("Pcs")      ,ICN("call")     , 13/*KABC::PhoneNumber::Pcs  */);
   model.addCategory(i18n("Pager")    ,ICN("pager")    , 14/*KABC::PhoneNumber::Pager*/);
   model.addCategory(i18n("Preferred"),ICN("preferred"), 10000                         );
#undef ICN
#undef IS_ENABLED
}

///Constructor
MainWindow::MainWindow(QWidget* parent)
   : KXmlGuiWindow(parent), m_pInitialized(false)
#ifdef ENABLE_VIDEO
      ,m_pVideoDW(nullptr)
#endif
{

   //On OSX, QStandardPaths doesn't work as expected, it is better to pack the .ui in the DMG
#ifdef Q_OS_MAC
   QDir dir(QApplication::applicationDirPath());
   dir.cdUp();
   dir.cd("Resources/kxmlgui5/ring-kde/");
   setXMLFile(dir.path()+"/ring-kdeui.rc");
   setUnifiedTitleAndToolBarOnMac(true);
#endif

   if ((!CallModel::instance().isConnected()) || (!CallModel::instance().isValid())) {
      QTimer::singleShot(5000,this,SLOT(timeout()));
   }
   static bool init = false;
   if (!init) {
      GlobalInstances::setInterface<KDEProfilePersister>();

      GlobalInstances::setInterface<KDEPixmapManipulation>();

      loadNumberCategories();

      /*******************************************
       *           Set the configurator          *
       ******************************************/

      PersonModel::instance()            .registerConfigarator<FallbackPersonCollection>(new FallbackPersonConfigurator(this));
      Media::RecordingModel::instance()  .registerConfigarator<LocalRecordingCollection>(new AudioRecordingConfigurator(this));
      CategorizedHistoryModel::instance().registerConfigarator<LocalHistoryCollection  >(new LocalHistoryConfigurator  (this));

      /*******************************************
       *           Load the collections          *
       ******************************************/

      CategorizedHistoryModel::instance().addCollection<LocalHistoryCollection>(LoadOptions::FORCE_ENABLED);

#ifdef Q_OS_LINUX
      CertificateModel::instance().addCollection<FolderCertificateCollection,QString, FlagPack<FolderCertificateCollection::Options>,QString>(
         QString("/usr/share/ca-certificates/"),
         FolderCertificateCollection::Options::ROOT
          | FolderCertificateCollection::Options::RECURSIVE
          | FolderCertificateCollection::Options::READ_ONLY,
         QObject::tr("System root certificates"),
         LoadOptions::FORCE_ENABLED
      );
#endif

      CategorizedBookmarkModel::instance().addCollection<LocalBookmarkCollection>();
      CategorizedBookmarkModel::instance().reloadCategories();

      PersonModel::instance().addCollection<FallbackPersonCollection>(LoadOptions::FORCE_ENABLED);

      GlobalInstances::setInterface<ItemModelStateSerialization>();
//       PersonModel::instance().backendModel()->load();
//       AccountModel::instance().setDefaultAccount(AccountModel::instance().getAccountById(ConfigurationSkeleton::defaultAccountId()));

      init = true;

//       PresenceCollectionModelExtension* ext = new PresenceCollectionModelExtension(this);
//       PersonModel::instance().backendModel()->addExtension(ext); //FIXME

      ProfileModel::instance();
   }

   //Belong to setupActions(), but is needed now
   m_sApp = this;
   setWindowIcon ( QIcon(":/appicon/icons/sc-apps-ring-kde.svgz") );
   setWindowTitle( i18n("Ring"                                  ) );

   m_pView = new View(this);
   ActionCollection::instance()->setupAction();
   m_pView->updateVolumeControls();

   ConfigurationSkeleton::self();

   // accept dnd
   setAcceptDrops(true);

   // tell the KXmlGuiWindow that this is indeed the main widget
   m_pCentralDW = new QDockWidget(this);
   m_pCentralDW->setSizePolicy  ( QSizePolicy::Expanding,QSizePolicy::Expanding );
   m_pCentralDW->setWidget      ( m_pView                                       );
   m_pCentralDW->setWindowTitle ( i18nc("Call tab","Call")                      );
   m_pCentralDW->setFeatures    ( QDockWidget::NoDockWidgetFeatures             );
   m_pView->setSizePolicy       ( QSizePolicy::Expanding,QSizePolicy::Expanding );
   m_pCentralDW->setStyleSheet  ( "\
      QDockWidget::title {\
         margin:0px;\
         padding:0px;\
         spacing:0px;\
         max-height:0px;\
      }\
      \
   ");

   m_pCentralDW->setTitleBarWidget(new QWidget());
   m_pCentralDW->setContentsMargins(0,0,0,0);
   m_pView->setContentsMargins     (0,0,0,0);
   addDockWidget( Qt::BottomDockWidgetArea, m_pCentralDW  );
   m_pCentralDW->setObjectName( "callDock" );
   m_pCentralDW->show();

   m_pStatusBarWidget = new QLabel       ( this );

   //System tray
   m_pTrayIcon        = new SysTray ( this->windowIcon(), this );

   m_pDock = new Dock(this);
   connect(m_pCentralDW ,SIGNAL(visibilityChanged(bool)),m_pDock,SLOT(updateTabIcons()));

   selectCallTab();
   setAutoSaveSettings();
   createGUI();

   m_pTrayIcon->addAction( ActionCollection::instance()->acceptAction  () );
   m_pTrayIcon->addAction( ActionCollection::instance()->mailBoxAction () );
   m_pTrayIcon->addAction( ActionCollection::instance()->holdAction    () );
   m_pTrayIcon->addAction( ActionCollection::instance()->transferAction() );
   m_pTrayIcon->addAction( ActionCollection::instance()->recordAction  () );
   m_pTrayIcon->addSeparator();
   m_pTrayIcon->addAction( ActionCollection::instance()->quitAction    () );

   connect(CallModel::instance().selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(selectCallTab()));

#ifdef ENABLE_VIDEO
   connect(&CallModel::instance(),&CallModel::rendererAdded,this,&MainWindow::displayVideoDock);
   connect(&CallModel::instance(),&CallModel::rendererRemoved,this,&MainWindow::hideVideoDock);
#endif

   statusBar()->addWidget(m_pStatusBarWidget);

   m_pTrayIcon->show();

   m_pView->setObjectName      ( "m_pView"       );
   statusBar()->setObjectName  ( "statusBar"     );
   m_pTrayIcon->setObjectName  ( "m_pTrayIcon"   );

   m_pInitialized = true ;

   //Add the toolbar Ring icon
   QList<KToolBar*> toolBars = this->findChildren<KToolBar*>();
   if (toolBars.size()) {
      KToolBar* tb = toolBars[0];
      QWidget* spacer = new QWidget(tb);
      spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
      tb->addWidget(spacer);
      QToolButton* btn = new QToolButton(tb);
      btn->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding);
      btn->setStyleSheet("padding:0px;spacing:0px;margin:0px;");
      btn->setIcon(QIcon(":/appicon/icons/sc-apps-ring-kde.svgz"));
      tb->addWidget(btn);
   }

   //Statusbar
   QStatusBar* bar = statusBar();
   int left,top,right,bottom;
   bar->layout()->getContentsMargins ( &left, &top, &right, &bottom );
   bar->layout()->setContentsMargins(0,top,0,bottom);

   //The presence button act as the label, remove the original one
   QList<QLabel*> labels = bar->findChildren<QLabel*>();
   for (int i=0;i<labels.size();i++) {
      bar->layout()->removeWidget(labels[i]);
      labels[i]->setHidden(true);
   }

   m_pPresent = new QToolButton(bar);
   m_pPresent->setAutoRaise(true);
   m_pPresent->setText(i18nc("The presence state is \"Online\"","Online"));
   m_pPresent->setCheckable(true);
   m_pPresent->setVisible(false/*AccountModel::instance().isPresenceEnabled() && AccountModel::instance().isPresencePublishSupported()*/);
//    m_pPresent->setStyleSheet("background-color:red;");
   bar->addWidget(m_pPresent);
   connect(&PresenceStatusModel::instance(),SIGNAL(currentNameChanged(QString)),this,SLOT(updatePresence(QString)));
   connect(&PresenceStatusModel::instance(),SIGNAL(currentNameChanged(QString)),this,SLOT(hidePresenceDock()));
   connect(&AccountModel::instance(),SIGNAL(presenceEnabledChanged(bool)),this,SLOT(slotPresenceEnabled(bool)));

   //Add the Ring hash
   bar->addWidget(new QLabel(i18n("Your Ring ID:"),bar));
   QLabel* ringId = new QLabel(bar);
   QFont f = ringId->font();
   f.setStyleHint(QFont::Monospace);
   ringId->setFont(f);
   ringId->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
   Account* a = AvailableAccountModel::instance().currentDefaultAccount(URI::SchemeType::RING);
   ringId->setText(a ? a->username() : i18n("None"));
   bar->addWidget(ringId);

   m_pPresenceDock = new QDockWidget(this);
   m_pPresenceDock->setObjectName("presence-dock");
   m_pPresenceDock->setWidget(new Presence(m_pPresenceDock));
   m_pPresenceDock->setAllowedAreas(Qt::TopDockWidgetArea);
   m_pPresenceDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
   addDockWidget( Qt::TopDockWidgetArea, m_pPresenceDock );
   m_pPresenceDock->setVisible(false);
   if (m_pPresenceDock->titleBarWidget())
      m_pPresenceDock->titleBarWidget()->setVisible(false);

   m_pCurAccL = new QLabel(i18n("Account:"));
   bar->addPermanentWidget(m_pCurAccL);

   m_pAccountStatus = new AutoComboBox(bar);
   m_pAccountStatus->setVisible(ConfigurationSkeleton::displayAccountBox());
   m_pCurAccL->setVisible(ConfigurationSkeleton::displayAccountBox());
   m_pAccountStatus->bindToModel(&AvailableAccountModel::instance(),AvailableAccountModel::instance().selectionModel());
   m_pAccountStatus->setMinimumSize(100,0);
   bar->addPermanentWidget(m_pAccountStatus);
   connect(m_pPresent,SIGNAL(toggled(bool)),m_pPresenceDock,SLOT(setVisible(bool)));

   QToolButton* m_pReloadButton = new QToolButton(this);
   m_pReloadButton->setIcon(QIcon::fromTheme("view-refresh"));
   bar->addPermanentWidget(m_pReloadButton);
   connect(m_pReloadButton,SIGNAL(clicked()),&AccountModel::instance(),SLOT(registerAllAccounts()));

   if (!CallModel::instance().isValid()) {
      KMessageBox::error(this,i18n("The Ring daemon (dring) is not available. Please be sure it is installed correctly or launch it manually"));
      QTimer::singleShot(2500,this,SLOT(timeout())); //FIXME this may leave the client in an unreliable state
      //exit(1); //Don't try to exit normally, it will segfault, the application is already in a broken state if this is reached //BUG break some slow netbooks
   }

   if (ConfigurationSkeleton::displayOnStart()
      && !qobject_cast<RingApplication*>(QApplication::instance())->startIconified())
      show();
   else
      close();

   //setupGui + default size doesn't really, use this for now
   resize(QSize(1024,768));

   if (!ConfigurationSkeleton::autoStartOverride())
      setAutoStart(true);

//    RecentModel::instance();

} //Ring

///Destructor
MainWindow::~MainWindow()
{
   delete m_pDock;

   delete m_pView            ;
   delete m_pTrayIcon        ;
   delete m_pStatusBarWidget ;
   delete m_pCentralDW       ;
   delete m_pPresent         ;
   delete m_pPresenceDock    ;

   delete &CallModel::instance();
   delete &PersonModel::instance();
   //saveState();
}

///Init everything
bool MainWindow::initialize() //TODO deprecate
{
   return !m_pInitialized;
}


/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

///Singleton
MainWindow* MainWindow::app()
{
   return m_sApp;
}

///Get the view (to be used with the singleton)
View* MainWindow::view()
{
   return app()->m_pView;
}

/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

///[Action]Hide Ring
bool MainWindow::queryClose()
{
   if (!isHidden()) {
      ConfigurationSkeleton::setDisplayContactDock ( m_pDock->contactDock()->isVisible()  );
      ConfigurationSkeleton::setDisplayHistoryDock ( m_pDock->historyDock()->isVisible()  );
      ConfigurationSkeleton::setDisplayBookmarkDock( m_pDock->bookmarkDock()->isVisible() );
   }
   hide();
   return false;
}

///Be sure the default size is look like a phone
QSize MainWindow::sizeHint() const
{
   return QSize(340,700);
}

///[Action] Quit action
void MainWindow::quitButton()
{
   qApp->quit();
}

void MainWindow::displayAccountCbb( bool checked )
{
   m_pAccountStatus->setVisible(checked);
   m_pCurAccL->setVisible(checked);
}

///Called when a call is coming
// void MainWindow::onIncomingCall(const Call* call) //FIXME
// {
   //FIXME create an infinite loop
//    if (call) {
      /*const Person* contact = call->peerContactMethod()->contact();
      if (contact) {
         const QPixmap px = (contact->photo()).type() == QVariant::Pixmap ? (contact->photo()).value<QPixmap>():QPixmap();
         KNotification::event(KNotification::Notification, i18n("New incoming call"), i18n("New call from:\n%1",call->peerName().isEmpty() ? call->peerContactMethod()->uri() : call->peerName()),px);
      }
      else
         KNotification::event(KNotification::Notification, i18n("New incoming call"), i18n("New call from:\n%1",call->peerName().isEmpty() ? call->peerContactMethod()->uri() : call->peerName()));
   *///}
// }

///Hide or show the statusbar presence widget
void MainWindow::slotPresenceEnabled(bool state)
{
   m_pPresent->setVisible(state && AccountModel::instance().isPresencePublishSupported());
}

///Update presence label
void MainWindow::updatePresence(const QString& status)
{
   m_pPresent->setText(status);
   m_pPresent->setToolTip(PresenceStatusModel::instance().currentMessage());
}

///Hide the presence dock when not required
void MainWindow::hidePresenceDock()
{
   m_pPresent->setChecked(false);
}

#ifdef ENABLE_VIDEO
///Display the video dock
void MainWindow::displayVideoDock(Call* c, Video::Renderer* r)
{
   Q_UNUSED(c)

   if (!m_pVideoDW) {
      m_pVideoDW = new VideoDock(this);
      addDockWidget( Qt::TopDockWidgetArea, m_pVideoDW  );
   }
   m_pVideoDW->addRenderer(r);
   m_pVideoDW->show();
}

void MainWindow::hideVideoDock(Call* c, Video::Renderer* r)
{
   Q_UNUSED(c)
   Q_UNUSED(r)
   if (m_pVideoDW) {
      m_pVideoDW->hide();
   }
}
#endif

///The daemon is not found
void MainWindow::timeout()
{
   if ((!CallModel::instance().isConnected()) || (!CallModel::instance().isValid())) {
      KMessageBox::error(this,ErrorMessage::NO_DAEMON_ERROR);
      exit(1);
   }
}

///Select the call tab
void MainWindow::selectCallTab()
{
   QList<QTabBar*> tabBars = this->findChildren<QTabBar*>();
   if(tabBars.count()) {
      foreach(QTabBar* bar, tabBars) {
         for (int i=0;i<bar->count();i++) {
            if (bar->tabText(i).replace('&',QString()) == i18n("Call")) {
               bar->setCurrentIndex(i);
               break;
            }
         }
      }
   }
}

bool MainWindow::isAutoStart() const
{
   const bool enabled = ConfigurationSkeleton::autoStart();

   const QString localPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/autostart/";

   const QFile f(localPath);

   if (enabled && f.exists()) {
      ConfigurationSkeleton::setAutoStart(true);
   }

   return ConfigurationSkeleton::autoStart();
}

void MainWindow::setAutoStart(bool value)
{
   Q_UNUSED(value)

   if (value) {
      const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "ring-kde/ring-kde.desktop");
      QFile f(path);

      if (f.exists()) {
         if (f.copy(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/autostart/ring-kde.desktop"))
            ConfigurationSkeleton::setAutoStart(true);
      }
      else {
         qWarning() << "Cannot enable autostart, file not found";
      }

   }
   else {
      QFile f(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/autostart/ring-kde.desktop");
      f.remove();
      ConfigurationSkeleton::setAutoStart(false);
   }
}
