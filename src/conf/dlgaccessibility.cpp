/****************************************************************************
 *   Copyright (C) 2012-2013 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

#include "dlgaccessibility.h"
#include "klib/configurationskeleton.h"

//KDE
#include <KConfigDialog>
#include <KLocale>

//SFLPhone
#include <klib/macromodel.h>

///Constructor
DlgAccessibility::DlgAccessibility(KConfigDialog* parent)
 : QWidget(parent),m_Changed(false)
{
   setupUi(this);

   m_pAddTB->setIcon    ( KIcon( "list-add"    ) );
   m_pRemoveTB->setIcon ( KIcon( "list-remove" ) );
   m_pInfoIconL->setPixmap(KIcon("dialog-information").pixmap(QSize(24,24)));
   m_pInfoL->setText(i18n("This page allow to create macros that can then be called using keybooard shortcuts or added to the toolbar. To create one, \
   assign a name and a character sequence. The sequence can be numeric or any character than can be interpretted as one (ex: \"A\" would be interpretted as 2)"));

   connect(m_pNameLE        , SIGNAL(textChanged(QString)) , this,SLOT(changed())     );
   connect(m_pCategoryCBB   , SIGNAL(textChanged(QString)) , this,SLOT(changed())     );
   connect(m_pDelaySB       , SIGNAL(valueChanged(int))    , this,SLOT(changed())     );
   connect(m_pSequenceLE    , SIGNAL(textChanged(QString)) , this,SLOT(changed())     );
   connect(m_pDescriptionLE , SIGNAL(textChanged(QString)) , this,SLOT(changed())     );
   connect(m_pAddTB         , SIGNAL(clicked())            , this,SLOT(addMacro())    );
   connect(m_pRemoveTB      , SIGNAL(clicked())            , this,SLOT(removeMacro()) );
   
   connect(m_pDelaySB       , SIGNAL(valueChanged(int))    , this,SLOT(slotDelaySB(int))      );
   connect(m_pNameLE        , SIGNAL(textChanged(QString)) , this,SLOT(slotNameLE(QString))        );
   connect(m_pCategoryCBB->lineEdit()   , SIGNAL(textChanged(QString)) , this,SLOT(slotCategoryCBB(QString))   );
   connect(m_pSequenceLE    , SIGNAL(textChanged(QString)) , this,SLOT(slotSequenceLE(QString))    );
   connect(m_pDescriptionLE , SIGNAL(textChanged(QString)) , this,SLOT(slotDescriptionLE(QString)) );

   connect(this , SIGNAL(updateButtons()) , parent,SLOT(updateButtons()) );
   
   connect(MacroModel::getInstance(),SIGNAL(selectMacro(Macro*)),this,SLOT(selectMacro(Macro*)));
   connect(MacroModel::getInstance(),SIGNAL(layoutChanged()),m_pMacroListTV,SLOT(expandAll()));
   m_pMacroListTV->setModel(MacroModel::getInstance());
   m_pMacroListTV->expandAll();
   connect(m_pMacroListTV->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),MacroModel::getInstance(),SLOT(setCurrent(QModelIndex,QModelIndex)));
}

///Destructor
DlgAccessibility::~DlgAccessibility()
{
   
}

///Save
void DlgAccessibility::updateSettings()
{
   MacroModel::getInstance()->save();
}

///Load
void DlgAccessibility::updateWidgets()
{
   
}

void DlgAccessibility::changed()
{
   m_Changed = true;
   emit updateButtons();
}

bool DlgAccessibility::hasChanged()
{
   return m_Changed;
}

void DlgAccessibility::addMacro()
{
   Macro* ret = MacroModel::getInstance()->newMacro();
   if (ret) {
      m_pMacroFrm->setEnabled(true);
   }
}

void DlgAccessibility::removeMacro()
{
   
}

void DlgAccessibility::selectMacro(Macro* macro)
{
   if (macro) {
      m_pMacroListTV->expandAll();
      m_pNameLE->setText(macro->name());
      m_pCategoryCBB->lineEdit()->setText(macro->category());
      m_pDelaySB->setValue(macro->delay());
      m_pSequenceLE->setText(macro->sequence());
      m_pDescriptionLE->setText(macro->description());
      m_pMacroFrm->setEnabled(true);
      m_pNameLE->selectAll();
      m_pNameLE->setFocus();
      QModelIndex newIdx = macro->index();
      if (newIdx != m_pMacroListTV->selectionModel()->currentIndex()) {
         m_pMacroListTV->selectionModel()->setCurrentIndex(newIdx,QItemSelectionModel::ClearAndSelect);
      }
   }
}

//Widget change
void DlgAccessibility::slotNameLE(const QString& newText)
{
   Macro* current = MacroModel::getInstance()->getCurrentMacro();
   if (current) {
      current->setName(newText);
   }
}

void DlgAccessibility::slotCategoryCBB(const QString& newText)
{
   Macro* current = MacroModel::getInstance()->getCurrentMacro();
   if (current) {
      current->setCategory(newText);
   }
}

void DlgAccessibility::slotDelaySB(int newValue)
{
   Macro* current = MacroModel::getInstance()->getCurrentMacro();
   if (current) {
      current->setDelay(newValue);
   }
}

void DlgAccessibility::slotSequenceLE(const QString& newText)
{
   Macro* current = MacroModel::getInstance()->getCurrentMacro();
   if (current) {
      current->setSequence(newText);
   }
}

void DlgAccessibility::slotDescriptionLE(const QString& newText)
{
   Macro* current = MacroModel::getInstance()->getCurrentMacro();
   if (current) {
      current->setDescription(newText);
   }
}
