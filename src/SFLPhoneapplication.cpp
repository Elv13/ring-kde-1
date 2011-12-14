/***************************************************************************
 *   Copyright (C) 2009-2012 by Savoir-Faire Linux                         *
 *   Author : Emmanuel Lepage Valle <emmanuel.lepage@savoirfairelinux.com >*
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 **************************************************************************/

//Parent
#include "SFLPhoneapplication.h"

//KDE
#include <KCmdLineArgs>
#include <KIconLoader>
#include <KStandardDirs>
#include <KNotification>
#include <KSystemTrayIcon>
#include <KMainWindow>

//SFLPhone library
#include "lib/instance_interface_singleton.h"
#include "lib/configurationmanager_interface_singleton.h"
#include "lib/callmanager_interface_singleton.h"

//SFLPhone
#include "SFLPhone.h"


/**
 * The application constructor
 */
SFLPhoneApplication::SFLPhoneApplication()
  : KApplication()
  //, sflphoneWindow_(0)
{
  // SFLPhoneApplication is created from main.cpp.

  // Start remaining initialisation
  initializePaths();
  initializeMainWindow();
  connect(this,SIGNAL(aboutToQuit()),this,SLOT(quit2()));
}



/**
 * Destructor
 */
SFLPhoneApplication::~SFLPhoneApplication()
{
   // automatically destroyed
   //sflphoneWindow_ = 0;
   disableSessionManagement();
   InstanceInterface& instance              = InstanceInterfaceSingleton::getInstance();
   Q_NOREPLY instance.Unregister(getpid());
   instance.connection().disconnectFromBus(instance.connection().baseService());
}



/**
 * Return the sflphone window
 */
// SFLPhone* SFLPhoneApplication::getSFLPhoneWindow() const
// {
//   return sflphoneWindow_;
// }


/**
 * Initialisation of the main window.
 */
void SFLPhoneApplication::initializeMainWindow()
{
  // Fetch the command line arguments
  //KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

  // Enable KDE session restore.
//   int restoredWindow = -1;
//   if( kapp->isSessionRestored() ) {
//     int n = 0;
//     while( KMainWindow::canBeRestored( ++n ) ) {
//       if( KMainWindow::classNameOfToplevel( n ) != QLatin1String( "SFLPhone" ) ) {
//         continue;
//       }
// 
//       restoredWindow = n;
//       break;
//     }
//   }

  // Create the main window and initialize it
//   sflphoneWindow_ = new SFLPhone();
//   if( ! sflphoneWindow_->initialize() ) {
//     exit(1);
//     return;
//   }

  // Initialize KApplication
  //setTopWidget( sflphoneWindow_ );
//   sflphoneWindow_->show();
}



/**
 * Initialize additional paths
 */
void SFLPhoneApplication::initializePaths()
{
  // Add compile time paths as fallback
  KGlobal::dirs()       -> addPrefix( QString(DATA_INSTALL_DIR) );
  KIconLoader::global() -> addAppDir( QString(DATA_INSTALL_DIR) + "/share" );

}

Q_NOREPLY void SFLPhoneApplication::quit2()
{
   
}

#include "SFLPhoneapplication.moc"
