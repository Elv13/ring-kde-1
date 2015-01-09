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
#include "accessibility.h"

//KDE
#include <KSpeech>
#include <KDebug>
#include <KLocale>
#include <KIcon>

//Ring
#include <call.h>
#include <callmodel.h>
#include <phonenumber.h>
#include "kspeechinterfacesingleton.h"
#include "ring.h"
#include "view.h"

Accessibility* Accessibility::m_pInstance = nullptr;

///Constructor
Accessibility::Accessibility() : QObject(0),QList<KAction*>()
{
   KAction* action = new KAction(0);
   action->setObjectName ( "listCall"               );
   action->setShortcut   ( Qt::CTRL + Qt::Key_L     );
   action->setText       ( i18n("List all current calls") );
   action->setIcon       ( KIcon("text-speak")      );
   *this << action;
   connect(action,SIGNAL(triggered(bool)),this,SLOT(listCall()));

   action = new KAction(0);
   action->setObjectName ( "currentCallDetails"       );
   action->setShortcut   ( Qt::CTRL + Qt::Key_I       );
   action->setText       ( i18n("Get current call details") );
   action->setIcon       ( KIcon("text-speak")        );
   *this << action;
   connect(action,SIGNAL(triggered(bool)),this,SLOT(currentCallDetails()));
}

Accessibility::~Accessibility()
{
   for (int i=0;i<QList<KAction*>::size();i++){
      delete QList<KAction*>::at(i);
   }
}

///Signleton
Accessibility* Accessibility::instance()
{
   if (! m_pInstance) {
      m_pInstance = new Accessibility();
   }
   return m_pInstance;
}

///Use the speech daemon to read details about the current calls
void Accessibility::listCall()
{
   if (CallModel::instance()->getActiveCalls().size()>0) {
      KSpeechInterfaceSingleton::instance()->say(i18np("You currently have <numid>%1</numid> call","You currently have <numid>%1</numid> calls",CallModel::instance()->getActiveCalls().size()), KSpeech::soPlainText);
      foreach (Call* call,CallModel::instance()->getActiveCalls()) {
         KSpeechInterfaceSingleton::instance()->say(i18n("Call from %1, number %2",call->peerName(),numberToDigit((!call->peerPhoneNumber()->uri().isEmpty())?call->peerPhoneNumber()->uri():call->dialNumber())), KSpeech::soPlainText);
      }
   }
   else {
      KSpeechInterfaceSingleton::instance()->say(i18n("You currently have no call"), KSpeech::soPlainText);
   }
}

///Convert number to digit so the speech daemon say "one two three" instead of "one hundred and twenty three"
QString Accessibility::numberToDigit(const QString &number)
{
   QString toReturn;
   for(int i=0;i<number.count();i++) {
      if (i+1 < number.count() && (number[i] >= 0x30 && number[i] <= 0x39) && (number[i+1] >= 0x30 && number[i+1] <= 0x39))
         toReturn += QString(number[i])+' ';
      else
         toReturn += number[i];
   }
   return toReturn;
}

///Use the speech daemon to read the current call details
void Accessibility::currentCallDetails()
{
   foreach (Call* call,CallModel::instance()->getActiveCalls()) {
      if (Ring::view()->currentCall() == call) {
         QString toSay = i18n("The current call is %1",i18n(call->toHumanStateName(call->state()).toAscii() ));
         if (!call->peerName().trimmed().isEmpty())
            toSay += i18n(",Your peer is %1",numberToDigit(call->peerName()));
         if (!call->peerPhoneNumber()->uri().isEmpty())
            toSay += i18n(", the peer phone number is %1 ",numberToDigit(call->peerPhoneNumber()->uri())    );
         else if (!call->dialNumber().isEmpty())
            toSay += i18n(", the phone number is %1 ",numberToDigit(call->dialNumber()));

         const int nSec = QDateTime::fromTime_t(call->startTimeStamp()).time().secsTo( QTime::currentTime() );
         if (nSec>0)
            toSay += i18n(" and you have been talking since %1 seconds",nSec );

         KSpeechInterfaceSingleton::instance()->say(toSay, KSpeech::soPlainText);
      }
   }
}

///Helper function is make code shorter
void Accessibility::say(const QString &message)
{
   KSpeechInterfaceSingleton::instance()->say(message, KSpeech::soPlainText);
}
