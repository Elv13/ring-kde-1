/****************************************************************************
 *   Copyright (C) 2009 by Savoir-Faire Linux                               *
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

//Parent
#include "call.h"

//Qt
#include <QFile>

//SFLPhone library
#include "callmanager_interface_singleton.h"
#include "configurationmanager_interface_singleton.h"
#include "contactbackend.h"
#include "contact.h"
#include "account.h"
#include "accountlist.h"
#include "videomodel.h"
#include "instantmessagingmodel.h"


const call_state Call::actionPerformedStateMap [13][5] =
{
//                      ACCEPT                  REFUSE                  TRANSFER                   HOLD                           RECORD            /**/
/*INCOMING     */  {CALL_STATE_INCOMING   , CALL_STATE_INCOMING    , CALL_STATE_ERROR        , CALL_STATE_INCOMING     ,  CALL_STATE_INCOMING     },/**/
/*RINGING      */  {CALL_STATE_ERROR      , CALL_STATE_RINGING     , CALL_STATE_ERROR        , CALL_STATE_ERROR        ,  CALL_STATE_RINGING      },/**/
/*CURRENT      */  {CALL_STATE_ERROR      , CALL_STATE_CURRENT     , CALL_STATE_TRANSFERRED  , CALL_STATE_CURRENT      ,  CALL_STATE_CURRENT      },/**/
/*DIALING      */  {CALL_STATE_DIALING    , CALL_STATE_OVER        , CALL_STATE_ERROR        , CALL_STATE_ERROR        ,  CALL_STATE_ERROR        },/**/
/*HOLD         */  {CALL_STATE_ERROR      , CALL_STATE_HOLD        , CALL_STATE_TRANSF_HOLD  , CALL_STATE_HOLD         ,  CALL_STATE_HOLD         },/**/
/*FAILURE      */  {CALL_STATE_ERROR      , CALL_STATE_FAILURE     , CALL_STATE_ERROR        , CALL_STATE_ERROR        ,  CALL_STATE_ERROR        },/**/
/*BUSY         */  {CALL_STATE_ERROR      , CALL_STATE_BUSY        , CALL_STATE_ERROR        , CALL_STATE_ERROR        ,  CALL_STATE_ERROR        },/**/
/*TRANSFER     */  {CALL_STATE_TRANSFERRED, CALL_STATE_TRANSFERRED , CALL_STATE_CURRENT      , CALL_STATE_TRANSFERRED  ,  CALL_STATE_TRANSFERRED  },/**/
/*TRANSF_HOLD  */  {CALL_STATE_TRANSF_HOLD, CALL_STATE_TRANSF_HOLD , CALL_STATE_HOLD         , CALL_STATE_TRANSF_HOLD  ,  CALL_STATE_TRANSF_HOLD  },/**/
/*OVER         */  {CALL_STATE_ERROR      , CALL_STATE_ERROR       , CALL_STATE_ERROR        , CALL_STATE_ERROR        ,  CALL_STATE_ERROR        },/**/
/*ERROR        */  {CALL_STATE_ERROR      , CALL_STATE_ERROR       , CALL_STATE_ERROR        , CALL_STATE_ERROR        ,  CALL_STATE_ERROR        },/**/
/*CONF         */  {CALL_STATE_ERROR      , CALL_STATE_CURRENT     , CALL_STATE_TRANSFERRED  , CALL_STATE_CURRENT      ,  CALL_STATE_CURRENT      },/**/
/*CONF_HOLD    */  {CALL_STATE_ERROR      , CALL_STATE_HOLD        , CALL_STATE_TRANSF_HOLD  , CALL_STATE_HOLD         ,  CALL_STATE_HOLD         },/**/
};//                                                                                                                                                    


const function Call::actionPerformedFunctionMap[13][5] =
{ 
//                      ACCEPT               REFUSE            TRANSFER                 HOLD                  RECORD             /**/
/*INCOMING       */  {&Call::accept     , &Call::refuse   , &Call::acceptTransf   , &Call::acceptHold  ,  &Call::setRecord     },/**/
/*RINGING        */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::nothing     ,  &Call::setRecord     },/**/
/*CURRENT        */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::hold        ,  &Call::setRecord     },/**/
/*DIALING        */  {&Call::call       , &Call::cancel   , &Call::nothing        , &Call::nothing     ,  &Call::nothing       },/**/
/*HOLD           */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::unhold      ,  &Call::setRecord     },/**/
/*FAILURE        */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::nothing     ,  &Call::nothing       },/**/
/*BUSY           */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::nothing     ,  &Call::nothing       },/**/
/*TRANSFERT      */  {&Call::transfer   , &Call::hangUp   , &Call::transfer       , &Call::hold        ,  &Call::setRecord     },/**/
/*TRANSFERT_HOLD */  {&Call::transfer   , &Call::hangUp   , &Call::transfer       , &Call::unhold      ,  &Call::setRecord     },/**/
/*OVER           */  {&Call::nothing    , &Call::nothing  , &Call::nothing        , &Call::nothing     ,  &Call::nothing       },/**/
/*ERROR          */  {&Call::nothing    , &Call::nothing  , &Call::nothing        , &Call::nothing     ,  &Call::nothing       },/**/
/*CONF           */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::hold        ,  &Call::setRecord     },/**/
/*CONF_HOLD      */  {&Call::nothing    , &Call::hangUp   , &Call::nothing        , &Call::unhold      ,  &Call::setRecord     },/**/
};//                                                                                                                                 


const call_state Call::stateChangedStateMap [13][6] =
{
//                      RINGING                  CURRENT                BUSY                HOLD                        HUNGUP           FAILURE             /**/
/*INCOMING     */ {CALL_STATE_INCOMING    , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*RINGING      */ {CALL_STATE_RINGING     , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*CURRENT      */ {CALL_STATE_CURRENT     , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*DIALING      */ {CALL_STATE_RINGING     , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*HOLD         */ {CALL_STATE_HOLD        , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*FAILURE      */ {CALL_STATE_FAILURE     , CALL_STATE_FAILURE    , CALL_STATE_BUSY   , CALL_STATE_FAILURE      ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*BUSY         */ {CALL_STATE_BUSY        , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_BUSY         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*TRANSFER     */ {CALL_STATE_TRANSFERRED , CALL_STATE_TRANSFERRED, CALL_STATE_BUSY   , CALL_STATE_TRANSF_HOLD  ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*TRANSF_HOLD  */ {CALL_STATE_TRANSF_HOLD , CALL_STATE_TRANSFERRED, CALL_STATE_BUSY   , CALL_STATE_TRANSF_HOLD  ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*OVER         */ {CALL_STATE_OVER        , CALL_STATE_OVER       , CALL_STATE_OVER   , CALL_STATE_OVER         ,  CALL_STATE_OVER  ,  CALL_STATE_OVER     },/**/
/*ERROR        */ {CALL_STATE_ERROR       , CALL_STATE_ERROR      , CALL_STATE_ERROR  , CALL_STATE_ERROR        ,  CALL_STATE_ERROR ,  CALL_STATE_ERROR    },/**/
/*CONF         */ {CALL_STATE_CURRENT     , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
/*CONF_HOLD    */ {CALL_STATE_HOLD        , CALL_STATE_CURRENT    , CALL_STATE_BUSY   , CALL_STATE_HOLD         ,  CALL_STATE_OVER  ,  CALL_STATE_FAILURE  },/**/
};//                                                                                                                                                             

const function Call::stateChangedFunctionMap[13][6] =
{ 
//                      RINGING                  CURRENT             BUSY              HOLD                    HUNGUP           FAILURE            /**/
/*INCOMING       */  {&Call::nothing    , &Call::start     , &Call::startWeird     , &Call::startWeird   ,  &Call::startStop    , &Call::start   },/**/
/*RINGING        */  {&Call::nothing    , &Call::start     , &Call::start          , &Call::start        ,  &Call::startStop    , &Call::start   },/**/
/*CURRENT        */  {&Call::nothing    , &Call::nothing   , &Call::warning        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
/*DIALING        */  {&Call::nothing    , &Call::warning   , &Call::warning        , &Call::warning      ,  &Call::stop         , &Call::warning },/**/
/*HOLD           */  {&Call::nothing    , &Call::nothing   , &Call::warning        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
/*FAILURE        */  {&Call::nothing    , &Call::warning   , &Call::warning        , &Call::warning      ,  &Call::stop         , &Call::nothing },/**/
/*BUSY           */  {&Call::nothing    , &Call::nothing   , &Call::nothing        , &Call::warning      ,  &Call::stop         , &Call::nothing },/**/
/*TRANSFERT      */  {&Call::nothing    , &Call::nothing   , &Call::warning        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
/*TRANSFERT_HOLD */  {&Call::nothing    , &Call::nothing   , &Call::warning        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
/*OVER           */  {&Call::nothing    , &Call::warning   , &Call::warning        , &Call::warning      ,  &Call::stop         , &Call::warning },/**/
/*ERROR          */  {&Call::nothing    , &Call::nothing   , &Call::nothing        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
/*CONF           */  {&Call::nothing    , &Call::nothing   , &Call::warning        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
/*CONF_HOLD      */  {&Call::nothing    , &Call::nothing   , &Call::warning        , &Call::nothing      ,  &Call::stop         , &Call::nothing },/**/
};//                                                                                                                                                   

const char * Call::historyIcons[3] = {ICON_HISTORY_INCOMING, ICON_HISTORY_OUTGOING, ICON_HISTORY_MISSED};

ContactBackend* Call::m_pContactBackend = nullptr;
Call*           Call::m_sSelectedCall   = nullptr;

void Call::setContactBackend(ContactBackend* be)
{
   m_pContactBackend = be;
}

///Constructor
Call::Call(call_state startState, QString callId, QString peerName, QString peerNumber, QString account)
   : m_isConference(false),m_pStopTime(nullptr),m_pStartTime(nullptr),
   m_ContactChanged(false),m_pContact(nullptr),m_pImModel(nullptr)
{
   this->m_CallId          = callId     ;
   this->m_PeerPhoneNumber = peerNumber ;
   this->m_PeerName        = peerName   ;
   this->m_Account         = account    ;
   this->m_Recording       = false      ;
   this->m_pStartTime      = nullptr    ;
   this->m_pStopTime       = nullptr    ;
   changeCurrentState(startState)       ;

   m_ContactChanged = true;
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   connect(&callManager,SIGNAL(recordPlaybackStopped(QString)), this, SLOT(stopPlayback(QString))  );
   connect(&callManager,SIGNAL(updatePlaybackScale(int,int))  , this, SLOT(updatePlayback(int,int)));
   if (m_pContactBackend)
      connect(m_pContactBackend,SIGNAL(collectionChanged()),this,SLOT(contactBackendChanged()));

   emit changed();
}

///Destructor
Call::~Call()
{
   //if (m_pStartTime) delete m_pStartTime ;
   //if (m_pStopTime)  delete m_pStopTime  ;
   this->disconnect();
}

///Constructor
Call::Call(QString confId, QString account): m_isConference(false),m_pStopTime(nullptr),m_pStartTime(nullptr),
   m_ContactChanged(false),m_pContact(nullptr),m_pImModel(nullptr)
{
   m_isConference  = m_ConfId.isEmpty();
   this->m_ConfId  = confId  ;
   this->m_Account = account ;

   if (m_isConference) {
      CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
      MapStringString        details    = callManager.getConferenceDetails(m_ConfId)  ;
      m_CurrentState = confStatetoCallState(details["CONF_STATE"]);
   }
}

/*****************************************************************************
 *                                                                           *
 *                               Call builder                                *
 *                                                                           *
 ****************************************************************************/

///Build a call from its ID
Call* Call::buildExistingCall(QString callId)
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   MapStringString       details     = callManager.getCallDetails(callId).value();

   qDebug() << "Constructing existing call with details : " << details;

   QString    peerNumber = details[ CALL_PEER_NUMBER ];
   QString    peerName   = details[ CALL_PEER_NAME   ];
   QString    account    = details[ CALL_ACCOUNTID   ];
   call_state startState = getStartStateFromDaemonCallState(details[CALL_STATE], details[CALL_TYPE]);

   Call* call            = new Call(startState, callId, peerName, peerNumber, account)                   ;

   if (!details[ CALL_TIMESTAMP_START ].isEmpty())
      call->m_pStartTime = new QDateTime(QDateTime::fromTime_t(details[ CALL_TIMESTAMP_START ].toInt())) ;
   else
      call->m_pStartTime = new QDateTime(QDateTime::currentDateTime())                                   ;

   call->m_Recording     = callManager.getIsRecording(callId)                                            ;
   call->m_HistoryState  = getHistoryStateFromType(details[STATE_KEY]);

   return call;
} //buildExistingCall

///Build a call from a dialing call (a call that is about to exist)
Call* Call::buildDialingCall(QString callId, const QString & peerName, QString account)
{
   Call* call = new Call(CALL_STATE_DIALING, callId, peerName, "", account);
   call->m_HistoryState = NONE;
   return call;
}

///Build a call from a dbus event
Call* Call::buildIncomingCall(const QString & callId)
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   MapStringString details = callManager.getCallDetails(callId).value();

   QString from     = details[ CALL_PEER_NUMBER ];
   QString account  = details[ CALL_ACCOUNTID   ];
   QString peerName = details[ CALL_PEER_NAME   ];
   
   Call* call = new Call(CALL_STATE_INCOMING, callId, peerName, from, account);
   call->m_HistoryState = MISSED;
   return call;
} //buildIncomingCall

///Build a rigging call (from dbus)
Call* Call::buildRingingCall(const QString & callId)
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   MapStringString details = callManager.getCallDetails(callId).value();
   
   QString from     = details[ CALL_PEER_NUMBER ];
   QString account  = details[ CALL_ACCOUNTID   ];
   QString peerName = details[ CALL_PEER_NAME   ];
   
   Call* call = new Call(CALL_STATE_RINGING, callId, peerName, from, account);
   call->m_HistoryState = OUTGOING;
   return call;
} //buildRingingCall

/*****************************************************************************
 *                                                                           *
 *                                  History                                  *
 *                                                                           *
 ****************************************************************************/

///Build a call that is already over
Call* Call::buildHistoryCall(const QString & callId, uint startTimeStamp, uint stopTimeStamp, QString account, QString name, QString number, QString type)
{
   if(name == "empty") name = "";
   Call* call            = new Call(CALL_STATE_OVER, callId, name, number, account );

   QDateTime* start = new QDateTime(QDateTime::fromTime_t(startTimeStamp));
   QDateTime* stop  = new QDateTime(QDateTime::fromTime_t(stopTimeStamp));

   if (start){
      call->m_pStartTime = start;
      call->m_pStopTime  = stop;
   }
   
   call->m_HistoryState  = getHistoryStateFromType(type);
   
   return call;
}

///Get the history state from the type (see Call.cpp header)
history_state Call::getHistoryStateFromType(QString type)
{
   if(type == MISSED_STRING        )
      return MISSED   ;
   else if(type == OUTGOING_STRING )
      return OUTGOING ;
   else if(type == INCOMING_STRING )
      return INCOMING ;
   return NONE        ;
}

///Get the start sate from the daemon state
call_state Call::getStartStateFromDaemonCallState(QString daemonCallState, QString daemonCallType)
{
   if(daemonCallState      == DAEMON_CALL_STATE_INIT_CURRENT  )
      return CALL_STATE_CURRENT  ;
   else if(daemonCallState == DAEMON_CALL_STATE_INIT_HOLD     )
      return CALL_STATE_HOLD     ;
   else if(daemonCallState == DAEMON_CALL_STATE_INIT_BUSY     )
      return CALL_STATE_BUSY     ;
   else if(daemonCallState == DAEMON_CALL_STATE_INIT_INACTIVE && daemonCallType == DAEMON_CALL_TYPE_INCOMING )
      return CALL_STATE_INCOMING ;
   else if(daemonCallState == DAEMON_CALL_STATE_INIT_INACTIVE && daemonCallType == DAEMON_CALL_TYPE_OUTGOING )
      return CALL_STATE_RINGING  ;
   else if(daemonCallState == DAEMON_CALL_STATE_INIT_INCOMING )
      return CALL_STATE_INCOMING ;
   else if(daemonCallState == DAEMON_CALL_STATE_INIT_RINGING  )
      return CALL_STATE_RINGING  ;
   else
      return CALL_STATE_FAILURE  ;
} //getStartStateFromDaemonCallState

/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

///Transfer state from internal to daemon internal syntaz
daemon_call_state Call::toDaemonCallState(const QString& stateName)
{
   if(stateName == QString(CALL_STATE_CHANGE_HUNG_UP)        )
      return DAEMON_CALL_STATE_HUNG_UP ;
   if(stateName == QString(CALL_STATE_CHANGE_RINGING)        )
      return DAEMON_CALL_STATE_RINGING ;
   if(stateName == QString(CALL_STATE_CHANGE_CURRENT)        )
      return DAEMON_CALL_STATE_CURRENT ;
   if(stateName == QString(CALL_STATE_CHANGE_UNHOLD_CURRENT) )
      return DAEMON_CALL_STATE_CURRENT ;
   if(stateName == QString(CALL_STATE_CHANGE_HOLD)           )
      return DAEMON_CALL_STATE_HOLD    ;
   if(stateName == QString(CALL_STATE_CHANGE_BUSY)           )
      return DAEMON_CALL_STATE_BUSY    ;
   if(stateName == QString(CALL_STATE_CHANGE_FAILURE)        )
      return DAEMON_CALL_STATE_FAILURE ;
   
   qDebug() << "stateChanged signal received with unknown state.";
   return DAEMON_CALL_STATE_FAILURE    ;
} //toDaemonCallState

///Transform a conference call state to a proper call state
call_state Call::confStatetoCallState(const QString& stateName)
{
   if      ( stateName == CONF_STATE_CHANGE_HOLD   )
      return CALL_STATE_CONFERENCE_HOLD;
   else if ( stateName == CONF_STATE_CHANGE_ACTIVE )
      return CALL_STATE_CONFERENCE;
   else
      return CALL_STATE_ERROR; //Well, this may bug a little
}

///Transform a backend state into a translated string
const QString Call::toHumanStateName() const
{
   switch (m_CurrentState) {
      case CALL_STATE_INCOMING:
         return ( "Ringing (in)"      );
         break;
      case CALL_STATE_RINGING:
         return ( "Ringing (out)"     );
         break;
      case CALL_STATE_CURRENT:
         return ( "Talking"           );
         break;
      case CALL_STATE_DIALING:
         return ( "Dialing"           );
         break;
      case CALL_STATE_HOLD:
         return ( "Hold"              );
         break;
      case CALL_STATE_FAILURE:
         return ( "Failed"            );
         break;
      case CALL_STATE_BUSY:
         return ( "Busy"              );
         break;
      case CALL_STATE_TRANSFERRED:
         return ( "Transfer"          );
         break;
      case CALL_STATE_TRANSF_HOLD:
         return ( "Transfer hold"     );
         break;
      case CALL_STATE_OVER:
         return ( "Over"              );
         break;
      case CALL_STATE_ERROR:
         return ( "Error"             );
         break;
      case CALL_STATE_CONFERENCE:
         return ( "Conference"        );
         break;
      case CALL_STATE_CONFERENCE_HOLD:
         return ( "Conference (hold)" );
      default:
         return "";
   }
}

///Get the time (second from 1 jan 1970) when the call ended
QString Call::getStopTimeStamp()     const
{
   if (m_pStopTime == nullptr)
      return QString("0");
   return QString::number(m_pStopTime->toTime_t());
}

///Get the time (second from 1 jan 1970) when the call started
QString Call::getStartTimeStamp()    const
{
   if (m_pStartTime == nullptr)
      return QString("0");
   return QString::number(m_pStartTime->toTime_t());
}

///Get the number where the call have been transferred
const QString& Call::getTransferNumber()    const
{
   return m_TransferNumber;
}

///Get the call / peer number
const QString& Call::getCallNumber()        const
{
   return m_CallNumber;
}

///Return the call id
const QString& Call::getCallId()            const
{
   return m_CallId;
}

///Return the peer phone number
const QString& Call::getPeerPhoneNumber()   const
{
   return m_PeerPhoneNumber;
}

///Get the peer name
const QString& Call::getPeerName()          const
{
   return m_PeerName;
}

///Generate the best possible peer name
const QString Call::getFormattedName()
{
   if (isConference())
      return "Conference";
   else if (m_pContact && !m_pContact->getFormattedName().isEmpty())
      return m_pContact->getFormattedName();
   else if (!getPeerName().isEmpty())
      return m_PeerName;
   else
      return m_PeerPhoneNumber;
}

///If the call have a valid record
bool Call::hasRecording()                   const
{
   return !getRecordingPath().isEmpty() && QFile::exists(getRecordingPath());
}

///Get the current state
call_state Call::getCurrentState()          const
{
   return m_CurrentState;
}

///Get the call recording
bool Call::getRecording()                   const
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   ((Call*) this)->m_Recording        = callManager.getIsRecording(m_CallId);
   return m_Recording;
}

///Get the call account id
Account* Call::getAccount()                 const
{
   return AccountList::getInstance()->getAccountById(m_Account);
}

///Is this call a conference
bool Call::isConference()                   const
{
   return m_isConference;
}

///Get the conference ID
const QString& Call::getConfId()            const
{
   return m_ConfId;
}

///Get the recording path
const QString& Call::getRecordingPath()     const
{
   return m_RecordingPath;
}

///Get the current codec
QString Call::getCurrentCodecName()         const
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   return callManager.getCurrentAudioCodecName(m_CallId);
}

///Get the state
call_state Call::getState()                 const
{
   return m_CurrentState;
}

///Get the history state
history_state Call::getHistoryState()       const
{
   return m_HistoryState;
}

///Is this call over?
bool Call::isHistory()                      const
{
   return (getState() == CALL_STATE_OVER);
}

///Is this call selected (useful for GUIs)
bool Call::isSelected() const
{
   return m_sSelectedCall == this;
}

///This function could also be called mayBeSecure or haveChancesToBeEncryptedButWeCantTell.
bool Call::isSecure() const {

   if (m_Account.isEmpty()) {
      qDebug() << "Account not set, can't check security";
      return false;
   }

   Account* currentAccount = AccountList::getInstance()->getAccountById(m_Account);
   return currentAccount && ((currentAccount->isTlsEnable()) || (currentAccount->getTlsMethod()));
} //isSecure

Contact* Call::getContact()
{
   if (!m_pContact && m_ContactChanged) {
      m_pContact = m_pContactBackend->getContactByPhone(m_PeerPhoneNumber,true,getAccount());
   }
   return m_pContact;
}

///Return the renderer associated with this call or nullptr
VideoRenderer* Call::getVideoRenderer()
{
   return VideoModel::getInstance()->getRenderer(this);
}


/*****************************************************************************
 *                                                                           *
 *                                  Setters                                  *
 *                                                                           *
 ****************************************************************************/

///Set the transfer number
void Call::setTransferNumber(const QString& number)
{
   m_TransferNumber = number;
}

///This call is a conference
void Call::setConference(bool value)
{
   m_isConference = value;
}

///Set the call number
void Call::setCallNumber(const QString& number)
{
   m_CallNumber = number;
   emit changed();
}

///Set the conference ID
void Call::setConfId(QString value)
{
   m_ConfId = value;
}

///Set the recording path
void Call::setRecordingPath(const QString& path)
{
   m_RecordingPath = path;
}

///Set peer name
void Call::setPeerName(const QString& name)
{
   m_PeerName = name;
}

///Set selected
void Call::setSelected(const bool value)
{
   if (value) {
      m_sSelectedCall = this;
   }
}

/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

///The call state just changed
call_state Call::stateChanged(const QString& newStateName)
{
   call_state previousState = m_CurrentState;
   if (!m_isConference) {
      daemon_call_state dcs = toDaemonCallState(newStateName);
      changeCurrentState(stateChangedStateMap[m_CurrentState][dcs]);

      CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
      MapStringString details = callManager.getCallDetails(m_CallId).value();
      if (details[CALL_PEER_NAME] != m_PeerName)
         m_PeerName = details[CALL_PEER_NAME];

      (this->*(stateChangedFunctionMap[previousState][dcs]))();
   }
   else {
      //Until now, it does not worth using stateChangedStateMap, conferences are quite simple
      m_CurrentState = confStatetoCallState(newStateName);
   }
   qDebug() << "Calling stateChanged " << newStateName << " -> " << toDaemonCallState(newStateName) << " on call with state " << previousState << ". Become " << m_CurrentState;
   return m_CurrentState;
} //stateChanged

///An account have been performed
call_state Call::actionPerformed(call_action action)
{
   call_state previousState = m_CurrentState;
   //update the state
   if (previousState < 13 && action < 5) {
      changeCurrentState(actionPerformedStateMap[previousState][action]);
      //execute the action associated with this transition
      (this->*(actionPerformedFunctionMap[previousState][action]))();
      qDebug() << "Calling action " << action << " on call with state " << previousState << ". Become " << m_CurrentState;
      //return the new state
   }
   return m_CurrentState;
} //actionPerformed

///Change the state
void Call::changeCurrentState(call_state newState)
{
   m_CurrentState = newState;

   emit changed();

   if (m_CurrentState == CALL_STATE_OVER)
      emit isOver(this);
}

///Send a text message
void Call::sendTextMessage(QString message)
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   Q_NOREPLY callManager.sendTextMessage(isConference()?m_ConfId:m_CallId,message);
   qDebug() << "MODEL IS" << m_pImModel;
   if (!m_pImModel) {
        m_pImModel = InstantMessagingModelManager::getInstance()->getModel(this);
   }
   qDebug() << "MODEL IS2" << m_pImModel;
   m_pImModel->addOutgoingMessage(message);
}

///Private version of setStartTime
QDateTime* Call::setStartTime_private(QDateTime* time)
{
   //if (m_pStartTime && time)
   //   delete m_pStartTime;
   
   return m_pStartTime = time;
}

///Private version of setStopTime
QDateTime* Call::setStopTime_private(QDateTime* time)
{
   //if (m_pStopTime && time)
   //   delete m_pStopTime;
   return m_pStopTime  = time;
}

/*****************************************************************************
 *                                                                           *
 *                              Automate function                            *
 *                                                                           *
 ****************************************************************************/
///@warning DO NOT TOUCH THAT, THEY ARE CALLED FROM AN ARRAY, HIGH FRAGILITY

///Do nothing (literally)
void Call::nothing()
{
   
}

///Accept the call
void Call::accept()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Accepting call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   Q_NOREPLY callManager.accept(m_CallId);
   setStartTime_private(new QDateTime(QDateTime::currentDateTime()));
   this->m_HistoryState = INCOMING;
}

///Refuse the call
void Call::refuse()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Refusing call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   Q_NOREPLY callManager.refuse(m_CallId);
   setStartTime_private(new QDateTime(QDateTime::currentDateTime()));
   this->m_HistoryState = MISSED;
}

///Accept the transfer
void Call::acceptTransf()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Accepting call and transferring it to number : " << m_TransferNumber << ". callId : " << m_CallId  << "ConfId:" << m_ConfId;
   callManager.accept(m_CallId);
   Q_NOREPLY callManager.transfer(m_CallId, m_TransferNumber);
}

///Put the call on hold
void Call::acceptHold()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Accepting call and holding it. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   callManager.accept(m_CallId);
   Q_NOREPLY callManager.hold(m_CallId);
   this->m_HistoryState = INCOMING;
}

///Hang up
void Call::hangUp()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   setStopTime_private(new QDateTime(QDateTime::currentDateTime()));
   qDebug() << "Hanging up call. callId : " << m_CallId << "ConfId:" << m_ConfId;
   if (!isConference())
      Q_NOREPLY callManager.hangUp(m_CallId);
   else
      Q_NOREPLY callManager.hangUpConference(m_ConfId);
}

///Cancel this call
void Call::cancel()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Canceling call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   Q_NOREPLY callManager.hangUp(m_CallId);
}

///Put on hold
void Call::hold()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Holding call. callId : " << m_CallId << "ConfId:" << m_ConfId;
   if (!isConference())
      Q_NOREPLY callManager.hold(m_CallId);
   else
      Q_NOREPLY callManager.holdConference(m_ConfId);
}

///Start the call
void Call::call()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "account = " << m_Account;
   if(m_Account.isEmpty()) {
      qDebug() << "Account is not set, taking the first registered.";
      this->m_Account = AccountList::getCurrentAccount()->getAccountId();
   }
   if(!m_Account.isEmpty()) {
      qDebug() << "Calling " << m_CallNumber << " with account " << m_Account << ". callId : " << m_CallId  << "ConfId:" << m_ConfId;
      callManager.placeCall(m_Account, m_CallId, m_CallNumber);
      this->m_Account = m_Account;
      this->m_PeerPhoneNumber = m_CallNumber;
      m_ContactChanged = true;
      if (m_pContactBackend) {
         if (getContact())
            m_PeerName = m_pContact->getFormattedName();
      }
      setStartTime_private(new QDateTime(QDateTime::currentDateTime()));
      this->m_HistoryState = OUTGOING;
   }
   else {
      qDebug() << "Trying to call " << m_TransferNumber << " with no account registered . callId : " << m_CallId  << "ConfId:" << m_ConfId;
      this->m_HistoryState = NONE;
      throw "No account registered!";
   }
}

///Trnasfer the call
void Call::transfer()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Transferring call to number : " << m_TransferNumber << ". callId : " << m_CallId;
   Q_NOREPLY callManager.transfer(m_CallId, m_TransferNumber);
   setStopTime_private(new QDateTime(QDateTime::currentDateTime()));
}

///Unhold the call
void Call::unhold()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Unholding call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   if (!isConference())
      Q_NOREPLY callManager.unhold(m_CallId);
   else
      Q_NOREPLY callManager.unholdConference(m_ConfId);
}

///Record the call
void Call::setRecord()
{
   CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
   qDebug() << "Setting record " << !m_Recording << " for call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   Q_NOREPLY callManager.setRecording((!m_isConference)?m_CallId:m_ConfId);
   bool oldRecStatus = m_Recording;
   m_Recording = !m_Recording;
   if (oldRecStatus != m_Recording)
      emit changed();
}

///Start the timer
void Call::start()
{
   qDebug() << "Starting call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   setStartTime_private(new QDateTime(QDateTime::currentDateTime()));
}

///Toggle the timer
void Call::startStop()
{
   qDebug() << "Starting and stoping call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   setStartTime_private(new QDateTime(QDateTime::currentDateTime()));
   setStopTime_private(new QDateTime(QDateTime::currentDateTime()));
}

///Stop the timer
void Call::stop()
{
   qDebug() << "Stoping call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   setStopTime_private(new QDateTime(QDateTime::currentDateTime()));
}

///Handle error instead of crashing
void Call::startWeird()
{
   qDebug() << "Starting call. callId : " << m_CallId  << "ConfId:" << m_ConfId;
   setStartTime_private(new QDateTime(QDateTime::currentDateTime()));
   qDebug() << "Warning : call " << m_CallId << " had an unexpected transition of state at its start.";
}

///Print a warning
void Call::warning()
{
   qDebug() << "Warning : call " << m_CallId << " had an unexpected transition of state.";
}

/*****************************************************************************
 *                                                                           *
 *                             Keyboard handling                             *
 *                                                                           *
 ****************************************************************************/

///Input text on the call item
void Call::appendText(const QString& str)
{
   QString* editNumber;
   
   switch (m_CurrentState) {
   case CALL_STATE_TRANSFERRED :
   case CALL_STATE_TRANSF_HOLD :
      editNumber = &m_TransferNumber;
      break;
   case CALL_STATE_DIALING     :
      editNumber = &m_CallNumber;
      break;
   default                     :
      qDebug() << "Backspace on call not editable. Doing nothing.";
      return;
   }

   editNumber->append(str);

   emit changed();
}

///Remove the last character
void Call::backspaceItemText()
{
   QString* editNumber;

   switch (m_CurrentState) {
      case CALL_STATE_TRANSFERRED      :
      case CALL_STATE_TRANSF_HOLD      :
         editNumber = &m_TransferNumber;
         break;
      case CALL_STATE_DIALING          :
         editNumber = &m_CallNumber;
         break;
      default                          :
         qDebug() << "Backspace on call not editable. Doing nothing.";
         return;
   }
   QString text = *editNumber;
   int textSize = text.size();
   if(textSize > 0) {
      *editNumber = text.remove(textSize-1, 1);
      emit changed();
   }
   else {
      changeCurrentState(CALL_STATE_OVER);
   }
}


/*****************************************************************************
 *                                                                           *
 *                                   SLOTS                                   *
 *                                                                           *
 ****************************************************************************/

///Play the record, if any
void Call::playRecording()
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   bool retval = callManager.startRecordedFilePlayback(getRecordingPath());
   if (retval)
      emit playbackStarted();
}

///Stop the record, if any
void Call::stopRecording()
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   callManager.stopRecordedFilePlayback(getRecordingPath());
   emit playbackStopped(); //TODO remove this, it is a workaround for bug #11942
}

///seek the record, if any
void Call::seekRecording(double position)
{
   CallManagerInterface& callManager = CallManagerInterfaceSingleton::getInstance();
   callManager.recordPlaybackSeek(position);
}

///Daemon record playback stopped
void Call::stopPlayback(QString filePath)
{
   if (filePath == getRecordingPath()) {
      emit playbackStopped();
   }
}

///Daemon playback position chnaged
void Call::updatePlayback(int position,int size)
{
   emit playbackPositionChanged(position,size);
}

///Called to notice the call thatits contact might be outdated
void Call::contactBackendChanged()
{
   m_ContactChanged = true;
}
