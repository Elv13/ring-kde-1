/***************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                         *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>*
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
#ifndef VIEW_H
#define VIEW_H

#include "ui_View_base.h"
#include <QtWidgets/QWidget>

//Qt
class QString;

//Ring
#include "call.h"
#include "callmodel.h"
class Person;
class CallViewToolbar;
class CallViewOverlay;
class HistoryDelegate;
class ConferenceDelegate;
class AutoCompletion;
class CanvasObjectManager;
class EventManager;
class ContactMethod;
class ColorDelegate;

/**
 * This is the main view class for ring-kde.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 * As the state of the view has effects on the window,
 * it emits some signals to ask for changes that the window has
 * to treat.
 *
 * @short Main view
 * @author Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>
 * @author Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>
 * @version 1.4.1
 */
class View : public QWidget, public Ui::View
{
   Q_OBJECT
   friend class EventManager;

private:
   CallViewToolbar*     m_pCanvasToolbar  ;
   CallViewOverlay*     m_pTransferOverlay;
   ConferenceDelegate*  m_pConfDelegate   ;
   HistoryDelegate*     m_pHistoryDelegate;
   AutoCompletion*      m_pAutoCompletion ;
   CanvasObjectManager* m_pCanvasManager  ;
   EventManager*        m_pEventManager   ;

public:
   //Constructors & Destructors
   explicit View(QWidget *parent);
   virtual ~View();

   AutoCompletion* autoCompletion    () const;
   bool            messageBoxFocussed() const;

private Q_SLOTS:
   void sendMessage          ();
   void slotAutoCompleteClicked(ContactMethod* n);
   void loadAutoCompletion   ();

public Q_SLOTS:
   void updateVolumeControls ();

   void displayVolumeControls ( bool checked = true );
   void displayDialpad        ( bool checked = true );
   void displayMessageBox     ( bool checked = true );

   void incomingCall          ( Call* call          );

   void paste();
};

#endif // VIEW_H
