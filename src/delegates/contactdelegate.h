/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#ifndef CONTACTDELEGATE_H
#define CONTACTDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QProxyStyle>

class DelegateDropOverlay;
class ContactMethodDelegate;
class CategorizedTreeView          ;

class ContactDelegate : public QStyledItemDelegate
{
public:
   explicit ContactDelegate(CategorizedTreeView* parent);

   QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   void setChildDelegate(ContactMethodDelegate* child);

private:
   DelegateDropOverlay* m_pDelegatedropoverlay;
   ContactMethodDelegate* m_pChildDelegate      ;
   CategorizedTreeView* m_pView               ;
};

#endif
