/***************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                         *
 *   Copyright (C) 2015 by Emmanuel Lepage Vallee                          *
 *   Author : Emmanuel Lepage Vallee <elv1313@gmail.com>                   *
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
#include "dockbase.h"

//Qt
#include <QtCore/QMimeData>
#include <QtCore/QSortFilterProxyModel>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMenu>

//Ring
#include "mime.h"
#include "menumodelview.h"
#include "klib/kcfg_settings.h"

///KeyPressEaterC: keygrabber
class KeyPressEaterC : public QObject
{
   Q_OBJECT
public:
   explicit KeyPressEaterC(DockBase* parent) : QObject(parent), m_pDock(parent)
   {}

protected:
   bool eventFilter(QObject *obj, QEvent *event) override {
      if (event->type() == QEvent::KeyPress) {
      QKeyEvent* e = (QKeyEvent*)event;
      if (e->key() != Qt::Key_Up && e->key() != Qt::Key_Down) {
         m_pDock->keyPressEvent(e);
         return true;
      }
   }
   // standard event processing
   return QObject::eventFilter(obj, event);
   }

private:
   DockBase* m_pDock;
};

///Constructor
DockBase::DockBase(QWidget* parent) : QDockWidget(parent),m_pMenu(nullptr)
{
   QWidget* mainWidget = new QWidget(this);
   setupUi(mainWidget);

   setWidget(mainWidget);
   m_pMenuBtn  ->setHidden(true);
   m_pKeyPressEater = new KeyPressEaterC( this );

   m_pView->installEventFilter(m_pKeyPressEater    );
   m_pView->setSortingEnabled (true                );
   m_pView->sortByColumn      (0,Qt::AscendingOrder);

   connect(m_pView     ,SIGNAL(contextMenuRequest(QModelIndex)), this , SLOT(slotContextMenu(QModelIndex)));
   connect(m_pFilterLE ,SIGNAL(textChanged(QString))           , this , SLOT(expandTree())                );
   connect(m_pView     ,SIGNAL(doubleClicked(QModelIndex))     , this , SLOT(slotDoubleClick(QModelIndex)));

   expandTree();
} //DockBase

///Destructor
DockBase::~DockBase()
{
   if (m_pMenu)
      delete m_pMenu;

   delete m_pKeyPressEater;
}

void DockBase::setProxyModel(QSortFilterProxyModel* proxy)
{
   m_pView->setModel(proxy);
   if (proxy) {
      connect(m_pFilterLE ,SIGNAL(filterStringChanged(QString))     , proxy, SLOT(setFilterRegExp(QString))   );
      connect(proxy       ,SIGNAL(layoutChanged())                  , this , SLOT(expandTree())               );
      connect(proxy       ,SIGNAL(rowsInserted(QModelIndex,int,int)), this , SLOT(expandTreeRows(QModelIndex)));
   }
}

void DockBase::setDelegate(QStyledItemDelegate* delegate)
{
   m_pView->setDelegate(delegate);
}

CategorizedTreeView* DockBase::view() const
{
   return m_pView;
}

void DockBase::setSortingModel(QAbstractItemModel* m, QItemSelectionModel* s)
{
   m_pMenuBtn->setHidden(false);
   m_pMenuBtn->setModel(m, s);
}

void DockBase::setMenuConstructor(std::function<QMenu*()> cst)
{
   m_fMenuConstructor = cst;
}

QMenu* DockBase::menu() const
{
   if (!m_pMenu)
      m_pMenu = m_fMenuConstructor();

   return m_pMenu;
}

///Called when someone right click on the 'index'
void DockBase::slotContextMenu(QModelIndex index)
{
   if (!index.parent().isValid())
      return;

   menu();
   //TODO setIndex
   menu()->exec(QCursor::pos());
}

void DockBase::slotDoubleClick(const QModelIndex& index)
{
   Q_UNUSED(index)
   //TODO add an execute(QModelIndex) variant to the UAM, use it for callagain
}

///Called when a call is dropped on transfer
void DockBase::transferEvent(QMimeData* data)
{
   Q_UNUSED(data)
   //TODO add an execute(QModelIndex) variant to the UAM, use it for transfer
//    if (data->hasFormat( RingMimes::CALLID)) {
//       bool ok = false;
// 
//       if (!m_pMenu)
//          m_pMenu = new Menu::Person(this);
// 
//       const ::ContactMethod* result = m_pMenu->showNumberSelector(ok);
// 
//       if (ok && result) {
//          Call* call = CallModel::instance().fromMime(data->data(RingMimes::CALLID));
//          if (dynamic_cast<Call*>(call)) {
//             CallModel::instance().transfer(call, result);
//          }
//       }
//    }
//    else
//       qDebug() << "Invalid mime data";
}

///Handle keypresses ont the dock
void DockBase::keyPressEvent(QKeyEvent* event) {
   int key = event->key();
   if(key == Qt::Key_Escape)
      m_pFilterLE->setText(QString());
   else if(key == Qt::Key_Return || key == Qt::Key_Enter) {

   }
   else if((key == Qt::Key_Backspace) && (m_pFilterLE->text().size()))
      m_pFilterLE->setText(m_pFilterLE->text().left( m_pFilterLE->text().size()-1 ));
   else if (!event->text().isEmpty() && !(key == Qt::Key_Backspace))
      m_pFilterLE->setText(m_pFilterLE->text()+event->text());
} //keyPressEvent

///Expand the tree according to the user preferences
void DockBase::expandTree()
{
   m_pView->expandToDepth( 2 );
}

void DockBase::expandTreeRows(const QModelIndex& idx)
{
   if (!idx.isValid()) //Only top level
      m_pView->expandToDepth( 2 );
}

#include <dockbase.moc>
