/* Copyright (C) 2013  Lucio Carreras
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
#include "ContextMenu.h"
#include "HelperStructs/Helper.h"

#include <QAction>
#include <QList>
#include <QDebug>

ContextMenu::ContextMenu(QWidget* parent) :
    QMenu(parent)
{

    _info_action = new QAction(QIcon(Helper::getIconPath("info_small.png")),
                               tr("Info"), this);
    _edit_action = new QAction(QIcon(Helper::getIconPath("edit.png")),
                               tr("Edit"), this);
    _remove_action = new QAction(QIcon(Helper::getIconPath("delete.png")),
                                 tr("Remove"), this);
    _delete_action = new QAction(QIcon(Helper::getIconPath("delete.png")),
                                 tr("Delete"), this);
    _play_next_action =
        new QAction(QIcon(Helper::getIconPath("fwd_orange.png")),
                    tr("Play next"), this);
    _append_action = new QAction(QIcon(Helper::getIconPath("append.png")),
                                 tr("Append"), this);
    _sort_tno_action = new QAction(tr("Sort by track number"), this);
    _invert_selection_action = new QAction(tr("Invert Selection"), this);
}


void ContextMenu::clear_actions()
{
    QList<QAction*> actions = this->actions();
    if (actions.size() > 0) {
        foreach(QAction * a, actions)
        this->removeAction(a);
    }

    disconnect(_info_action, SIGNAL(triggered()),
               this, SIGNAL(sig_info_clicked()));
    disconnect(_edit_action, SIGNAL(triggered()),
               this, SIGNAL(sig_edit_clicked()));
    disconnect(_remove_action, SIGNAL(triggered()), this,
               SIGNAL(sig_remove_clicked()));
    disconnect(_delete_action, SIGNAL(triggered()), this,
               SIGNAL(sig_delete_clicked()));
    disconnect(_play_next_action, SIGNAL(triggered()), this,
               SIGNAL(sig_play_next_clicked()));
    disconnect(_sort_tno_action, SIGNAL(triggered()), this,
               SIGNAL(sig_sort_tno_clicked()));
    disconnect(_invert_selection_action, SIGNAL(triggered()), this,
               SIGNAL(sig_invert_selection_clicked()));
}

void ContextMenu::setup_entries(int entries)
{
    clear_actions();

    if (entries & ENTRY_INFO) {
        this->addAction(_info_action);
        connect(_info_action, SIGNAL(triggered()),
                this, SIGNAL(sig_info_clicked()));
    }
    if (entries & ENTRY_EDIT) {
        this->addAction(_edit_action);
        connect(_edit_action, SIGNAL(triggered()),
                this, SIGNAL(sig_edit_clicked()));
    }
    if (entries & ENTRY_REMOVE) {
        this->addAction(_remove_action);
        connect(_remove_action, SIGNAL(triggered()),
                this, SIGNAL(sig_remove_clicked()));
    }
    if (entries & ENTRY_DELETE) {
        this->addAction(_delete_action);
        connect(_delete_action, SIGNAL(triggered()),
                this, SIGNAL(sig_delete_clicked()));
    }
    if (entries & ENTRY_PLAY_NEXT) {
        this->addAction(_play_next_action);
        connect(_play_next_action, SIGNAL(triggered()),
                this, SIGNAL(sig_play_next_clicked()));
    }
    if (entries & ENTRY_APPEND) {
        this->addAction(_append_action);
        connect(_append_action, SIGNAL(triggered()),
                this, SIGNAL(sig_append_clicked()));
    }
    if (entries & ENTRY_SORT_TNO) {
        this->addAction(_sort_tno_action);
        connect(_sort_tno_action, SIGNAL(triggered()),
                this, SIGNAL(sig_sort_tno_clicked()));
    }
    if (entries & ENTRY_INVERT_SELECTION) {
        this->addAction(_invert_selection_action);
        connect(_invert_selection_action, SIGNAL(triggered()),
                this, SIGNAL(sig_invert_selection_clicked()));
    }
}
