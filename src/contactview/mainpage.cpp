/***************************************************************************
 *   Copyright (C) 2017 by Bluesystems                                     *
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
#include "mainpage.h"

#include <QQmlApplicationEngine>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QQmlEngine>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QIcon>
#include <QtCore/QMimeData>
#include <QtCore/QTimer>
#include <QtCore/QSortFilterProxyModel>
#include <QQmlContext>

#include <../ringapplication.h>
#include "peerstimelinemodel.h"
#include "phonedirectorymodel.h"
#include <contactmethod.h>
#include <individual.h>
#include <person.h>
#include <call.h>
#include <callmodel.h>

// Remove inactive calls from the CallModel
class ActiveCallProxy2 : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ActiveCallProxy2(QObject* parent) : QSortFilterProxyModel(parent) {}

protected:
    virtual bool filterAcceptsRow(int row, const QModelIndex & srcParent ) const override;
};

class MainPagePrivate : public QObject {
    Q_OBJECT
public:
    QSharedPointer<QAbstractItemModel> m_CallsModel;
    QSharedPointer<QAbstractItemModel> m_Invididual;
    QSharedPointer<QAbstractItemModel> m_TimelineModel;
    QSharedPointer<QAbstractItemModel> m_DeduplicatedTimelineModel;
    QQuickItem* m_pItem {nullptr};
    QQuickItem* m_pHeader {nullptr};

    QList< QTimer* > m_lTimers;

    MainPage* q_ptr;

public Q_SLOTS:
    void slotWindowChanged();
};

MainPage::MainPage(QQuickItem* parent) :
    QQuickItem(parent), d_ptr(new MainPagePrivate)
{
    d_ptr->q_ptr = this;
    auto cp = new ActiveCallProxy2(&CallModel::instance());
    cp->setSourceModel(&CallModel::instance());

    QFile file(QStringLiteral(":/assets/welcome.html"));
    if (file.open(QIODevice::ReadOnly))
        RingApplication::engine()->rootContext()->setContextProperty(QStringLiteral("welcomeMessage"), file.readAll());

    RingApplication::engine()->rootContext()->setContextProperty(QStringLiteral("ActiveCallProxyModel"), cp);

    connect(this, &QQuickItem::windowChanged, d_ptr, &MainPagePrivate::slotWindowChanged);
    connect(&CallModel::instance(), &CallModel::callAttentionRequest, this, &MainPage::showVideo);

    installEventFilter(this);

    QTimer::singleShot(500, [this]() {
        setContactMethod(qvariant_cast<ContactMethod*>(
            PeersTimelineModel::instance().deduplicatedTimelineModel()
                ->index(0, 0).data((int)Ring::Role::Object)
        ));
    });

    d_ptr->m_DeduplicatedTimelineModel = PeersTimelineModel::instance().deduplicatedTimelineModel();
}

MainPage::~MainPage()
{
    d_ptr->m_TimelineModel.clear();
    d_ptr->m_Invididual.clear();
    d_ptr->m_CallsModel.clear();
    d_ptr->m_DeduplicatedTimelineModel.clear();

    // Release the shared pointer reference from the timer.
    for (auto t : qAsConst(d_ptr->m_lTimers)) {
        disconnect(t);
        delete t;
    }

    delete d_ptr;
}

bool MainPage::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    Q_UNUSED(event)
    //TODO the context menu
    return false;
}

void MainPage::setContactMethod(ContactMethod* cm)
{
    if ((!cm) || (!d_ptr->m_pItem))
        return;

    if (cm->type() == ContactMethod::Type::TEMPORARY)
        cm = PhoneDirectoryModel::instance().fromTemporary(
            static_cast<TemporaryContactMethod*>(cm)
        );

    // Keep a reference for 5 minutes to avoid double free from QML
    for (auto ptr : {d_ptr->m_Invididual, d_ptr->m_CallsModel, d_ptr->m_TimelineModel})
        if (ptr) {
            auto t = new QTimer(this);
            t->setInterval(5 * 60 * 1000);
            t->setSingleShot(true);
            connect(t, &QTimer::timeout, this, [t, this, ptr]() {
                this->d_ptr->m_lTimers.removeAll(t);
            });
            t->start();
            d_ptr->m_lTimers << t;
        }


    // Keep a strong reference because QML wont
    d_ptr->m_Invididual = cm->individual();
    d_ptr->m_CallsModel = cm->callsModel();

    d_ptr->m_TimelineModel = cm->individual()->timelineModel();

    d_ptr->m_pItem->setProperty( "currentContactMethod", QVariant::fromValue(cm));
    d_ptr->m_pItem->setProperty( "currentIndividual", QVariant::fromValue(d_ptr->m_Invididual));
    d_ptr->m_pItem->setProperty( "timelineModel", QVariant::fromValue(d_ptr->m_TimelineModel));
}

void MainPage::setPerson(Person* p)
{
    d_ptr->m_pItem->setProperty( "currentPerson", QVariant::fromValue(p));
    d_ptr->m_pItem->setProperty( "currentIndividual", QVariant::fromValue(p->individual()));
}

void MainPage::setCurrentPage(MainPage::Pages page)
{
    QString name;
    switch (page) {
        case MainPage::Pages::INFORMATION:
            name = QStringLiteral("INFORMATION");
            break;
        case MainPage::Pages::TIMELINE:
            name = QStringLiteral("TIMELINE");
            break;
        case MainPage::Pages::CALL_HISTORY:
            name = QStringLiteral("CALL_HISTORY");
            break;
        case MainPage::Pages::RECORDINGS:
            name = QStringLiteral("RECORDINGS");
            break;
        case MainPage::Pages::SEARCH:
            name = QStringLiteral("SEARCH");
            break;
        case MainPage::Pages::MEDIA:
            name = QStringLiteral("MEDIA");
            break;
    }

    d_ptr->m_pItem->setProperty( "currentPage", name);
}

bool ActiveCallProxy2::filterAcceptsRow(int row, const QModelIndex& srcParent ) const
{
    // Conferences are always active
    if (srcParent.isValid())
        return true;

    return sourceModel()->index(row, 0)
        .data((int)Call::Role::LifeCycleState) == QVariant::fromValue(Call::LifeCycleState::PROGRESS);
}

void MainPagePrivate::slotWindowChanged()
{
    auto ctx = QQmlEngine::contextForObject(q_ptr);
    Q_ASSERT(ctx);

    QQmlComponent comp(ctx->engine(), QStringLiteral("qrc:/ViewContact.qml"), q_ptr);

    m_pItem = qobject_cast<QQuickItem*>(comp.create(ctx));

    if (!m_pItem) {
        qDebug() << "Previous error" << comp.errorString();
    }

    Q_ASSERT(m_pItem);
    m_pItem->setParentItem(q_ptr);

    auto anchors = qvariant_cast<QObject*>(m_pItem->property("anchors"));
    anchors->setProperty("fill", QVariant::fromValue(q_ptr));
}

void MainPage::showVideo(Call* c)
{
    setContactMethod(c->peerContactMethod());
    QMetaObject::invokeMethod(d_ptr->m_pItem, "showVideo");
}

QQuickItem* MainPage::page() const
{
    return d_ptr->m_pItem;
}


QQuickItem* MainPage::header() const
{
    return d_ptr->m_pHeader;
}

void MainPage::setHeader(QQuickItem* item)
{
    d_ptr->m_pItem->setProperty("contactHeader", QVariant::fromValue(item));
    d_ptr->m_pHeader = item;
}

bool MainPage::isMobile() const
{
    return d_ptr->m_pItem->property("mobile").toBool();
}

void MainPage::setMobile(bool v)
{
    d_ptr->m_pItem->setProperty("mobile", v);
}

#include <mainpage.moc>
// kate: space-indent on; indent-width 4; replace-tabs on;
