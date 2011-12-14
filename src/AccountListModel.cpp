/***************************************************************************
 *   Copyright (C) 2009-2012 by Savoir-Faire Linux                         *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>         *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>*
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
#include "AccountListModel.h"

//SFLPhone library
#include "lib/sflphone_const.h"

//SFLPhone
#include "conf/ConfigAccountList.h"

///Constructor
AccountListModel::AccountListModel(QObject *parent)
 : QAbstractListModel(parent)
{
   this->accounts = new ConfigAccountList();
}

///Destructor
AccountListModel::~AccountListModel()
{
}


/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

///Get data from the model
QVariant AccountListModel::data ( const QModelIndex& index, int role) const
{
   if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
      return QVariant();

   const Account * account = (*accounts)[index.row()];
   if(index.column() == 0 && role == Qt::DisplayRole)
      return QVariant(account->getAlias());
   else if(index.column() == 0 && role == Qt::CheckStateRole)
      return QVariant(account->isEnabled() ? Qt::Checked : Qt::Unchecked);
   else if(index.column() == 0 && role == Qt::DecorationRole) {
      if(! account->isEnabled())
         return QVariant(QIcon(ICON_ACCOUNT_LED_GRAY));
      else if(account->isRegistered())
         return QVariant(QIcon(ICON_ACCOUNT_LED_GREEN));
      else
         return QVariant(QIcon(ICON_ACCOUNT_LED_RED));
   }
   return QVariant();
}

///Flags for "index"
Qt::ItemFlags AccountListModel::flags(const QModelIndex & index) const
{
   if (index.column() == 0)
      return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
   return QAbstractItemModel::flags(index);
}

///Get the account list
QString AccountListModel::getOrderedList() const
{
   return accounts->getOrderedList();
}


/*****************************************************************************
 *                                                                           *
 *                                  Setters                                  *
 *                                                                           *
 ****************************************************************************/

///Set model data
bool AccountListModel::setData(const QModelIndex & index, const QVariant &value, int role)
{
   if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole) {
      (*accounts)[index.row()]->setEnabled(value.toBool());
      emit dataChanged(index, index);
      return true;
   }
   return false;
}


/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

///Move account up
bool AccountListModel::accountUp( int index )
{
   if(index > 0 && index <= rowCount()) {
      accounts->upAccount(index);
      emit dataChanged(this->index(index - 1, 0, QModelIndex()), this->index(index, 0, QModelIndex()));
      return true;
   }
   return false;
}

///Move account down
bool AccountListModel::accountDown( int index )
{
   if(index >= 0 && index < rowCount()) {
      accounts->downAccount(index);
      emit dataChanged(this->index(index, 0, QModelIndex()), this->index(index + 1, 0, QModelIndex()));
      return true;
   }
   return false;
}

///Remove an account
bool AccountListModel::removeAccount( int index )
{
   if(index >= 0 && index < rowCount()) {
      accounts->removeAccount(accounts->getAccountAt(index));
      emit dataChanged(this->index(index, 0, QModelIndex()), this->index(rowCount(), 0, QModelIndex()));
      return true;
   }
   return false;
}

///Add an account
bool AccountListModel::addAccount(const QString& alias )
{
   accounts->addAccount(alias);
   emit dataChanged(this->index(0, 0, QModelIndex()), this->index(rowCount(), 0, QModelIndex()));
   return true;
}

///Number of account
int AccountListModel::rowCount(const QModelIndex & /*parent*/) const
{
   return accounts->size();
}