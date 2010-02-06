/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERLISTMODEL_H
#define USERLISTMODEL_H

#include <QAbstractItemModel>
#include <QString>
#include <QPixmap>
#include <QList>
#include <QStringList>
#include <QRegExp>

#include <boost/pool/object_pool.hpp>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"

using namespace dcpp;

class WulforUtil;

namespace dcpp{
    inline uint qHash(const boost::intrusive_ptr<dcpp::User> &ptr){
        ulong key = (unsigned long)(void*)ptr.get();

        if (sizeof(ulong) > sizeof(uint)) {
            return uint((key >> (8 * sizeof(uint) - 1)) ^ key);
        } else {
            return uint(key);
        }
    }
}

#include <QHash>

#define COLUMN_NICK     0
#define COLUMN_SHARE    1
#define COLUMN_COMMENT  2
#define COLUMN_TAG      3
#define COLUMN_CONN     4
#define COLUMN_IP       5
#define COLUMN_EMAIL    6

typedef QMap<QString, QVariant> UserMap;

class UserListItem{

public:
    UserListItem(const QList<QVariant> &data, UserListItem *parent = 0);
    UserListItem(const UserListItem&);
    void operator=(const UserListItem&);
    virtual ~UserListItem();

    void appendChild(UserListItem *child);

    UserListItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    UserListItem *parent();
    void updateColumn(int, QVariant);

    QList<UserListItem*> childItems;

    bool isOp;
    bool isAway;
    QString speed;
    QString cid;
    QPixmap *px;
    bool px_loaded;
    UserPtr ptr;
private:

    QList<QVariant> itemData;
    UserListItem *parentItem;

};

class UserListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    UserListModel(QObject * parent = 0);
    virtual ~UserListModel();

    virtual int rowCount(const QModelIndex & index = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & index = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & parent) const;

    void clear();
    void removeUser(const UserPtr&);

    void addUser (const UserMap &map, const UserPtr&);
    void addUser (  const QString& nick,
                    const qlonglong share = 0,
                    const QString& comment = QString(),
                    const QString& tag = QString(),
                    const QString& conn = QString(),
                    const QString& ip = QString(),
                    const QString& email = QString(),

                    bool isOp = false,
                    bool isAway = false,
                    const QString& speed = QString(),
                    const QString& cid= QString(),
                    const UserPtr& = UserPtr()
                );

    UserListItem *itemForPtr(const UserPtr&);

    int getSortColumn() const {
        return sortColumn;
    }

    Qt::SortOrder getSortOrder() const {
        return sortOrder;
    }

    QString CIDforNick(const QString&);
    bool hasUser(const UserPtr &usr) { return users.contains(usr); }

    QStringList matchNicksContaining(const QString & part, bool stripTags = false) const;
    QStringList matchNicksStartingWith(const QString & part, bool stripTags = false) const;

    void repaint() { emit layoutChanged(); }

signals:
    void listUpdated();

private:
    UserListItem *rootItem;

    typedef QHash<UserPtr, UserListItem*> USRMap;
    typedef QHash<QString, UserPtr> NICKHash;

    USRMap users;
    NICKHash nicks;

    int sortColumn;
    Qt::SortOrder sortOrder;
    QRegExp stripper;

    boost::object_pool<UserListItem> pool;

    WulforUtil *WU;
};

#endif // USERLISTMODEL_H
