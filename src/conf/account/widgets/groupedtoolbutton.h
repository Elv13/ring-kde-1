// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef GROUPEDTOOLBUTTON_H
#define GROUPEDTOOLBUTTON_H

// Qt
#include <QtWidgets/QToolButton>
#include "typedefs.h"

/**
 * A thin tool button which can be grouped with another and look like one solid
 * bar:
 *
 * (button1 | button2)
 */
class LIB_EXPORT GroupedToolButton : public QToolButton
{
    Q_OBJECT
public:
    enum GroupPosition {
        NotGrouped  = 0,
        GroupLeft   = 1,
        GroupRight  = 2,
        GroupCenter = 3
    };

    explicit GroupedToolButton(QWidget* parent = nullptr);

    void setGroupPosition(GroupedToolButton::GroupPosition groupPosition);

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    GroupPosition mGroupPosition;
};

class LeftToolButton : public GroupedToolButton {
public:
   explicit LeftToolButton(QWidget* parent = nullptr) : GroupedToolButton(parent) {
      setGroupPosition(GroupPosition::GroupLeft);
   }
};
class RightToolButton : public GroupedToolButton {
public:
   explicit RightToolButton(QWidget* parent = nullptr) : GroupedToolButton(parent) {
      setGroupPosition(GroupPosition::GroupRight);
   }
};
class CenterToolButton : public GroupedToolButton {
public:
   explicit CenterToolButton(QWidget* parent = nullptr) : GroupedToolButton(parent) {
      setGroupPosition(GroupPosition::GroupCenter);
   }
};

#endif /* GROUPEDTOOLBUTTON_H */
