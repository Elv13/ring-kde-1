/***************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                              *
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
#ifndef WIZARD_H
#define WIZARD_H

#include <QtWidgets/QWidget>

class QHBoxLayout;
class QLineEdit;
class Account;

class Wizard : public QWidget
{
   Q_OBJECT
public:
   explicit Wizard(QWidget* parent = nullptr);
   virtual ~Wizard();

protected:
   virtual void paintEvent  (QPaintEvent*  event ) override;

private:
   QWidget*     m_pCurrentPage;
   QHBoxLayout* m_pLayout     ;
   QLineEdit*   m_pName       ;
   Account*     m_pAccount    ;

   //Event filter
   bool eventFilter( QObject *obj, QEvent *event) override;

private Q_SLOTS:
   void slotNext();
   void slotEmail();
   void slotPrint();
   void slotCopy();
   void slotConfigure();
   void slotComplete();
};

#endif
