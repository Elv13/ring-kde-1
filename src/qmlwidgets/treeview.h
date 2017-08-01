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
#pragma once

#include <simpleflickable.h>

// Qt
#include <QtCore/QAbstractItemModel>
class QQmlComponent;

class TreeViewPrivate;

class TreeView : public SimpleFlickable
{
    Q_OBJECT
public:
    Q_PROPERTY(QSharedPointer<QAbstractItemModel> model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QQmlComponent* delegate READ delegate WRITE setDelegate)

    explicit TreeView(QQuickItem* parent = nullptr);
    virtual ~TreeView();

    void setModel(QSharedPointer<QAbstractItemModel> model);
    QSharedPointer<QAbstractItemModel> model() const;

    void setDelegate(QQmlComponent* delegate);
    QQmlComponent* delegate() const;

Q_SIGNALS:
    void modelChanged(QSharedPointer<QAbstractItemModel> model);

private:
    TreeViewPrivate* d_ptr;
    Q_DECLARE_PRIVATE(TreeView)
};
