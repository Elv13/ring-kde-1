
/***************************************************************************
 *   Copyright (C) 2009 by Savoir-Faire Linux                              *
 *   Author : Jérémy Quentin                                               *
 *   jeremy.quentin@savoirfairelinux.com                                   *
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
 ***************************************************************************/

#include "SFLPhoneView.h"

#include <QtGui/QLabel>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QBrush>
#include <QtGui/QPalette>
#include <QtGui/QInputDialog>

#include <klocale.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kmenu.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseelist.h>

#include "sflphone_const.h"
#include "conf/ConfigurationSkeleton.h"
#include "configurationmanager_interface_singleton.h"
#include "callmanager_interface_singleton.h"
#include "instance_interface_singleton.h"
#include "ActionSetAccountFirst.h"
#include "ContactItemWidget.h"
#include "SFLPhone.h"
#include "typedefs.h"
#include "Dialpad.h"
#include "CallTreeView.h"
#include "CallTreeItem.h"


using namespace KABC;

ConfigurationDialog * SFLPhoneView::configDialog;
AccountList * SFLPhoneView::accountList;
QString SFLPhoneView::priorAccountId;

SFLPhoneView::SFLPhoneView(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	
	errorWindow = new QErrorMessage(this);

	callList = new CallList(this);

	callTree = new CallTreeView(page_callList);
	historyTree = new CallTreeView(page_callHistory);

	page_callList->layout()->addWidget(callTree);
	page_callHistory->layout()->addWidget(historyTree);

	historyLoaded = false;

	for(int i = 0 ; i < callList->size() ; i++)
	{
		Call * call = (*callList)[i];
		if(call->getState() == CALL_STATE_OVER)
		{
			addCallToCallHistory(call);
		}
		else
		{
			addCallToCallList(call);
		}
	}
	
	accountList = new AccountList(false);
	accountList->updateAccounts();
	
	configDialog = new ConfigurationDialog(this);
	configDialog->setObjectName("configDialog");
	configDialog->setModal(true);
	
	wizard = new AccountWizard(this);
	wizard->setModal(false);
	
	QPalette pal = QPalette(palette());
	pal.setColor(QPalette::AlternateBase, Qt::lightGray);
	setPalette(pal);
		
	stackedWidget_screen->setCurrentWidget(page_callList);
	
	connect(&callManager, SIGNAL(callStateChanged(const QString &, const QString &)),
	        this,         SLOT(on1_callStateChanged(const QString &, const QString &)));
	connect(&callManager, SIGNAL(incomingCall(const QString &, const QString &, const QString &)),
	        this,         SLOT(on1_incomingCall(const QString &, const QString &)));
	connect(&callManager, SIGNAL(incomingMessage(const QString &, const QString &)),
	        this,         SLOT(on1_incomingMessage(const QString &, const QString &)));
	connect(&callManager, SIGNAL(voiceMailNotify(const QString &, int)),
	        this,         SLOT(on1_voiceMailNotify(const QString &, int)));
	connect(&callManager, SIGNAL(volumeChanged(const QString &, double)),
	        this,         SLOT(on1_volumeChanged(const QString &, double)));
	
	connect(&configurationManager, SIGNAL(accountsChanged()),
	        accountList,           SLOT(updateAccounts()));

	connect(&configurationManager, SIGNAL(audioManagerChanged()),
		this,         SLOT(on1_audioManagerChanged()));
	        
	connect(configDialog, SIGNAL(clearCallHistoryAsked()),
	        callList,     SLOT(clearHistory()));
	        
	connect(configDialog, SIGNAL(changesApplied()),
	        this,         SLOT(loadWindow()));
	        
	connect(accountList, SIGNAL(accountListUpdated()),
	        this,        SLOT(updateStatusMessage()));
	connect(accountList, SIGNAL(accountListUpdated()),
	        this,        SLOT(updateWindowCallState()));
	        
} 



SFLPhoneView::~SFLPhoneView()
{
}

void SFLPhoneView::saveState()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	configurationManager.setHistory(callList->getHistoryMap());
}

void SFLPhoneView::loadWindow()
{
	qDebug() << "\nloadWindow";
	updateWindowCallState();
	updateRecordButton();
	updateVolumeButton();
	updateRecordBar();
	updateVolumeBar();
	updateVolumeControls();
	updateDialpad();
	updateSearchHistory();
	updateAddressBookEnabled();
	updateAddressBook();
	updateStatusMessage();
	qDebug() << "Finished loadWindow\n";
}

Account * SFLPhoneView::accountInUse()
{
	Account * priorAccount = accountList->getAccountById(priorAccountId);
	if(priorAccount && priorAccount->getAccountDetail(ACCOUNT_STATUS) == ACCOUNT_STATE_REGISTERED )
	{
		return priorAccount;
	}
	else
	{
		return accountList->firstRegisteredAccount();
	}
}

QString SFLPhoneView::accountInUseId()
{
	Account * firstRegistered = accountInUse();
	if(firstRegistered == NULL)
	{
		return QString();
	}
	else
	{
		return firstRegistered->getAccountId();
	}
}

AccountList * SFLPhoneView::getAccountList()
{
	return accountList;
}

QErrorMessage * SFLPhoneView::getErrorWindow()
{
	return errorWindow;
}

CallTreeItem* SFLPhoneView::addCallToCallList(Call * call)
{	  
	return callTree->insert(call);
}

CallTreeItem* SFLPhoneView::addCallToCallHistory(Call * call)
{
	return historyTree->insert(call);

}

void SFLPhoneView::addContactToContactList(Contact * contact)
{
	QListWidgetItem * item = contact->getItem();
	QWidget * widget = contact->getItemWidget();
	if(item && widget)
	{
		listWidget_addressBook->addItem(item);
		listWidget_addressBook->setItemWidget(item, widget);
	}
}

void SFLPhoneView::typeString(QString str)
{
	qDebug() << "typeString";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	
	if(stackedWidget_screen->currentWidget() == page_callList)
	{
		CallTreeItem * item = callTree->currentItem();
		callManager.playDTMF(str);
		Call *currentCall = 0;
		Call *candidate = 0;

		if(item)
		{
			Call *call = item->call();

			if (call)
			{
				if(call->getState() == CALL_STATE_CURRENT)
				{
					currentCall = call;
				}
			}
		}

		for(int i = callList->size() - 1 ; i >= 0 ; i--)
		{
			Call *call = (*callList)[i];

			if(currentCall != call && call->getState() == CALL_STATE_CURRENT)
			{

				action(call, CALL_ACTION_HOLD);
			}
			else if(call->getState() == CALL_STATE_DIALING)
			{
				candidate = call;
			}
		}
		
		if(!currentCall && !candidate)
		{
			qDebug() << "Typing when no item is selected. Opening an item.";
			candidate = callList->addDialingCall();
			addCallToCallList(candidate);
			callTree->setCurrentRow(callTree->count() + 1);
		}

		if(!currentCall && candidate)
		{
			candidate->appendText(str);
		}
	}
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		qDebug() << "In call history.";
		lineEdit_searchHistory->setText(lineEdit_searchHistory->text() + str);
		lineEdit_searchHistory->setFocus();
	}
	if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		qDebug() << "In address book.";
		lineEdit_addressBook->setText(lineEdit_addressBook->text() + str);
		lineEdit_addressBook->setFocus();
	}
}

void SFLPhoneView::backspace()
{
	qDebug() << "backspace";
	if(stackedWidget_screen->currentWidget() == page_callList)
	{
		qDebug() << "In call list.";
		CallTreeItem * item = callTree->currentItem();
		if(!item)
		{
			qDebug() << "Backspace when no item is selected. Doing nothing.";
		}
		else
		{
			Call * call = callTree->currentItem()->call();
			if(!call)
			{
				qDebug() << "Error : Backspace on unexisting call.";
			}
			else
			{
				call->backspaceItemText();
				if(call->getState() == CALL_STATE_OVER)
				{
					callTree->removeCurrent();

					if(call->getHistoryState() != NONE)
					{
						historyTree->insert(call);
					}
				}
			}
		}
	}
}

void SFLPhoneView::escape()
{
	qDebug() << "escape";
	if(stackedWidget_screen->currentWidget() == page_callList )
	{
		qDebug() << "In call list.";
		CallTreeItem * item = callTree->currentItem();
		if(!item)
		{
			qDebug() << "Escape when no item is selected. Doing nothing.";
		}
		else
		{
			Call * call = item->call();

			if(!call)
			{
				qDebug() << "Error : Escape on unexisting call.";
			}
			else
			{
				if(call->getState() == CALL_STATE_TRANSFER ||call->getState() == CALL_STATE_TRANSF_HOLD)
				{
					action(call, CALL_ACTION_TRANSFER);
				}
				else
				{
					action(call, CALL_ACTION_REFUSE);
				}
			}
		}
	}
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		qDebug() << "In call history.";
		lineEdit_searchHistory->clear();
	}
	if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		qDebug() << "In address book.";
		lineEdit_addressBook->clear();
	}
}

void SFLPhoneView::enter()
{
	qDebug() << "enter";
	if(stackedWidget_screen->currentWidget() == page_callList )
	{
		qDebug() << "In call list.";
		CallTreeItem * item = callTree->currentItem();
		if(!item)
		{
			qDebug() << "Enter when no item is selected. Doing nothing.";
		}
		else
		{
			Call * call = item->call();
			if(!call)
			{
				qDebug() << "Error : Enter on unexisting call.";
			}
			else
			{
				int state = call->getState();
				if(state == CALL_STATE_INCOMING || state == CALL_STATE_DIALING || state == CALL_STATE_TRANSFER || state == CALL_STATE_TRANSF_HOLD)
				{
					action(call, CALL_ACTION_ACCEPT);
				}
				else
				{
					qDebug() << "Enter when call selected not in appropriate state. Doing nothing.";
				}
			}
		}
	}
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		qDebug() << "In call history.";
		CallTreeItem * item = historyTree->currentItem();
		if(!item)
		{
			qDebug() << "Enter when no item is selected. Doing nothing.";
		}
		else
		{
			changeScreen(SCREEN_MAIN);
			
			Call * pastCall = item->call();
			if (!pastCall)
			{
				qDebug() << "pastCall null";
			}
			Call * call = callList->addDialingCall(pastCall->getPeerName(), pastCall->getAccountId());
			call->appendText(pastCall->getPeerPhoneNumber());
			addCallToCallList(call);
			callTree->setCurrentRow(callTree->count() - 1);
			action(call, CALL_ACTION_ACCEPT);
		}
	}
	if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		qDebug() << "In address book.";
		QListWidgetItem * item = listWidget_addressBook->currentItem();
		if(!item)
		{
			qDebug() << "Enter when no item is selected. Doing nothing.";
		}
		else
		{
			changeScreen(SCREEN_MAIN);
			ContactItemWidget * w = (ContactItemWidget *) (listWidget_addressBook->itemWidget(item));
			Call * call = callList->addDialingCall(w->getContactName());
			call->appendText(w->getContactNumber());
			addCallToCallList(call);
			callTree->setCurrentRow(callTree->count() - 1);
			action(call, CALL_ACTION_ACCEPT);
		}
	}
}

void SFLPhoneView::action(Call * call, call_action action)
{
	if(! call)
	{
		qDebug() << "Error : action " << action << "applied on null object call. Should not happen.";
	}
	else
	{
		try
		{
			call->actionPerformed(action);
		}
		catch(const char * msg)
		{
			errorWindow->showMessage(QString(msg));
		}
		
		//updateCallItem(call);
		updateWindowCallState();
	}
}


/*******************************************
******** Update Display Functions **********
*******************************************/

void SFLPhoneView::updateWindowCallState()
{
	qDebug() << "updateWindowCallState";
	
	bool enabledActions[6]= {true,true,true,true,true,true};
	QString buttonIconFiles[6] = {ICON_CALL, ICON_HANGUP, ICON_HOLD, ICON_TRANSFER, ICON_REC_DEL_OFF, ICON_MAILBOX};
	QString actionTexts[6] = {ACTION_LABEL_CALL, ACTION_LABEL_HANG_UP, ACTION_LABEL_HOLD, ACTION_LABEL_TRANSFER, ACTION_LABEL_RECORD, ACTION_LABEL_MAILBOX};
	
	CallTreeItem * item;
	
	bool transfer = false;
	//tells whether the call is in recording position
	bool recordActivated = false;
	enabledActions[SFLPhone::Mailbox] = accountInUse() && ! accountInUse()->getAccountDetail(ACCOUNT_MAILBOX).isEmpty();
	if(stackedWidget_screen->currentWidget() == page_callList)
	{
		item = callTree->currentItem();
		if (!item)
		{
			qDebug() << "No item selected.";
			enabledActions[SFLPhone::Refuse] = false;
			enabledActions[SFLPhone::Hold] = false;
			enabledActions[SFLPhone::Transfer] = false;
			enabledActions[SFLPhone::Record] = false;
		}
		else
		{
			Call * call = item->call();	

			if (call)
			{
				call_state state = call->getState();
				recordActivated = call->getRecording();
				switch (state)
				{
				case CALL_STATE_INCOMING:
					qDebug() << "Reached CALL_STATE_INCOMING with call " << call->getCallId();
					buttonIconFiles[SFLPhone::Accept] = ICON_ACCEPT;
					buttonIconFiles[SFLPhone::Refuse] = ICON_REFUSE;
					actionTexts[SFLPhone::Accept] = ACTION_LABEL_ACCEPT;
					actionTexts[SFLPhone::Refuse] = ACTION_LABEL_REFUSE;
					break;
				case CALL_STATE_RINGING:
					qDebug() << "Reached CALL_STATE_RINGING with call " << call->getCallId();
					enabledActions[SFLPhone::Hold] = false;
					enabledActions[SFLPhone::Transfer] = false;
					break;
				case CALL_STATE_CURRENT:
					qDebug() << "details = " << CallManagerInterfaceSingleton::getInstance().getCallDetails(call->getCallId()).value();
					qDebug() << "Reached CALL_STATE_CURRENT with call " << call->getCallId();
					buttonIconFiles[SFLPhone::Record] = ICON_REC_DEL_ON;
					break;
				case CALL_STATE_DIALING:
					qDebug() << "Reached CALL_STATE_DIALING with call " << call->getCallId();
					enabledActions[SFLPhone::Hold] = false;
					enabledActions[SFLPhone::Transfer] = false;
					enabledActions[SFLPhone::Record] = false;
					actionTexts[SFLPhone::Accept] = ACTION_LABEL_ACCEPT;
					buttonIconFiles[SFLPhone::Accept] = ICON_ACCEPT;
					break;
				case CALL_STATE_HOLD:
					qDebug() << "Reached CALL_STATE_HOLD with call " << call->getCallId();
					buttonIconFiles[SFLPhone::Hold] = ICON_UNHOLD;
					actionTexts[SFLPhone::Hold] = ACTION_LABEL_UNHOLD;
					break;		
				case CALL_STATE_FAILURE:
					qDebug() << "Reached CALL_STATE_FAILURE with call " << call->getCallId();
					enabledActions[SFLPhone::Accept] = false;
					enabledActions[SFLPhone::Hold] = false;
					enabledActions[SFLPhone::Transfer] = false;
					enabledActions[SFLPhone::Record] = false;
					break;
				case CALL_STATE_BUSY:
					qDebug() << "Reached CALL_STATE_BUSY with call " << call->getCallId();
					enabledActions[SFLPhone::Accept] = false;
					enabledActions[SFLPhone::Hold] = false;
					enabledActions[SFLPhone::Transfer] = false;
					enabledActions[SFLPhone::Record] = false;
				break;
				case CALL_STATE_TRANSFER:
					qDebug() << "Reached CALL_STATE_TRANSFER with call " << call->getCallId();
					buttonIconFiles[SFLPhone::Accept] = ICON_EXEC_TRANSF;
					actionTexts[SFLPhone::Transfer] = ACTION_LABEL_GIVE_UP_TRANSF;
					transfer = true;
					buttonIconFiles[SFLPhone::Record] = ICON_REC_DEL_ON;
					break;
				case CALL_STATE_TRANSF_HOLD:
					qDebug() << "Reached CALL_STATE_TRANSF_HOLD with call " << call->getCallId();
					buttonIconFiles[SFLPhone::Accept] = ICON_EXEC_TRANSF;
					buttonIconFiles[SFLPhone::Hold] = ICON_UNHOLD;
					actionTexts[SFLPhone::Transfer] = ACTION_LABEL_GIVE_UP_TRANSF;
					actionTexts[SFLPhone::Hold] = ACTION_LABEL_UNHOLD;
					transfer = true;
					break;
				case CALL_STATE_OVER:
					qDebug() << "Error : Reached CALL_STATE_OVER with call " << call->getCallId() << "!";
					break;
				case CALL_STATE_ERROR:
					qDebug() << "Error : Reached CALL_STATE_ERROR with call " << call->getCallId() << "!";
					break;
				default:
					qDebug() << "Error : Reached unexisting state for call " << call->getCallId() << "!";
					break;
				}
			}
		}
	}
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		item = historyTree->currentItem();
		buttonIconFiles[SFLPhone::Accept] = ICON_ACCEPT;
		actionTexts[SFLPhone::Accept] = ACTION_LABEL_CALL_BACK;
		if (!item)
		{
			qDebug() << "No item selected.";
			enabledActions[SFLPhone::Accept] = false;
			enabledActions[SFLPhone::Refuse] = false;
			enabledActions[SFLPhone::Hold] = false;
			enabledActions[SFLPhone::Transfer] = false;
			enabledActions[SFLPhone::Record] = false;
		}
		else
		{
			enabledActions[SFLPhone::Refuse] = false;
			enabledActions[SFLPhone::Hold] = false;
			enabledActions[SFLPhone::Transfer] = false;
			enabledActions[SFLPhone::Record] = false;
		}
	}
	if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		item = addressBookTree->currentItem();
		buttonIconFiles[SFLPhone::Accept] = ICON_ACCEPT;
		if (!item)
		{
			qDebug() << "No item selected.";
			enabledActions[SFLPhone::Accept] = false;
			enabledActions[SFLPhone::Refuse] = false;
			enabledActions[SFLPhone::Hold] = false;
			enabledActions[SFLPhone::Transfer] = false;
			enabledActions[SFLPhone::Record] = false;
		}
		else
		{
			enabledActions[SFLPhone::Refuse] = false;
			enabledActions[SFLPhone::Hold] = false;
			enabledActions[SFLPhone::Transfer] = false;
			enabledActions[SFLPhone::Record] = false;
		}
	}
	
	qDebug() << "Updating Window.";
	
	emit enabledActionsChangeAsked(enabledActions);
	emit actionIconsChangeAsked(buttonIconFiles);
	emit actionTextsChangeAsked(actionTexts);
	emit transferCheckStateChangeAsked(transfer);
	emit recordCheckStateChangeAsked(recordActivated);

	qDebug() << "Window updated.";
}

void SFLPhoneView::updateSearchHistory()
{
	qDebug() << "updateSearchHistory";
	lineEdit_searchHistory->setVisible(!lineEdit_searchHistory->text().isEmpty());
}


void SFLPhoneView::updateCallHistory()
{
	qDebug() << "updateCallHistory";
	while(listWidget_callHistory->count() > 0)
	{
		listWidget_callHistory->takeItem(0);
	}
	QString textSearched = lineEdit_searchHistory->text();
	for(int i = callList->size() - 1 ; i >= 0 ; i--)
	{
		Call * call = (*callList)[i];
		qDebug() << "" << call->getCallId();
		if(
		     call->getState() == CALL_STATE_OVER && 
		     call->getHistoryState() != NONE && 
		    (call->getPeerPhoneNumber().contains(textSearched) || call->getPeerName().contains(textSearched))
		  )
		{
			qDebug() << "call->getPeerPhoneNumber()=" << call->getPeerPhoneNumber() << " contains textSearched=" << textSearched;
			addCallToCallHistory(call);
		}
	}
// 	alternateColors(listWidget_callHistory);
}

void SFLPhoneView::updateAddressBook()
{
	qDebug() << "updateAddressBook";
	while(listWidget_addressBook->count() > 0)
	{
		QListWidgetItem * item = listWidget_addressBook->takeItem(0);
		delete item;
	}
	if(isAddressBookEnabled())
	{
		if(loadAddressBook())
		{
			qDebug() << "add loaded";
			QString textSearched = lineEdit_addressBook->text();
			if(textSearched.isEmpty())
			{
				label_addressBookFull->setVisible(false);
				return;
			}
			bool full = false;
			QVector<Contact *> contactsFound = findContactsInKAddressBook(textSearched, full);
			qDebug() << "Full : " << full;
			label_addressBookFull->setVisible(full);
			for(int i = 0 ; i < contactsFound.size() ; i++)
			{
				Contact * contact = contactsFound[i];
				addContactToContactList(contact);
			}
			alternateColors(listWidget_addressBook);
		}
		else
		{
			lineEdit_addressBook->setClickMessage(i18n("Address book loading..."));
			lineEdit_addressBook->setEnabled(false);
			label_addressBookFull->setVisible(false);
		}
	}
	
}

void SFLPhoneView::alternateColors(QListWidget * listWidget)
{
	for(int i = 0 ; i < listWidget->count(); i++)
	{
		QListWidgetItem* item = listWidget->item(i);
		QBrush c = (i % 2 == 1) ? palette().base() : palette().alternateBase();
		item->setBackground( c );
	}
	listWidget->setUpdatesEnabled( true );

}

QVector<Contact *> SFLPhoneView::findContactsInKAddressBook(QString textSearched, bool & full)
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	MapStringInt addressBookSettings = configurationManager.getAddressbookSettings().value();
	int maxResults = addressBookSettings[ADDRESSBOOK_MAX_RESULTS];
	int typesDisplayed = phoneNumberTypesDisplayed();
	bool displayPhoto = addressBookSettings[ADDRESSBOOK_DISPLAY_CONTACT_PHOTO];
	AddressBook * ab = KABC::StdAddressBook::self(true);
	QVector<Contact *> results = QVector<Contact *>();
	AddressBook::Iterator it;
	full = false;
	int k = 0;
	for ( it = ab->begin(); it != ab->end() && !full ; it++ ) {
		if(it->name().contains(textSearched, Qt::CaseInsensitive) || it->nickName().contains(textSearched, Qt::CaseInsensitive))
		{
			for(int i = 0 ; i < it->phoneNumbers().count() ; i++)
			{
				int typeFlag = it->phoneNumbers().at(i).type();
				if((typesDisplayed & typeFlag) != 0)
				{
					results.append(new Contact( *it, it->phoneNumbers().at(i), displayPhoto ));
					k++;
				}
			}
		}
		if(k >= maxResults)
		{
			full = true;
		}
	}
	return results;
}


int SFLPhoneView::phoneNumberTypesDisplayed()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	MapStringInt addressBookSettings = configurationManager.getAddressbookSettings().value();
	int typesDisplayed = 0;
	if(addressBookSettings[ADDRESSBOOK_DISPLAY_BUSINESS])
	{
		typesDisplayed = typesDisplayed | PhoneNumber::Work;
	}
	if(addressBookSettings[ADDRESSBOOK_DISPLAY_MOBILE])
	{
		typesDisplayed = typesDisplayed | PhoneNumber::Cell;
	}
	if(addressBookSettings[ADDRESSBOOK_DISPLAY_HOME])
	{
		typesDisplayed = typesDisplayed | PhoneNumber::Home;
	}
	return typesDisplayed;
}

void SFLPhoneView::updateRecordButton()
{
	qDebug() << "updateRecordButton";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	double recVol = callManager.getVolume(RECORD_DEVICE);
	if(recVol == 0.00)
	{
		toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_0));
	}
	else if(recVol < 0.33)
	{
		toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_1));
	}
	else if(recVol < 0.67)
	{
		toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_2));
	}
	else
	{
		toolButton_recVol->setIcon(QIcon(ICON_REC_VOL_3));
	}
	if(recVol > 0)
	{	
		toolButton_recVol->setChecked(false);
	}
}
void SFLPhoneView::updateVolumeButton()
{
	qDebug() << "updateVolumeButton";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	double sndVol = callManager.getVolume(SOUND_DEVICE);
	if(sndVol == 0.00)
	{
		toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_0));
	}
	else if(sndVol < 0.33)
	{
		toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_1));
	}
	else if(sndVol < 0.67)
	{
		toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_2));
	}
	else
	{
		toolButton_sndVol->setIcon(QIcon(ICON_SND_VOL_3));
	}
	if(sndVol > 0)
	{
		toolButton_sndVol->setChecked(false);
	}
}


void SFLPhoneView::updateRecordBar()
{
	qDebug() << "updateRecordBar";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	double recVol = callManager.getVolume(RECORD_DEVICE);
	int value = (int)(recVol * 100);
	slider_recVol->setValue(value);
}
void SFLPhoneView::updateVolumeBar()
{
	qDebug() << "updateVolumeBar";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	double sndVol = callManager.getVolume(SOUND_DEVICE);
	int value = (int)(sndVol * 100);
	slider_sndVol->setValue(value);
}

void SFLPhoneView::updateVolumeControls()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	int display = 0;

	if(configurationManager.getAudioManager() == ConfigurationSkeleton::EnumInterface::ALSA)
	{
		display = configurationManager.getVolumeControls();

		((SFLPhone*)parent())->action_displayVolumeControls->setEnabled(true);
	}
	else
	{
		((SFLPhone*)parent())->action_displayVolumeControls->setEnabled(false);
	}
	      
	qDebug() << "updateVolumeControls " << display;
		
	((SFLPhone*)parent())->action_displayVolumeControls->setChecked(display);
	widget_recVol->setVisible(display);
	widget_sndVol->setVisible(display);
}

void SFLPhoneView::updateDialpad()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	int display = configurationManager.getDialpad();
	
	qDebug() << "updateDialpad " << display;

	widget_dialpad->setVisible(display);
}


void SFLPhoneView::updateStatusMessage()
{
	qDebug() << "updateStatusMessage";
	Account * account = accountInUse();

	if(account == NULL)
	{
		emit statusMessageChangeAsked(i18n("No registered accounts"));
	}
	else
	{
		emit statusMessageChangeAsked(i18n("Using account") 
					      + " \'" + account->getAlias() 
					      + "\' (" + account->getAccountDetail(ACCOUNT_TYPE) + ")") ;
	}
}



/************************************************************
************            Autoconnect             *************
************************************************************/

void SFLPhoneView::displayVolumeControls()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	int display = configurationManager.getVolumeControls();
	configurationManager.setVolumeControls(!display);
	updateVolumeControls();
}

void SFLPhoneView::displayDialpad()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	int display = configurationManager.getDialpad();

	configurationManager.setDialpad(!display);
	updateDialpad();
}


void SFLPhoneView::on_widget_dialpad_typed(QString text)      
{ 
	typeString(text); 
}


void SFLPhoneView::on_lineEdit_searchHistory_textChanged()
{
	qDebug() << "on_lineEdit_searchHistory_textChanged";
	updateSearchHistory();
	updateCallHistory();
	updateWindowCallState();
}

void SFLPhoneView::on_lineEdit_addressBook_textChanged()
{
	qDebug() << "on_lineEdit_addressBook_textChanged";
	updateAddressBook();
	updateWindowCallState();
}

void SFLPhoneView::on_slider_recVol_valueChanged(int value)
{
	qDebug() << "on_slider_recVol_valueChanged(" << value << ")";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	callManager.setVolume(RECORD_DEVICE, (double)value / 100.0);
	updateRecordButton();
}

void SFLPhoneView::on_slider_sndVol_valueChanged(int value)
{
	qDebug() << "on_slider_sndVol_valueChanged(" << value << ")";
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	callManager.setVolume(SOUND_DEVICE, (double)value / 100.0);
	updateVolumeButton();
}


void SFLPhoneView::on_toolButton_recVol_clicked(bool checked)
{
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	qDebug() << "on_toolButton_recVol_clicked().";
	if(!checked)
	{
		qDebug() << "checked";
		toolButton_recVol->setChecked(false);
		slider_recVol->setEnabled(true);
		callManager.setVolume(RECORD_DEVICE, (double)slider_recVol->value() / 100.0);
	}
	else
	{
		qDebug() << "unchecked";
		toolButton_recVol->setChecked(true);
		slider_recVol->setEnabled(false);
		callManager.setVolume(RECORD_DEVICE, 0.0);
	}
	updateRecordButton();
}


void SFLPhoneView::on_toolButton_sndVol_clicked(bool checked)
{
	CallManagerInterface & callManager = CallManagerInterfaceSingleton::getInstance();
	qDebug() << "on_toolButton_sndVol_clicked().";
	if(!checked)
	{
		qDebug() << "checked";
		toolButton_sndVol->setChecked(false);
		slider_sndVol->setEnabled(true);
		callManager.setVolume(SOUND_DEVICE, (double)slider_sndVol->value() / 100.0);
	}
	else
	{
		qDebug() << "unchecked";
		toolButton_sndVol->setChecked(true);
		slider_sndVol->setEnabled(false);
		callManager.setVolume(SOUND_DEVICE, 0.0);
	}
	updateVolumeButton();
}


void SFLPhoneView::on_listWidget_callList_currentItemChanged()
{
	qDebug() << "on_listWidget_callList_currentItemChanged";
	updateWindowCallState();
}

void SFLPhoneView::on_listWidget_callList_itemChanged()
{
	qDebug() << "on_listWidget_callList_itemChanged";
	stackedWidget_screen->setCurrentWidget(page_callList);
}

void SFLPhoneView::on_listWidget_callList_itemDoubleClicked(CallTreeItem * item)
{
	qDebug() << "on_listWidget_callList_itemDoubleClicked";
	Call * call = item->call();
	call_state state = call->getCurrentState();
	switch(state)
	{
		case CALL_STATE_HOLD:
			action(call, CALL_ACTION_HOLD);
			break;
		case CALL_STATE_DIALING:
			action(call, CALL_ACTION_ACCEPT);
			break;
		default:
			qDebug() << "Double clicked an item with no action on double click.";
	}
}

void SFLPhoneView::on_listWidget_callHistory_itemDoubleClicked(CallTreeItem * item)
{
	qDebug() << "on_listWidget_callHistory_itemDoubleClicked";
	changeScreen(SCREEN_MAIN);
	Call * pastCall = item->call();
	Call * call = callList->addDialingCall(pastCall->getPeerName(), pastCall->getAccountId());
	call->appendText(pastCall->getPeerPhoneNumber());
	addCallToCallList(call);
	callTree->setCurrentRow(callTree->count() - 1);
	action(call, CALL_ACTION_ACCEPT);
}


void SFLPhoneView::on_listWidget_addressBook_itemDoubleClicked(CallTreeItem * item)
{
	qDebug() << "on_listWidget_addressBook_itemDoubleClicked";
	changeScreen(SCREEN_MAIN);
// TOFIX
//	ContactItemWidget * w = (ContactItemWidget *) (listWidget_addressBook->itemWidget(item));
//	Call * call = callList->addDialingCall(w->getContactName());
//	call->appendItemText(w->getContactNumber());
//	addCallToCallList(call);
//	callTree->setCurrentRow(callTree->count() - 1);
//	action(call, CALL_ACTION_ACCEPT);
}

void SFLPhoneView::on_stackedWidget_screen_currentChanged(int index)
{
	qDebug() << "on_stackedWidget_screen_currentChanged";
	switch(index)
	{
		case SCREEN_MAIN:
			qDebug() << "Switched to call list screen.";
			emit windowTitleChangeAsked(i18n("SFLphone") + " - " + i18n("Main screen"));
			break;
		case SCREEN_HISTORY:
			qDebug() << "Switched to call history screen.";
			if(!historyLoaded)
			{
				updateCallHistory();
				historyLoaded = true;
			}
			alternateColors(listWidget_callHistory);
			emit windowTitleChangeAsked(i18n("SFLphone") + " - " + i18n("Call history"));
			break;
		case SCREEN_ADDRESS:
			qDebug() << "Switched to address book screen.";
			updateAddressBook();
			emit windowTitleChangeAsked(i18n("SFLphone") + " - " + i18n("Address book"));
			break;
		default:
			qDebug() << "Error : reached an unknown index \"" << index << "\" with stackedWidget_screen.";
			break;
	}
}

void SFLPhoneView::contextMenuEvent(QContextMenuEvent *event)
{
	KMenu menu(this);
	if( ( stackedWidget_screen->currentWidget() == page_callHistory && listWidget_callHistory->currentItem() ) || 
	    ( stackedWidget_screen->currentWidget() == page_addressBook && listWidget_addressBook->currentItem() ) )
	{
		KAction * action_edit = new KAction(&menu);
		action_edit->setText(i18n("Edit before call"));
		connect(action_edit, SIGNAL(triggered()),
		        this  , SLOT(editBeforeCall()));
		menu.addAction(action_edit);
	}
	SFLPhone * window = (SFLPhone * ) this->parent();
	QList<QAction *> callActions = window->getCallActions();
	menu.addAction(callActions.at((int) SFLPhone::Accept));
	menu.addAction(callActions[SFLPhone::Refuse]);
	menu.addAction(callActions[SFLPhone::Hold]);
	menu.addAction(callActions[SFLPhone::Transfer]);
	menu.addAction(callActions[SFLPhone::Record]);
	menu.addSeparator();
	
	QAction * action = new ActionSetAccountFirst(NULL, &menu);
	action->setChecked(priorAccountId.isEmpty());
	connect(action,  SIGNAL(setFirst(Account *)),
	        this  ,  SLOT(setAccountFirst(Account *)));
	menu.addAction(action);
	
	QVector<Account *> accounts = accountList->registeredAccounts();
	for (int i = 0 ; i < accounts.size() ; i++)
	{
		Account * account = accounts.at(i);
		QAction * action = new ActionSetAccountFirst(account, &menu);
		action->setChecked(account->getAccountId() == priorAccountId);
		connect(action, SIGNAL(setFirst(Account *)),
		        this  , SLOT(setAccountFirst(Account *)));
		menu.addAction(action);
	}
	menu.exec(event->globalPos());
}

void SFLPhoneView::editBeforeCall()
{
	qDebug() << "editBeforeCall";
	QString name;
	QString number;
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		CallTreeItem * item = historyTree->currentItem();
		if(item)
		{
			Call * call = item->call();
			if(call)
			{
				name = call->getPeerName();
				number = call->getPeerPhoneNumber();
			}
		}
	}
	else if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		QListWidgetItem * item = listWidget_addressBook->currentItem();
		if(item)
		{
			ContactItemWidget * w = (ContactItemWidget *) (listWidget_addressBook->itemWidget(listWidget_addressBook->currentItem()));
			name = w->getContactName();
			number = w->getContactNumber();
		}
	}
	else
	{	return;	}
	bool ok;
	QString newNumber = QInputDialog::getText(this, i18n("Edit before call"), QString(), QLineEdit::Normal, number, &ok);
	if(ok)
	{
		changeScreen(SCREEN_MAIN);
		Call * call = callList->addDialingCall(name);
		call->appendText(newNumber);
		addCallToCallList(call);
		callTree->setCurrentRow(callTree->count() - 1);
		action(call, CALL_ACTION_ACCEPT);
	}
}

void SFLPhoneView::setAccountFirst(Account * account)
{
	qDebug() << "setAccountFirst : " << (account ? account->getAlias() : QString());
	if(account)
	{
		priorAccountId = account->getAccountId();
	}
	else
	{
		priorAccountId = QString();
	}
	updateStatusMessage();
}

void SFLPhoneView::on_listWidget_callHistory_currentItemChanged()
{
	qDebug() << "on_listWidget_callHistory_currentItemChanged";
	updateWindowCallState();
}

void SFLPhoneView::on_listWidget_addressBook_currentItemChanged()
{
	qDebug() << "on_listWidget_addressBook_currentItemChanged";
	updateWindowCallState();
}


void SFLPhoneView::configureSflPhone()
{
	configDialog->reload();
	configDialog->show();
}

void SFLPhoneView::accountCreationWizard()
{
	wizard->show();
}
	

void SFLPhoneView::accept()
{
	if(stackedWidget_screen->currentWidget() == page_callList)
	{
		CallTreeItem * item = callTree->currentItem();
		if(!item)
		{
			qDebug() << "Calling when no item is selected. Opening an item.";
			Call * call = callList->addDialingCall();
			addCallToCallList(call);
			callTree->setCurrentRow(callTree->count() - 1);
		}
		else
		{
			Call * call = item->call();
			if(!call)
			{
				qDebug() << "Error : Accept triggered on unexisting call.";
			}
			else
			{
				int state = call->getState();
				if(state == CALL_STATE_RINGING || state == CALL_STATE_CURRENT || state == CALL_STATE_HOLD || state == CALL_STATE_BUSY)
				{
					qDebug() << "Calling when item currently ringing, current, hold or busy. Opening an item.";
					Call * call = callList->addDialingCall();
					addCallToCallList(call);
					callTree->setCurrentRow(callTree->count() - 1);
				}
				else
				{
					action(call, CALL_ACTION_ACCEPT);
				}
			}
		}
	}
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		changeScreen(SCREEN_MAIN);
		Call * pastCall = historyTree->currentItem()->call();
		Call * call = callList->addDialingCall(pastCall->getPeerName());
		call->appendText(pastCall->getPeerPhoneNumber());
		addCallToCallList(call);
		callTree->setCurrentRow(callTree->count() - 1);
		action(call, CALL_ACTION_ACCEPT);
	}
	if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		changeScreen(SCREEN_MAIN);
		ContactItemWidget * w = (ContactItemWidget *) (listWidget_addressBook->itemWidget(listWidget_addressBook->currentItem()));
		Call * call = callList->addDialingCall(w->getContactName());
		call->appendText(w->getContactNumber());
		addCallToCallList(call);
		callTree->setCurrentRow(callTree->count() - 1);
		action(call, CALL_ACTION_ACCEPT);
	}
}

void SFLPhoneView::refuse()
{
	if(stackedWidget_screen->currentWidget() == page_callList)
	{
		CallTreeItem * item = callTree->currentItem();
		if(!item)
		{
			qDebug() << "Error : Hanging up when no item selected. Should not happen.";
		}
		else
		{
			action(item->call(), CALL_ACTION_REFUSE);
		}
	}
	if(stackedWidget_screen->currentWidget() == page_callHistory)
	{
		lineEdit_searchHistory->clear();
	}
	if(stackedWidget_screen->currentWidget() == page_addressBook)
	{
		lineEdit_addressBook->clear();
	}
}

void SFLPhoneView::hold()
{
	CallTreeItem * item = callTree->currentItem();
	if(!item)
	{
		qDebug() << "Error : Holding when no item selected. Should not happen.";
	}
	else
	{
		action(item->call(), CALL_ACTION_HOLD);
	}
}

void SFLPhoneView::transfer()
{
	CallTreeItem * item = callTree->currentItem();
	if(!item)
	{
		qDebug() << "Error : Transfering when no item selected. Should not happen.";
	}
	else
	{
		action(item->call(), CALL_ACTION_TRANSFER);
	}
}

void SFLPhoneView::record()
{
	CallTreeItem * item = callTree->currentItem();
	if(!item)
	{
		qDebug() << "Error : Recording when no item selected. Should not happen.";
	}
	else
	{
		action(item->call(), CALL_ACTION_RECORD);
	}
}

void SFLPhoneView::mailBox()
{
	Account * account = accountInUse();
	QString mailBoxNumber = account->getAccountDetail(ACCOUNT_MAILBOX);
	Call * call = callList->addDialingCall();
	call->appendText(mailBoxNumber);
	addCallToCallList(call);
	callTree->setCurrentRow(callTree->count() - 1);
	action(call, CALL_ACTION_ACCEPT);
}

void SFLPhoneView::on1_callStateChanged(const QString &callID, const QString &state)
{
	qDebug() << "Signal : Call State Changed for call  " << callID << " . New state : " << state;
	Call * call = callList->findCallByCallId(callID);
	if(!call)
	{
		if(state == CALL_STATE_CHANGE_RINGING)
		{
			call = callList->addRingingCall(callID);
			addCallToCallList(call);
		}
		else
		{
			qDebug() << "Call doesn't exist in this client. Might have been initialized by another client instance before this one started.";
			return;
		}
	}
	else
	{
		call->stateChanged(state);
	}
	//	updateCallItem(call);
	updateWindowCallState();
}

void SFLPhoneView::on1_error(MapStringString details)
{
	qDebug() << "Signal : Daemon error : " << details;
}

void SFLPhoneView::on1_incomingCall(const QString & /*accountID*/, const QString & callID)
{
	qDebug() << "Signal : Incoming Call ! ID = " << callID;
	Call * call = callList->addIncomingCall(callID);
	addCallToCallList(call);
	callTree->setCurrentRow(callTree->count() - 1);

	changeScreen(SCREEN_MAIN);

	((SFLPhone*)parent())->activateWindow();
	((SFLPhone*)parent())->raise();
	((SFLPhone*)parent())->setVisible(true);

	emit incomingCall(call);
}

void SFLPhoneView::on1_incomingMessage(const QString &accountID, const QString &message)
{
	qDebug() << "Signal : Incoming Message for account " << accountID << " ! \nMessage : " << message;
}

void SFLPhoneView::on1_voiceMailNotify(const QString &accountID, int count)
{
	qDebug() << "Signal : VoiceMail Notify ! " << count << " new voice mails for account " << accountID;
}

void SFLPhoneView::on1_volumeChanged(const QString & /*device*/, double value)
{
	qDebug() << "Signal : Volume Changed !";
	if(! (toolButton_recVol->isChecked() && value == 0.0))
		updateRecordBar();
	if(! (toolButton_sndVol->isChecked() && value == 0.0))
		updateVolumeBar();
}

void SFLPhoneView::on1_audioManagerChanged()
{
	qDebug() << "Signal : Audio Manager Changed !";

	updateVolumeControls();
}

void SFLPhoneView::enableAddressBook()
{
	qDebug() << "\nenableAddressBook\n";
	lineEdit_addressBook->setClickMessage(QString());
	lineEdit_addressBook->setEnabled(true);
	AddressBook * ab = StdAddressBook::self(true);
	disconnect(ab,         SIGNAL(addressBookChanged(AddressBook *)),
	           this,       SLOT(enableAddressBook()));
}

bool SFLPhoneView::loadAddressBook()
{
	qDebug() << "loadAddressBook";
	AddressBook * ab = StdAddressBook::self(true);
	if(ab->loadingHasFinished())
	{
		return true;
	}
	else
	{
		connect(ab,         SIGNAL(addressBookChanged(AddressBook *)),
		        this,       SLOT(enableAddressBook()));
		return false;
	}
}


void SFLPhoneView::updateAddressBookEnabled()
{
	qDebug() << "updateAddressBookEnabled";
	bool enabled = isAddressBookEnabled();
	emit addressBookEnableAsked(enabled);
	if(! enabled && stackedWidget_screen->currentWidget() == page_addressBook)
	{
		changeScreen(SCREEN_MAIN);
	}
}


bool SFLPhoneView::isAddressBookEnabled()
{
	ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
	MapStringInt addressBookSettings = configurationManager.getAddressbookSettings().value();
	return addressBookSettings[ADDRESSBOOK_ENABLE];
}

void SFLPhoneView::changeScreen(int screen)
{
	qDebug() << "changeScreen";
	switch(screen)
	{
		case SCREEN_MAIN:
			stackedWidget_screen->setCurrentWidget(page_callList);
			break;
		case SCREEN_HISTORY:
			stackedWidget_screen->setCurrentWidget(page_callHistory);
			break;
		case SCREEN_ADDRESS:
			stackedWidget_screen->setCurrentWidget(page_addressBook);
			break;
		default:
			break;
	}
	updateWindowCallState();
	emit screenChanged(screen);
}

#include "SFLPhoneView.moc"
