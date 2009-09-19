/*
 *      keylist.h
 *
 *      Copyright 2008 gpg4usb-team <gpg4usb@cpunk.de>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef __KEYLIST_H__
#define __KEYLIST_H__

#include "context.h"

class QWidget;
class QVBoxLayout;
class QTableWidgetItem;
class QLabel;
class QMessageBox;
class QtGui;
class QTableWidget;
class QPushButton;
class QMenu;

class KeyList : public QWidget
{
    Q_OBJECT

public:
    void setColumnWidth(int row, int size);
    void addMenuAction(QAction *act);

    KeyList(GpgME::Context *ctx, QString iconpath, QWidget *parent = 0);
    QList<QString> *getChecked();
    QList<QString> *getSelected();

public slots:
    void refresh();

private:
    GpgME::Context *mCtx;
    QTableWidget *mKeyList;
    QString iconPath;
    QMenu *popupMenu;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // __KEYLIST_H__
