/***************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                              *
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
#ifndef SECURITYPROGRESS_H
#define SECURITYPROGRESS_H

#include <QtWidgets/QProgressBar>

//Qt
class QTableView;
class QLabel;
class QHBoxLayout;
class QLineEdit;

//Ring
#include <securityevaluationmodel.h>
class SecurityProgress;
class SecurityFlaw;

class IssuesIcon : public QWidget {
   Q_OBJECT
public:
   explicit IssuesIcon(QWidget* parent = nullptr);
   ~IssuesIcon();

   //Setter
   void setBuddy(QWidget* buddy);
   void setModel(SecurityEvaluationModel* model);

   //Getter
   QWidget* buddy() const;

   //Mutator
   void addFlaw(const SecurityFlaw* flaw);
   void setupForLineEdit(QLineEdit* le);
   void reset();
protected:
   virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
   QHBoxLayout* m_pLayout;
   QWidget*     m_pBuddy;
   SecurityEvaluationModel* m_pModel;
private Q_SLOTS:
   void slotSolved();
   void slotFlawClicked();
Q_SIGNALS:
   void selectFlaw(const QModelIndex idx);
};

class SecurityLevelWidget : public QWidget
{
   Q_OBJECT
public:
   explicit SecurityLevelWidget(QWidget* parent = nullptr);
   virtual ~SecurityLevelWidget();

   //Getter
   QTableView* view() const;

   //Setter
   void setModel(SecurityEvaluationModel* model);
protected:
   //Virtual events
   virtual void mouseReleaseEvent ( QMouseEvent * event ) override;
private:
   //Attributes
   SecurityProgress* m_pLevel;
   QTableView* m_pView;
   SecurityEvaluationModel* m_pModel;

   //Widgets
   QLabel* m_pInfoL   ;
   QLabel* m_pWarningL;
   QLabel* m_pIssueL  ;
   QLabel* m_pErrorL  ;

private Q_SLOTS:
   void reloadCount();
   void dblClicked(const QModelIndex& idx);
};

#endif
