/************************************************************************************
 *   Copyright (C) 2009 by Savoir-Faire Linux                                       *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>                  *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/

//Parent
#include "AccountList.h"

//SFLPhone
#include "sflphone_const.h"

//SFLPhone library
#include "configurationmanager_interface_singleton.h"

AccountList* AccountList::m_spAccountList   = nullptr;
QString      AccountList::m_sPriorAccountId = ""     ;

///Constructors
AccountList::AccountList(QStringList & _accountIds)
{
   m_pAccounts = new QVector<Account*>();
   for (int i = 0; i < _accountIds.size(); ++i) {
      (*m_pAccounts) += Account::buildExistingAccountFromId(_accountIds[i]);
   }
}

///Constructors
///@param fill Whether to fill the list with accounts from configurationManager or not.
AccountList::AccountList(bool fill)
{
   m_pAccounts = new QVector<Account *>();
   if(fill)
      updateAccounts();
}

///Destructor
AccountList::~AccountList()
{
   foreach(Account* a,*m_pAccounts) {
      delete a;
   }
   delete m_pAccounts;
}

///Singleton
AccountList* AccountList::getInstance()
{
   if (not m_spAccountList) {
      m_spAccountList = new AccountList(true);
   }
   return m_spAccountList;
}

///Static destructor
void AccountList::destroy()
{
   if (m_spAccountList)
      delete m_spAccountList;
   m_spAccountList = nullptr;
}


/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

///Update accounts
void AccountList::update()
{
   ConfigurationManagerInterface & configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   Account * current;
   for (int i = 0; i < m_pAccounts->size(); i++) {
      current = (*m_pAccounts)[i];
      if (!(*m_pAccounts)[i]->isNew())
         removeAccount(current);
   }
   //ask for the list of accounts ids to the configurationManager
   QStringList accountIds = configurationManager.getAccountList().value();
   for (int i = 0; i < accountIds.size(); ++i) {
      m_pAccounts->insert(i, Account::buildExistingAccountFromId(accountIds[i]));
   }
} //update

///Update accounts
void AccountList::updateAccounts()
{
   qDebug() << "updateAccounts";
   ConfigurationManagerInterface& configurationManager = ConfigurationManagerInterfaceSingleton::getInstance();
   QStringList accountIds = configurationManager.getAccountList().value();
   m_pAccounts->clear();
   for (int i = 0; i < accountIds.size(); ++i) {
      qDebug() << "updateAccounts " << accountIds[i];
      (*m_pAccounts) += Account::buildExistingAccountFromId(accountIds[i]);
   }
   emit accountListUpdated();
} //updateAccounts


/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

///Get all accounts
const QVector<Account*>& AccountList::getAccounts()
{
   return *m_pAccounts;
}

///Get a single account
const Account* AccountList::getAccountAt (int i) const
{
   return (*m_pAccounts)[i];
}

///Get a single account
Account* AccountList::getAccountAt (int i)
{
   return (*m_pAccounts)[i];
}

///Get a serialized string of all accounts
QString AccountList::getOrderedList() const
{
   QString order;
   for( int i = 0 ; i < size() ; i++) {
      order += getAccountAt(i)->getAccountId() + "/";
   }
   return order;
}

///Get account using its ID
Account* AccountList::getAccountById(const QString & id) const
{
   if(id.isEmpty())
          return NULL;
   for (int i = 0; i < m_pAccounts->size(); ++i) {
      if (!(*m_pAccounts)[i]->isNew() && (*m_pAccounts)[i]->getAccountId() == id)
         return (*m_pAccounts)[i];
   }
   return NULL;
}

///Get account with a specific state
QVector<Account*> AccountList::getAccountsByState(const QString& state)
{
   QVector<Account *> v;
   for (int i = 0; i < m_pAccounts->size(); ++i) {
      if ((*m_pAccounts)[i]->getAccountRegistrationStatus() == state)
         v += (*m_pAccounts)[i];
   }
   return v;
}

///Get a list of all registerred account
QVector<Account*> AccountList::registeredAccounts() const
{
   qDebug() << "registeredAccounts";
   QVector<Account*> registeredAccounts;
   Account* current;
   for (int i = 0; i < m_pAccounts->count(); ++i) {
      current = (*m_pAccounts)[i];
      if(current->getAccountRegistrationStatus() == ACCOUNT_STATE_REGISTERED) {
         qDebug() << current->getAlias() << " : " << current;
         registeredAccounts.append(current);
      }
   }
   return registeredAccounts;
}

///Get the first registerred account (default account)
Account* AccountList::firstRegisteredAccount() const
{
   Account* current;
   for (int i = 0; i < m_pAccounts->count(); ++i) {
      current = (*m_pAccounts)[i];
      if(current && current->getAccountRegistrationStatus() == ACCOUNT_STATE_REGISTERED)
         return current;
      else if (current && (current->getAccountRegistrationStatus() == ACCOUNT_STATE_READY) && m_pAccounts->count() == 1)
         return current;
      else if (current && !(current->getAccountRegistrationStatus() == ACCOUNT_STATE_READY)) {
         qDebug() << "Account " << ((current)?current->getAccountId():"") << " is not registered ("
         << ((current)?current->getAccountRegistrationStatus():"") << ") State:"
         << ((current)?current->getAccountRegistrationStatus():"");
      }
   }
   return NULL;
}

///Get the account size
int AccountList::size() const
{
   return m_pAccounts->size();
}

///Return the current account
Account* AccountList::getCurrentAccount()
{
   Account* priorAccount = AccountList::getInstance()->getAccountById(m_sPriorAccountId);
   if(priorAccount && priorAccount->getAccountDetail(ACCOUNT_REGISTRATION_STATUS) == ACCOUNT_STATE_REGISTERED ) {
      return priorAccount;
   }
   else {
      return AccountList::getInstance()->firstRegisteredAccount();
   }
} //getCurrentAccount

///Return the previously used account ID
QString AccountList::getPriorAccoundId()
{
   return m_sPriorAccountId;
}


/*****************************************************************************
 *                                                                           *
 *                                  Setters                                  *
 *                                                                           *
 ****************************************************************************/

///Add an account
Account* AccountList::addAccount(QString & alias)
{
   Account* a = Account::buildNewAccountFromAlias(alias);
   (*m_pAccounts) += a;
   return a;
}

///Remove an account
void AccountList::removeAccount(Account* account)
{
   qDebug() << "Removing" << m_pAccounts;
   m_pAccounts->remove(m_pAccounts->indexOf(account));
}

///Set the previous account used
void AccountList::setPriorAccountId(const QString& value) {
   m_sPriorAccountId = value;
}


/*****************************************************************************
 *                                                                           *
 *                                 Operator                                  *
 *                                                                           *
 ****************************************************************************/

///Get the account from its index
const Account* AccountList::operator[] (int i) const
{
   return (*m_pAccounts)[i];
}

///Get the account from its index
Account* AccountList::operator[] (int i)
{
   return (*m_pAccounts)[i];
}
