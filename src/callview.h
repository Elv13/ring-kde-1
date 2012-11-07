/***************************************************************************
 *   Copyright (C) 2009-2012 by Savoir-Faire Linux                         *
 *   Author : Emmanuel Lepage Valle <emmanuel.lepage@savoirfairelinux.com >*
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
#ifndef CALL_VIEW
#define CALL_VIEW

//Qt
#include <QtGui/QTreeWidget>
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtCore/QTimer>

//SFLPhone
#include "lib/callmodel.h"

//Qt
class QTreeWidgetItem;
class QPushButton;

//KDE
class KLineEdit;

//SFLPhone
class CallTreeItem;
class CallTreeItemDelegate;
class CallViewOverlayToolbar;
class TipManager;
class Tip;

//Typedef
typedef CallModel<CallTreeItem*,QTreeWidgetItem*> TreeWidgetCallModel;

///CallViewOverlay: Display overlay on top of the call tree
class CallViewOverlay : public QWidget {
   Q_OBJECT

public:
   //Constructor
   CallViewOverlay(QWidget* parent);
   ~CallViewOverlay();

   //Setters
   void setCornerWidget(QWidget* wdg);
   void setVisible(bool enabled);
   void setAccessMessage(QString message);

protected:
   virtual void paintEvent  (QPaintEvent*  event );
   virtual void resizeEvent (QResizeEvent* e     );

private:
   QWidget* m_pIcon        ;
   uint     m_step         ;
   QTimer*  m_pTimer       ;
   bool     m_enabled      ;
   QColor   m_black        ;
   QString  m_accessMessage;

private slots:
   void changeVisibility();
};

///CallView: Central tree widget managing active calls
class CallView : public QTreeWidget {
   Q_OBJECT
   friend class CallTreeItemDelegate;

   public:
      CallView                    ( QWidget* parent = 0                                                               );
      ~CallView                   (                                                                                   );

      //Getters
      Call* getCurrentItem        (                                                                                   );
      QWidget* getWidget          (                                                                                   );
      bool haveOverlay            (                                                                                   );
      virtual QMimeData* mimeData ( const QList<QTreeWidgetItem *> items                                              ) const;
      CallViewOverlayToolbar*     overlayToolbar() { return m_pCanvasToolbar;                                         };

      //Setters
      void setTitle               ( const QString& title                                                              );

      //Mutator
      bool removeItem             ( Call* item                                                                        );
      bool dropMimeData           ( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action  );
      bool callToCall             ( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action  );
      bool phoneNumberToCall      ( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action  );
      bool contactToCall          ( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action  );
      void moveSelectedItem       ( Qt::Key direction                                                                 );
      void selectFirstItem        (                                                                                   );

   private:
      //Mutator
      QTreeWidgetItem* extractItem ( const QString& callId                             );
      QTreeWidgetItem* extractItem ( QTreeWidgetItem* item                             );
      CallTreeItem* insertItem     ( QTreeWidgetItem* item, QTreeWidgetItem* parent=0  );
      CallTreeItem* insertItem     ( QTreeWidgetItem* item, Call* parent               );
      void clearArtefact           (                                                   );

      //Attributes
      QPushButton*            m_pTransferB          ;
      KLineEdit*              m_pTransferLE         ;
      CallViewOverlay*        m_pTransferOverlay    ;
      CallViewOverlay*        m_pActiveOverlay      ;
      Call*                   m_pCallPendingTransfer;
      CallViewOverlayToolbar* m_pCanvasToolbar      ;
      TipManager*             m_pTip                ;
      Tip*                    m_pTip2               ;
      Tip*                    m_pTip3               ;

   protected:
      //Reimlementation
      virtual void dragEnterEvent ( QDragEnterEvent *e );
      virtual void dragMoveEvent  ( QDragMoveEvent  *e );
      virtual void dragLeaveEvent ( QDragLeaveEvent *e );
      virtual void resizeEvent    ( QResizeEvent    *e );
      virtual void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const;

   public slots:
      void destroyCall        ( Call* toDestroy);
      void itemDoubleClicked  ( QTreeWidgetItem* item, int column    );
      void itemClicked        ( QTreeWidgetItem* item, int column =0 );
      Call* addCall           ( Call* call, Call* parent =0          );
      Call* addConference     ( Call* conf                           );
      bool conferenceChanged  ( Call* conf                           );
      void conferenceRemoved  ( Call* conf                           );

      virtual void keyPressEvent(QKeyEvent* event);

   public slots:
      void clearHistory();
      void showTransferOverlay(Call* call);
      void transfer();
      void transferDropEvent(Call* call,QMimeData* data);
      void conversationDropEvent(Call* call,QMimeData* data);
      void hideOverlay();
      void moveCanvasTip();

   signals:
      ///Emitted when one of the call item change
      void itemChanged(Call*);

};
#endif
