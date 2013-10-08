/***************************************************************************
 *   Copyright (C) 2013 by Savoir-Faire Linux                              *
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
#include "eventmanager.h"

//Qt
#include <QtGui/QKeyEvent>
#include <QtGui/QDropEvent>

//KDE
#include <KDebug>

//SFLPhone
#include "sflphoneview.h"
#include "sflphone.h"
#include "canvasobjectmanager.h"
#include "widgets/kphonenumberselector.h"
#include "widgets/tips/tipcollection.h"
#include "widgets/callviewoverlay.h"
#include "widgets/autocompletion.h"
#include "klib/tipmanager.h"
#include <lib/phonenumber.h>
#include <lib/account.h>
#include <lib/phonedirectorymodel.h>
#include "klib/akonadibackend.h"

///Constructor
EventManager::EventManager(SFLPhoneView* parent): QObject(parent),m_pParent(parent)
{
   connect(CallModel::instance() , SIGNAL(callStateChanged(Call*,Call::State)) , this, SLOT(slotCallStateChanged(Call*,Call::State)) );
   connect(CallModel::instance() , SIGNAL(incomingCall(Call*))                 , this, SLOT(slotIncomingCall(Call*)) );
}

///Destructor
EventManager::~EventManager()
{
   
}

/*****************************************************************************
 *                                                                           *
 *                              Events filter                                *
 *                                                                           *
 ****************************************************************************/


bool EventManager::viewDragEnterEvent(const QDragEnterEvent* e)
{
   Q_UNUSED(e)
   return false;
}

bool EventManager::viewDropEvent(QDropEvent* e)
{
   const QModelIndex& idxAt = m_pParent->m_pView->indexAt(e->pos());
   CallModel::instance()->setData(idxAt,-1,Call::Role::DropState);
   e->accept();
   if (!idxAt.isValid()) { //Dropped on empty space
      if (e->mimeData()->hasFormat(MIME_CALLID)) {
         const QByteArray encodedCallId      = e->mimeData()->data( MIME_CALLID      );
         kDebug() << "Call dropped on empty space";
         Call* call =  CallModel::instance()->getCall(encodedCallId);
         if (CallModel::instance()->getIndex(call).parent().isValid()) {
            kDebug() << "Detaching participant";
            CallModel::instance()->detachParticipant(CallModel::instance()->getCall(encodedCallId));
         }
         else
            kDebug() << "The call is not in a conversation (doing nothing)";
      }
      else if (e->mimeData()->hasFormat(MIME_PHONENUMBER)) {
         const QByteArray encodedPhoneNumber = e->mimeData()->data( MIME_PHONENUMBER );
         kDebug() << "Phone number dropped on empty space";
         Call* newCall = CallModel::instance()->addDialingCall();
         newCall->setDialNumber(encodedPhoneNumber);
         newCall->actionPerformed(Call::Action::ACCEPT);
      }
      else if (e->mimeData()->hasFormat(MIME_CONTACT)) {
         const QByteArray encodedContact     = e->mimeData()->data( MIME_CONTACT     );
         kDebug() << "Contact dropped on empty space";
         const PhoneNumber* number = KPhoneNumberSelector().getNumber(encodedContact);
         if (number->uri().isEmpty()) {
            Call* newCall = CallModel::instance()->addDialingCall();
            newCall->setDialNumber(number->uri());
            newCall->actionPerformed(Call::Action::ACCEPT);
         }
      }
      else if (e->mimeData()->hasFormat("text/plain")) {
         Call* newCall = CallModel::instance()->addDialingCall();
         newCall->setDialNumber(e->mimeData()->data( "text/plain" ));
         newCall->actionPerformed(Call::Action::ACCEPT);
      }
      //Remove uneedded tip
      if (TipCollection::removeConference() == TipCollection::manager()->currentTip()) {
         m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::DROP);
      }
      return true;
   }
   else {
      //1) Get the right action
      const QPoint position = e->pos();
      const QRect targetRect = m_pParent->m_pView->visualRect(idxAt);
      Call::DropAction act = (position.x() < targetRect.x()+targetRect.width()/2)?Call::DropAction::Conference:Call::DropAction::Transfer;
      QMimeData* data = (QMimeData*)e->mimeData(); //Drop the const
      data->setProperty("dropAction",act);

      //2) Send to the model for processing
      m_pParent->m_pView->model()->dropMimeData(data,Qt::MoveAction,idxAt.row(),idxAt.column(),idxAt.parent());
   }

   //Remove item overlays
   for (int i = 0;i < m_pParent->m_pView->model()->rowCount();i++) {
      const QModelIndex& idx = m_pParent->m_pView->model()->index(i,0);
      m_pParent->m_pView->model()->setData(idx,-1,Call::Role::DropState);
      for (int j = 0;j < m_pParent->m_pView->model()->rowCount(idx);j++) {
         m_pParent->m_pView->model()->setData(m_pParent->m_pView->model()->index(j,0,idx),-1,Call::Role::DropState);
      }
   }
   return false;
}

bool EventManager::viewDragMoveEvent(const QDragMoveEvent* e)
{
   if (TipCollection::removeConference() != TipCollection::manager()->currentTip() /*&& idxAt.parent().isValid()*/) {
      if (e->mimeData()->hasFormat(MIME_CALLID)) {
         TipCollection::removeConference()->setText(i18n("Remove the call from the conference, the call will be put on hold"));
      }
      else if (e->mimeData()->hasFormat(MIME_PHONENUMBER)) {
         TipCollection::removeConference()->setText(i18n("Call %1",QString(e->mimeData()->data(MIME_PHONENUMBER))));
      }
      else if (e->mimeData()->hasFormat(MIME_CONTACT)) {
         Contact* c = AkonadiBackend::instance()->getContactByUid(e->mimeData()->data(MIME_CONTACT));
         if (c) {
            TipCollection::removeConference()->setText(i18n("Call %1",c->formattedName()));
         }
      }
      else if (e->mimeData()->hasFormat("text/plain")) {
         TipCollection::removeConference()->setText(i18n("Call %1",QString(e->mimeData()->data("text/plain"))));
      }
//          TipCollection::manager()->setCurrentTip(TipCollection::removeConference());
      m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::DRAG_MOVE);
   }
   if (TipCollection::removeConference() == TipCollection::manager()->currentTip()) {
      if (e->mimeData()->hasFormat(MIME_CALLID)) {
         TipCollection::removeConference()->setText(i18n("Remove the call from the conference, the call will be put on hold"));
      }
      else if (e->mimeData()->hasFormat(MIME_PHONENUMBER)) {
         TipCollection::removeConference()->setText(i18n("Call %1",QString(e->mimeData()->data(MIME_PHONENUMBER))));
      }
      else if (e->mimeData()->hasFormat(MIME_CONTACT)) {
         Contact* c = AkonadiBackend::instance()->getContactByUid(e->mimeData()->data(MIME_CONTACT));
         if (c) {
            TipCollection::removeConference()->setText(i18n("Call %1",c->formattedName()));
         }
      }
      else if (e->mimeData()->hasFormat("text/plain")) {
         TipCollection::removeConference()->setText(i18n("Call %1",QString(e->mimeData()->data("text/plain"))));
      }
   }
   //Just as drop, compute the position
   const QModelIndex& idxAt = m_pParent->m_pView->indexAt(e->pos());
   const QPoint position = e->pos();
   const QRect targetRect = m_pParent->m_pView->visualRect(idxAt);
   Call::DropAction act = (position.x() < targetRect.x()+targetRect.width()/2)?Call::DropAction::Conference:Call::DropAction::Transfer;
   CallModel::instance()->setData(idxAt,act,Call::Role::DropPosition);
   return true;
}

bool EventManager::viewDragLeaveEvent(const QDragMoveEvent* e)
{
   Q_UNUSED(e)
   return false;
}



bool EventManager::eventFilter(QObject *obj, QEvent *event)
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wswitch"
   switch (event->type()) {
      case QEvent::KeyPress: {
         const int key = static_cast<QKeyEvent*>(event)->key();
         if (key != Qt::Key_Left && key != Qt::Key_Right && key != Qt::Key_Down && key != Qt::Key_Up) {
            if (viewKeyEvent(static_cast<QKeyEvent*>(event))) return true;
         }
         } break;
      case QEvent::Drop:
         if (viewDropEvent(static_cast<QDropEvent*>(event))) return true;
         break;
      case QEvent::DragMove:
         if (viewDragMoveEvent(static_cast<QDragMoveEvent*>(event))) return true;
      case QEvent::DragLeave:
         m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::DRAG_LEAVE);
   };
   #pragma GCC diagnostic pop
   return QObject::eventFilter(obj, event);
} //eventFilter

bool EventManager::viewKeyEvent(const QKeyEvent* event)
{
      switch(event->key()) {
      case Qt::Key_Escape:
         escape();
         break;
      case Qt::Key_Return:
      case Qt::Key_Enter:
         if (m_pParent->m_pAutoCompletion && m_pParent->m_pAutoCompletion->selection()) {
            PhoneNumber* n = m_pParent->m_pAutoCompletion->selection();
            Call* call = m_pParent->currentCall();
            if (call->state() == Call::State::DIALING) {
               call->setDialNumber(n->uri());
               if (PhoneDirectoryModel::instance()->callWithAccount() && n->account() && n->account()->id() != "IP2IP")
                  call->setAccount(n->account());
            }
         }
         enter();
         break;
      case Qt::Key_Backspace:
         backspace();
         break;
      case Qt::Key_Up:
         if (m_pParent->m_pAutoCompletion && m_pParent->m_pAutoCompletion->isVisible()) {
            m_pParent->m_pAutoCompletion->moveUp();
         }
         break;
      case Qt::Key_Down:
         if (m_pParent->m_pAutoCompletion && m_pParent->m_pAutoCompletion->isVisible()) {
            m_pParent->m_pAutoCompletion->moveDown();
         }
         break;
      default: {
         const QString& text = event->text();
         if(! text.isEmpty()) {
            typeString(text);
         }
      }
      break;
   };
   return true;
}

///Called on keyboard
void EventManager::typeString(const QString& str)
{
   /* There is 5 cases
    * 1) There is no call, then create one
    * 2) There is one or more call and the active call is also selected, then send DTMF to the active call
    * 3) There is multiple call, but the active one is not selected, then create a new call or add to existing dialing call
    * 4) There is only inactive calls, then create a new one or add to existing dialing call
    * 5) There is only FAILURE, BUSY or UNKNOWN calls, then create a new one or add to existing dialing call
    * 
    * When adding to dialing call, select it to give user feedback of where the tone went.
    * 
    * Any other comportment need to be documented here or treated as a bug
    */

   Call* call = CallModel::instance()->getCall(m_pParent->m_pView->selectionModel()->currentIndex());
   Call* currentCall = nullptr;
   Call* candidate   = nullptr;


   //If the selected call is also the current one, then send DTMF and exit
   if(call && call->state() == Call::State::CURRENT) {
      currentCall = call;
      call->playDTMF(str);
      return;
   }

   foreach (Call* call2, CallModel::instance()->getCallList()) {
      if(dynamic_cast<Call*>(call2) && currentCall != call2 && call2->state() == Call::State::CURRENT) {
         m_pParent->action(call2, Call::Action::HOLD);
      }
      else if(dynamic_cast<Call*>(call2) && call2->state() == Call::State::DIALING) {
         candidate = call2;
      }
   }

   if(!currentCall && !candidate) {
      kDebug() << "Typing when no item is selected. Opening an item.";
      candidate = CallModel::instance()->addDialingCall();
      const QModelIndex& newCallIdx = CallModel::instance()->getIndex(candidate);
      if (newCallIdx.isValid()) {
         m_pParent->m_pView->selectionModel()->setCurrentIndex(newCallIdx,QItemSelectionModel::SelectCurrent);
      }
   }

   if (!candidate) {
      candidate = CallModel::instance()->addDialingCall();
   }
   if(!currentCall && candidate) {
      candidate->playDTMF(str);
      candidate->appendText(str);
   }
} //typeString

///Called when a backspace is detected
void EventManager::backspace()
{
   kDebug() << "backspace";
   Call* call = CallModel::instance()->getCall(m_pParent->m_pView->selectionModel()->currentIndex());
   if(!call) {
      kDebug() << "Error : Backspace on unexisting call.";
   }
   else {
      call->backspaceItemText();
   }
}

///Called when escape is detected
void EventManager::escape()
{
   kDebug() << "escape";
   Call* call = CallModel::instance()->getCall(m_pParent->m_pView->selectionModel()->currentIndex());
   if (m_pParent->m_pTransferOverlay && m_pParent->m_pTransferOverlay->isVisible()) {
      m_pParent->m_pTransferOverlay->setVisible(false);
      m_pParent->updateWindowCallState();
      return;
   }

   if(!call) {
      kDebug() << "Escape when no item is selected. Doing nothing.";
   }
   else {
      switch (call->state()) {
         case Call::State::TRANSFERRED:
         case Call::State::TRANSF_HOLD:
            m_pParent->action(call, Call::Action::TRANSFER);
            break;
         case Call::State::INCOMING:
         case Call::State::DIALING:
         case Call::State::HOLD:
         case Call::State::RINGING:
         case Call::State::CURRENT:
         case Call::State::FAILURE:
         case Call::State::BUSY:
         case Call::State::OVER:
         case Call::State::ERROR:
         case Call::State::CONFERENCE:
         case Call::State::CONFERENCE_HOLD:
         case Call::State::COUNT:
         default:
            m_pParent->action(call, Call::Action::REFUSE);
      }
   }
} //escape

///Called when enter is detected
void EventManager::enter()
{
   Call* call = CallModel::instance()->getCall(m_pParent->m_pView->selectionModel()->currentIndex()); //TODO use selectionmodel
   if(!call) {
      kDebug() << "Error : Enter on unexisting call.";
   }
   else {
      switch (call->state()) {
         case Call::State::INCOMING:
         case Call::State::DIALING:
         case Call::State::TRANSFERRED:
         case Call::State::TRANSF_HOLD:
            m_pParent->action(call, Call::Action::ACCEPT);
            break;
         case Call::State::HOLD:
            m_pParent->action(call, Call::Action::HOLD);
            break;
         case Call::State::RINGING:
         case Call::State::CURRENT:
         case Call::State::FAILURE:
         case Call::State::BUSY:
         case Call::State::OVER:
         case Call::State::ERROR:
         case Call::State::CONFERENCE:
         case Call::State::CONFERENCE_HOLD:
         case Call::State::COUNT:
         default:
            kDebug() << "Enter when call selected not in appropriate state. Doing nothing.";
      }
   }
}

/*****************************************************************************
 *                                                                           *
 *                                  Slots                                    *
 *                                                                           *
 ****************************************************************************/
void EventManager::slotCallStateChanged(Call* call, Call::State previousState)
{
   Q_UNUSED(call)
   Q_UNUSED(previousState)
   switch (call->state()) {
      case Call::State::RINGING:
         m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::CALL_RINGING);
         break;
      case Call::State::DIALING:
      case Call::State::INCOMING:
         break; //Handled elsewhere
      case Call::State::OVER:
         if (previousState == Call::State::DIALING || previousState == Call::State::OVER)
            m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::CALL_ENDED,i18n("Cancelled"));
         else
            m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::CALL_ENDED,i18n("Call ended"));
         break;
      case Call::State::FAILURE:
      case Call::State::BUSY:
         m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::CALL_BUSY);
         break;
      case Call::State::TRANSFERRED:
      case Call::State::TRANSF_HOLD:
      case Call::State::HOLD:
      case Call::State::CURRENT:
      case Call::State::ERROR:
      case Call::State::CONFERENCE:
      case Call::State::CONFERENCE_HOLD:
      case Call::State::COUNT:
      default:
         m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::CALL_STATE_CHANGED);
         kDebug() << "Enter when call selected not in appropriate state. Doing nothing.";
   }
}

void EventManager::slotIncomingCall(Call* call)
{
   Q_UNUSED(call)
   if (call->state() == Call::State::INCOMING || call->state() == Call::State::RINGING) {
      m_pParent->m_pCanvasManager->newEvent(CanvasObjectManager::CanvasEvent::CALL_RINGING);
   }
}
