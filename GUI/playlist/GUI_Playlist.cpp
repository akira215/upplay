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

#include <QDebug>
#include <QKeyEvent>
#include <QFileDialog>
#include <QScrollBar>
#include <QAbstractListModel>
#include <QStyleFactory>
#include <QMessageBox>
#include <QTextEdit>
#include <QAction>
#include <QMenu>
#include <QUrl>
#include <QFileInfo>

#include "HelperStructs/Helper.h"
#include "HelperStructs/PlaylistMode.h"
#include "HelperStructs/CSettingsStorage.h"
#include "HelperStructs/Style.h"
#include "HelperStructs/globals.h"
#include "GUI/playlist/GUI_Playlist.h"
#include "GUI/playlist/model/PlaylistItemModel.h"

class GUI_InfoDialog;

GUI_Playlist::GUI_Playlist(QWidget *parent, GUI_InfoDialog* dialog) :
    QWidget(parent)
{
    _parent = parent;

    ui = new Ui::Playlist_Window();
    ui->setupUi(this);
    initGUI();

    QAction* clear_action = new QAction(this);
    clear_action->setShortcut(QKeySequence("Ctrl+."));
    clear_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(clear_action, SIGNAL(triggered()),
            this->ui->btn_clear, SLOT(click()));
    this->addAction(clear_action);

    CSettingsStorage* settings = CSettingsStorage::getInstance();
    bool small_playlist_items = settings->getShowSmallPlaylist();

    ui->listView->show_big_items(!small_playlist_items);

    _info_dialog = dialog;

    setMode(settings->getPlaylistMode());

    ui->btn_numbers->setChecked(settings->getPlaylistNumbers());

    setAcceptDrops(true);

    connect(ui->btn_clear, SIGNAL(clicked()),
            this, SLOT(clear_playlist_slot()));
    connect(ui->btn_repAll, SIGNAL(clicked()),
            this, SLOT(playlist_mode_changed_slot()));
    connect(ui->btn_shuffle, SIGNAL(clicked()),
            this, SLOT(playlist_mode_changed_slot()));
    connect(ui->btn_append, SIGNAL(clicked()), 
            this, SLOT(playlist_mode_changed_slot()));
    connect(ui->btn_replace, SIGNAL(clicked()),
            this, SLOT(playlist_mode_changed_slot()));
    connect(ui->btn_playAdded, SIGNAL(clicked()),
            this, SLOT(playlist_mode_changed_slot()));

    connect(ui->listView,SIGNAL(sig_metadata_dropped(const MetaDataList&, int)),
            this, SLOT(metadata_dropped(const MetaDataList&, int)));
    connect(ui->listView, SIGNAL(sig_rows_removed(const QList<int>&, bool)),
            this, SLOT(rows_removed(const QList<int>&, bool)));
    connect(ui->listView, SIGNAL(sig_sort_tno()),
            this, SIGNAL(sig_sort_tno()));
    connect(ui->listView, SIGNAL(sig_selection_changed(MetaDataList&)),
            this, SLOT(selection_changed(MetaDataList&)));
    connect(ui->listView, SIGNAL(sig_selection_min_row(int)),
            this, SIGNAL(selection_min_row(int)));
    connect(ui->listView, SIGNAL(sig_double_clicked(int)),
            this, SLOT(double_clicked(int)));
    connect(ui->listView, SIGNAL(sig_no_focus()), this, SLOT(no_focus()));

    connect(ui->btn_numbers, SIGNAL(toggled(bool)),
            this, SLOT(btn_numbers_changed(bool)));
}


GUI_Playlist::~GUI_Playlist()
{
    delete ui;
}

// This is for restoring from settings
void GUI_Playlist::setMode(Playlist_Mode mode)
{
    _playlist_mode = mode;

    ui->btn_repAll->setChecked(_playlist_mode.repAll);
    ui->btn_shuffle->setChecked(_playlist_mode.shuffle);
    ui->btn_append->setChecked(_playlist_mode.append);
    ui->btn_replace->setChecked(_playlist_mode.replace);
    if (_playlist_mode.replace) {
        ui->btn_append->setEnabled(false);
    } else {
        ui->btn_append->setEnabled(true);
    }
    ui->btn_playAdded->setChecked(_playlist_mode.playAdded);
}

// This is for setting the modes from an openhome player. Only the
// repeat and shuffle bits
void GUI_Playlist::setPlayerMode(Playlist_Mode mode)
{
    _playlist_mode.repAll = mode.repAll;
    _playlist_mode.shuffle = mode.shuffle;

    ui->btn_repAll->setChecked(_playlist_mode.repAll);
    ui->btn_shuffle->setChecked(_playlist_mode.shuffle);
}

void GUI_Playlist::changeEvent(QEvent* e)
{
    e->accept();
}

void GUI_Playlist::resizeEvent(QResizeEvent* e)
{
    e->accept();
    this->ui->listView->update();
    this->ui->listView->reset();
}

void GUI_Playlist::focusInEvent(QFocusEvent *)
{
    this->ui->listView->setFocus();
}

void GUI_Playlist::no_focus()
{
    this->parentWidget()->setFocus();
    emit sig_no_focus();
}

void GUI_Playlist::psl_next_group_html(QString html)
{
    if (html.isEmpty()) {
        this->ui->btn_clear->setToolTip(tr("Clear Playlist"));
    } else {
        this->ui->btn_clear->setToolTip(html);
    }
}

// initialize gui
// maybe the button state (pressed/unpressed) should be loaded from db here
void GUI_Playlist::initGUI()
{
    ui->btn_repAll->setIcon(QIcon(Helper::getIconPath("repAll.png")));
    ui->btn_shuffle->setIcon(QIcon(Helper::getIconPath("shuffle.png")));
    ui->btn_append->setIcon(QIcon(Helper::getIconPath("append.png")));
    ui->btn_replace->setIcon(QIcon(Helper::getIconPath("broom.png")));
    ui->btn_playAdded->setIcon(QIcon(Helper::getIconPath("dynamic.png")));
    ui->btn_clear->setIcon(QIcon(Helper::getIconPath("broom.png")));
    ui->btn_numbers->setIcon(QIcon(Helper::getIconPath("numbers.png")));
}

// Slot: comes from listview
void GUI_Playlist::metadata_dropped(const MetaDataList& v_md, int row)
{
    emit dropped_tracks(v_md, row);
}

// SLOT: fill all tracks in v_metadata into playlist
void GUI_Playlist::fillPlaylist(MetaDataList& v_metadata, int cur_play_idx, int)
{
    ui->listView->setStyleSheet(styleSheet());
    ui->listView->fill(v_metadata, cur_play_idx);
    _total_msecs = 0;

    int actions = ENTRY_REMOVE | ENTRY_SORT_TNO | ENTRY_INVERT_SELECTION;

    ui->listView->set_context_menu_actions(actions);
    ui->listView->set_drag_enabled(true);

    foreach(MetaData md, v_metadata) {
        _total_msecs += md.length_ms;
    }

    set_total_time_label();
}

// private SLOT: clear button pressed
void GUI_Playlist::clear_playlist_slot()
{

    _total_msecs = 0;
    ui->lab_totalTime->setText(tr("Playlist empty"));
    ui->listView->clear();

    emit clear_playlist();
}


// private SLOT: playlist item pressed (init drag & drop)
void GUI_Playlist::selection_changed(MetaDataList&)
{
//    _info_dialog->setMetaData(v_md);
//    this->_info_dialog->set_tag_edit_visible(true);
}


void GUI_Playlist::double_clicked(int row)
{
    emit row_activated(row);
}

void GUI_Playlist::track_changed(int row)
{
    ui->listView->set_current_track(row);
}

// This is connected to the UI buttons
void GUI_Playlist::playlist_mode_changed_slot()
{
    this->parentWidget()->setFocus();

    _playlist_mode.repAll = ui->btn_repAll->isChecked();
    _playlist_mode.shuffle = ui->btn_shuffle->isChecked();
    _playlist_mode.append = ui->btn_append->isChecked();
    _playlist_mode.replace = ui->btn_replace->isChecked();
    _playlist_mode.playAdded = ui->btn_playAdded->isChecked();

    if (_playlist_mode.replace) {
        ui->btn_append->setEnabled(false);
    } else {
        ui->btn_append->setEnabled(true);
    }
    emit playlist_mode_changed(_playlist_mode);
}


void GUI_Playlist::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
}


void GUI_Playlist::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}

void GUI_Playlist::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->pos().y() < this->ui->listView->y()) {
        this->ui->listView->scrollUp();
    } else if (event->pos().y() >
               this->ui->listView->y() + this->ui->listView->height()) {
        this->ui->listView->scrollDown();
    }
}


void GUI_Playlist::dropEvent(QDropEvent* event)
{
    this->ui->listView->dropEventFromOutside(event);
}


void GUI_Playlist::set_total_time_label()
{
    QString text = "";

    ui->lab_totalTime->setContentsMargins(0, 2, 0, 2);

    int n_rows = ui->listView->get_num_rows();
    QString playlist_string = text + QString::number(n_rows);

    if (n_rows == 1) {
        playlist_string += tr(" Track - ");
    } else {
        playlist_string += tr(" Tracks - ");
    }

    playlist_string += Helper::cvtMsecs2TitleLengthString(_total_msecs, false);

    ui->lab_totalTime->setText(playlist_string);
}


void GUI_Playlist::psl_show_small_playlist_items(bool small_playlist_items)
{
    ui->listView->show_big_items(!small_playlist_items);
}


void GUI_Playlist::btn_numbers_changed(bool b)
{
    this->parentWidget()->setFocus();
    CSettingsStorage::getInstance()->setPlaylistNumbers(b);
    ui->listView->reset();
}


void GUI_Playlist::rows_removed(const QList<int>& lst, bool select_next_row)
{
    emit sig_rows_removed(lst, select_next_row);
}
