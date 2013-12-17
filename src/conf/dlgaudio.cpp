/****************************************************************************
 *   Copyright (C) 2009-2013 by Savoir-Faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "dlgaudio.h"

//Qt
#include <QtGui/QHeaderView>

//KDE
#include <KStandardDirs>
#include <KLineEdit>

//SFLPhone
#include "lib/dbus/configurationmanager.h"
#include "klib/kcfg_settings.h"
#include "conf/configurationdialog.h"
#include "lib/sflphone_const.h"
#include "lib/audiosettingsmodel.h"

///Constructor
DlgAudio::DlgAudio(KConfigDialog *parent)
 : QWidget(parent),m_Changed(false),m_IsLoading(false)
{
   setupUi(this);

   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   m_pAlwaysRecordCK->setChecked(configurationManager.getIsAlwaysRecording());

   KUrlRequester_destinationFolder->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
   KUrlRequester_destinationFolder->setUrl(KUrl(configurationManager.getRecordPath()));
   KUrlRequester_destinationFolder->lineEdit()->setReadOnly(true);

   m_pSuppressNoise->setChecked(AudioSettingsModel::instance()->isNoiseSuppressEnabled());
   m_pCPlayDTMFCk->setChecked(AudioSettingsModel::instance()->areDTMFMuted());

   alsaInputDevice->setModel   (AudioSettingsModel::instance()->inputDeviceModel () );
   alsaOutputDevice->setModel  (AudioSettingsModel::instance()->outputDeviceModel() );
   alsaRingtoneDevice->setModel(AudioSettingsModel::instance()->outputDeviceModel() );
   kcfg_interface->setModel    (AudioSettingsModel::instance()->audioManagerModel() );
   box_alsaPlugin->setModel    (AudioSettingsModel::instance()->alsaPluginModel  () );
   loadAlsaSettings();

   connect( box_alsaPlugin   , SIGNAL(activated(int)) , parent, SLOT(updateButtons()));
   connect( this             , SIGNAL(updateButtons()), parent, SLOT(updateButtons()));
   connect( m_pAlwaysRecordCK, SIGNAL(clicked(bool))  , this  , SLOT(changed())      );

   connect( box_alsaPlugin                  , SIGNAL(currentIndexChanged(int)) , SLOT(changed()));
   connect( m_pSuppressNoise                , SIGNAL(toggled(bool))            , SLOT(changed()));
   connect( m_pCPlayDTMFCk                  , SIGNAL(toggled(bool))            , SLOT(changed()));
   connect( alsaInputDevice                 , SIGNAL(currentIndexChanged(int)) , SLOT(changed()));
   connect( alsaOutputDevice                , SIGNAL(currentIndexChanged(int)) , SLOT(changed()));
   connect( alsaRingtoneDevice              , SIGNAL(currentIndexChanged(int)) , SLOT(changed()));
   connect( kcfg_interface                  , SIGNAL(currentIndexChanged(int)) , SLOT(changed()));
   connect( KUrlRequester_destinationFolder , SIGNAL(textChanged(QString))     , SLOT(changed()));
   connect( kcfg_interface                  , SIGNAL(currentIndexChanged(int)) , 
            AudioSettingsModel::instance()->audioManagerModel(),SLOT(setCurrentManager(int)));
   connect( kcfg_interface                  , SIGNAL(currentIndexChanged(int)) , SLOT(loadAlsaSettings()));
}

///Destructor
DlgAudio::~DlgAudio()
{
}

///Update the widgets
void DlgAudio::updateWidgets()
{
   loadAlsaSettings();
}

///Save the settings
void DlgAudio::updateSettings()
{
   if (m_Changed) {
      m_IsLoading = true;

      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      configurationManager.setRecordPath(KUrlRequester_destinationFolder->lineEdit()->text());
//       configurationManager.setAudioPlugin(box_alsaPlugin->currentText());

      configurationManager.setIsAlwaysRecording   ( m_pAlwaysRecordCK->isChecked()          );

      AudioSettingsModel::instance()->inputDeviceModel   ()->setCurrentDevice(alsaInputDevice->currentIndex   ());
      AudioSettingsModel::instance()->outputDeviceModel  ()->setCurrentDevice(alsaOutputDevice->currentIndex  ());
      AudioSettingsModel::instance()->ringtoneDeviceModel()->setCurrentDevice(alsaRingtoneDevice->currentIndex());
      AudioSettingsModel::instance()->alsaPluginModel    ()->setCurrentPlugin(box_alsaPlugin->currentIndex());
      AudioSettingsModel::instance()->setNoiseSuppressState(m_pSuppressNoise->isChecked());
      AudioSettingsModel::instance()->setDTMFMuted         (m_pCPlayDTMFCk  ->isChecked());

      m_Changed   = false;
      m_IsLoading = false;
   }
}

///Have this dialog changed
bool DlgAudio::hasChanged()
{
   return m_Changed;
}

///Tag the dialog as needing saving
void DlgAudio::changed()
{
   box_alsaPlugin->setDisabled(kcfg_interface->currentIndex());
   if (!m_IsLoading) {
      m_Changed = true;
      emit updateButtons();
   }
}

///Load alsa settings
void DlgAudio::loadAlsaSettings()
{
   m_IsLoading = true;
   AudioSettingsModel::instance()->reload();
   alsaInputDevice->setCurrentIndex    ( AudioSettingsModel::instance()->inputDeviceModel()->currentDevice().row()   );
   alsaOutputDevice->setCurrentIndex   ( AudioSettingsModel::instance()->outputDeviceModel()->currentDevice().row()  );
   alsaRingtoneDevice->setCurrentIndex ( AudioSettingsModel::instance()->ringtoneDeviceModel()->currentDevice().row());
   box_alsaPlugin->setCurrentIndex     ( AudioSettingsModel::instance()->alsaPluginModel()->currentPlugin().row()    );
   m_IsLoading = false;
}
