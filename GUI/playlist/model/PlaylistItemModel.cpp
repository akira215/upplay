/* Copyright (C) 2011  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GUI/playlist/model/PlaylistItemModel.h"
#include "HelperStructs/MetaData.h"

#include <QList>
#include <QDebug>


using namespace std;

PlaylistItemModel::PlaylistItemModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

PlaylistItemModel::~PlaylistItemModel()
{
}


int PlaylistItemModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return (int)(m_meta.size());
}

QVariant PlaylistItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= (int) m_meta.size() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return QVariant();
    }

    if (role == Qt::WhatsThisRole) {
        return m_meta[index.row()].toVariant();
    }

    else {
        return QVariant();
    }
}




Qt::ItemFlags PlaylistItemModel::flags(const QModelIndex& index =
                                       QModelIndex()) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool PlaylistItemModel::setData(const QModelIndex& index, const QVariant& value,
                                int role = Qt::EditRole)
{
    if (index.isValid() && role == Qt::EditRole) {
        MetaData md;
        if (MetaData::fromVariant(value, md)) {
            m_meta[index.row()] = md;
            emit dataChanged(index, index);
        }
        return true;
    }

    return false;
}


bool PlaylistItemModel::insertRows(int position, int rows,
                                   const QModelIndex& index)
{
    Q_UNUSED(index);

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    MetaDataList v_md_new;

    // copy old
    for (int i = 0; i < position; i++) {
        v_md_new.push_back(m_meta[i]);
    }

    // create new space
    for (int i = 0; i < rows; i++) {
        MetaData md;
        v_md_new.push_back(md);
    }

    // copy old
    for (uint i = (uint) position; i < m_meta.size(); i++) {
        v_md_new.push_back(m_meta[i]);
    }

    m_meta = v_md_new;

    endInsertRows();
    return true;
}

bool PlaylistItemModel::removeRows(int position, int rows,
                                   const QModelIndex& index)
{
    Q_UNUSED(index);
    if (rows < 1) {
        return true;
    }

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    MetaDataList v_md_new;

    for (uint i = 0; i < m_meta.size(); i++) {

        if (i >= (uint) position && i <= (uint)(position + rows - 1)) {
            continue;
        }

        v_md_new.push_back(m_meta[i]);
    }

    m_meta = v_md_new;

    endRemoveRows();
    return true;
}


void PlaylistItemModel::set_selected(QList<int>& rows)
{
    _selected_rows = rows;
    for (uint i = 0; i < m_meta.size(); i++) {
        m_meta[i].pl_selected = rows.contains(i);
    }
}

bool PlaylistItemModel::is_selected(int row) const
{
    return _selected_rows.contains(row);
}
