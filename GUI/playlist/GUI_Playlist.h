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
#ifndef GUI_PLAYLIST_H_
#define GUI_PLAYLIST_H_

#include <string>

#include <QObject>
#include <QMainWindow>
#include <QWidget>
#include <QKeyEvent>
#include <QTextEdit>
#include <QList>
#include <QFocusEvent>

#include "GUI_Playlist.h"
#include "ui_GUI_Playlist.h"
#include "GUI/playlist/delegate/PlaylistItemDelegate.h"
#include "playlist/playlist.h"
#include "HelperStructs/MetaData.h"
#include "HelperStructs/PlaylistMode.h"


class GUI_InfoDialog;

class GUI_Playlist : public QWidget, private Ui::Playlist_Window {
    Q_OBJECT

public:
    GUI_Playlist(QWidget *parent, GUI_InfoDialog* dialog);
    ~GUI_Playlist();

    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent* event);
    void dropEvent(QDropEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);

signals:
    // Emitted on dbl-click
    void row_activated(int);

    // Inform about first row in selection (might want to start there
    // if play is requested)
    void selection_min_row(int);

    void clear_playlist();
    void playlist_mode_changed(const Playlist_Mode&);

    void dropped_tracks(const MetaDataList&, int);
    void sig_rows_removed(const QList<int>&, bool);

    void search_similar_artists(const QString&);
    void sig_no_focus();
    void sig_sort_tno();

public slots:
    void fillPlaylist(MetaDataList&, int, int);
    void track_changed(int);
    void setMode(Playlist_Mode mode);
    void setPlayerMode(Playlist_Mode mode);
    void psl_show_small_playlist_items(bool small_items);
    void psl_next_group_html(QString html);

private slots:
    void selection_changed(MetaDataList&);
    void double_clicked(int);
    void clear_playlist_slot();
    void playlist_mode_changed_slot();

    void btn_numbers_changed(bool);
    void metadata_dropped(const MetaDataList&, int);
    void rows_removed(const QList<int>&, bool select_next_row);
    void no_focus();

private:
    Ui::Playlist_Window*            ui;
    QWidget*                        _parent;
    GUI_InfoDialog*                 _info_dialog;

    Playlist_Mode                   _playlist_mode;

    qint64      _total_msecs;

    void initGUI();

    void set_total_time_label();
    void check_dynamic_play_button();

protected:
    void changeEvent(QEvent* e);
    void resizeEvent(QResizeEvent *e);
    void focusInEvent(QFocusEvent *e);


};



#endif /* GUI_PLAYLIST_H_ */
