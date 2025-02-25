/****************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                          *
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
#include "dlggeneral.h"

//Qt
#include <QtWidgets/QToolButton>
#include <QtWidgets/QAction>

//KDE
#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <QIcon>

//Ring
#include "klib/kcfg_settings.h"
#include "conf/configurationdialog.h"
#include "categorizedhistorymodel.h"

///Constructor
DlgGeneral::DlgGeneral(KConfigDialog *parent)
 : QWidget(parent),m_HasChanged(false)
{
   setupUi(this);
   connect(toolButton_historyClear, SIGNAL(clicked()), this, SLOT(slotClearCallHistoryAsked()));
   toolButton_historyClear->setIcon(QIcon::fromTheme("edit-clear-history"));
   const bool isLimited = CategorizedHistoryModel::instance().isHistoryLimited();
   m_pKeepHistory->setChecked(!isLimited);
   m_pHistoryMax ->setEnabled(isLimited );

   m_pHistoryMax->setValue(CategorizedHistoryModel::instance().historyLimit());
   m_HasChanged = false;
   connect(this             , SIGNAL(updateButtons())                , parent, SLOT(updateButtons()));
}

///Destructor
DlgGeneral::~DlgGeneral()
{
}

///Have this dialog changed
bool DlgGeneral::hasChanged()
{
   return m_HasChanged;
}

///Tag this dialog as changed
void DlgGeneral::changed()
{
   m_HasChanged = true;
   emit updateButtons();
}

///Update all widgets
void DlgGeneral::updateWidgets()
{
}

///Save current settings
void DlgGeneral::updateSettings()
{
   CategorizedHistoryModel::instance().setHistoryLimited(m_pKeepHistory->isChecked());
   if (!m_pKeepHistory->isChecked())
      CategorizedHistoryModel::instance().setHistoryLimit(m_pHistoryMax->value());

   m_HasChanged = false;
}

void DlgGeneral::slotClearCallHistoryAsked()
{
   CategorizedHistoryModel::instance().clearAllCollections();
}
