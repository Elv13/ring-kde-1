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
#ifndef DIALPADTIP_H
#define DIALPADTIP_H
//Base
#include <tip/tip.h>

//Qt
#include <QtWidgets/QWidget>


//Qt
class QPainter;

///A tip to be passed to the TipLoader
class DialPadTip : public Tip
{
   Q_OBJECT
public:
   explicit DialPadTip(QWidget* parent = nullptr);
   virtual ~DialPadTip();

protected:
   virtual QRect getDecorationRect() override;
   virtual void  paintDecorations(QPainter& p, const QRect& textRect) override;
};

#endif
