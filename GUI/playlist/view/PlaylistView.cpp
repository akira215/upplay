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
#include <QApplication>
#include <QDebug>
#include <QUrl>
#include <QScrollBar>
#include <QTimer>

#include "HelperStructs/CustomMimeData.h"
#include "HelperStructs/Helper.h"
#include "PlaylistView.h"
#include "GUI/playlist/delegate/PlaylistItemDelegate.h"

// We avoid scrolling to make the currently playing track visible if the
// user is currently active (e.g. selecting tracks). Reset after 10 S
static const int user_activity_timeout_ms = 10 * 1000;

PlaylistView::PlaylistView(QWidget* parent)
    : QListView(parent), m_usertimer(new QTimer(this))
{
    m_usertimer->setSingleShot(true);
    m_usertimer->setInterval(user_activity_timeout_ms);

    setMouseTracking(true);

    _drag_allowed = true;
    _inner_drag_drop = false;
    _parent = parent;
    _qDrag = 0;
    _last_known_drag_row = -1;
    _model = new PlaylistItemModel(this);
    _delegate = new PlaylistItemDelegate(this, true);
    _rc_menu = 0;

    this->setModel(_model);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setSelectionRectVisible(true);
    this->setAlternatingRowColors(true);
    this->setMovement(QListView::Free);

    connect(this, SIGNAL(pressed(const QModelIndex&)),
            this, SLOT(row_pressed(const QModelIndex&)));
    connect(this, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(row_double_clicked(const QModelIndex&)));
    connect(this, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(row_released(const QModelIndex&)));
    connect(horizontalScrollBar(), SIGNAL(sliderPressed()),
            m_usertimer, SLOT(start()));
    connect(verticalScrollBar(), SIGNAL(sliderPressed()),
            m_usertimer, SLOT(start()));
}

PlaylistView::~PlaylistView()
{
    delete _rc_menu;
    delete _model;
}

void PlaylistView::mousePressEvent(QMouseEvent* event)
{
    m_usertimer->start(user_activity_timeout_ms);

    QPoint pos_org = event->pos();
    QPoint pos = QWidget::mapToGlobal(pos_org);

    switch (event->button()) {

    case Qt::LeftButton:
        if (!_drag_allowed) {
            break;
        }

        QListView::mousePressEvent(event);

        if ((this->model()->rowCount()) * 33 > event->pos().y()) {
            _drag_pos = event->pos();
        } else {
            _drag_pos.setY(-10);
            _drag = false;
        }

        break;

    case Qt::RightButton:
        _drag = false;

        QListView::mousePressEvent(event);
        pos.setY(pos.y());
        pos.setX(pos.x() + 10);

        if (_rc_menu) {
            _rc_menu->exec(pos);
        }

        break;

    default:
        _drag = false;
        break;
    }
}


void PlaylistView::mouseMoveEvent(QMouseEvent* event)
{
    m_usertimer->start(user_activity_timeout_ms);

    QPoint pos = event->pos();
    int distance =  abs(pos.x() - _drag_pos.x()) +  abs(pos.y() - _drag_pos.y());

    if (_drag && _qDrag && (distance > 10) && _drag_allowed) {
        _qDrag->exec(Qt::CopyAction);
    }
}


void PlaylistView::mouseReleaseEvent(QMouseEvent* event)
{
    m_usertimer->start(user_activity_timeout_ms);

    switch (event->button()) {

    case Qt::LeftButton:

        if (_qDrag) {
            delete _qDrag;
            _qDrag = NULL;
        }

        QListView::mouseReleaseEvent(event);
        event->accept();

        _drag = false;
        break;

    default:
        break;
    }
}


// get the min index of selected rows
int PlaylistView::get_min_selected()
{

    QModelIndexList lst = this->selectedIndexes();
    int min_row = 5000000;

    if (lst.size() == 0) {
        return 0;
    }

    foreach(QModelIndex i, lst) {
        if (i.row() < min_row) {
            min_row = i.row();
        }
    }

    return min_row;
}

// mark row as currently pressed
void PlaylistView::goto_row(int row)
{
    if ((row >= _model->rowCount()) || (row < 0)) {
        return;
    }

    this->clearSelection();

    QModelIndex idx = _model->index(row, 0);
    QList<int> lst_rows;
    lst_rows << row;
    this->select_rows(lst_rows);
    row_released(idx);
    this->scrollTo(idx);
}


void PlaylistView::keyPressEvent(QKeyEvent* event)
{
    m_usertimer->start(user_activity_timeout_ms);

    int key = event->key();

    Qt::KeyboardModifiers  modifiers = event->modifiers();
    int min_row = get_min_selected();
    int new_row = -1;


    switch (key) {
    case Qt::Key_A:
        if (modifiers & Qt::ControlModifier) {
            select_all();
        }
        break;

    case Qt::Key_Delete:
        remove_cur_selected_rows();
        break;

    case Qt::Key_Up:
        if (modifiers & Qt::ControlModifier) {
            break;
        }

        if (min_row > 0) {
            new_row = min_row - 1;
        } else {
            new_row = 0;
        }
        break;

    case Qt::Key_Down:
        if (modifiers & Qt::ControlModifier) {
            break;
        }

        if (min_row < _model->rowCount() - 1) {
            new_row = min_row + 1;
        } else {
            new_row = _model->rowCount() - 1;
        }
        break;

    case Qt::Key_PageUp:
        if (min_row > 10) {
            new_row = min_row - 10;
        } else {
            new_row = 0;
        }
        break;

    case Qt::Key_PageDown:
        if (min_row < _model->rowCount() - 10) {
            new_row = min_row + 10;
        } else {
            new_row =  _model->rowCount() - 1;
        }
        break;

    case Qt::Key_End:
        new_row = _model->rowCount() - 1;
        break;

    case Qt::Key_Home:
        new_row = 0;
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        this->sig_double_clicked(min_row);
        break;

    case Qt::Key_Tab:
        emit sig_no_focus();
        break;

    default:
        break;
    }

    if (new_row != -1) {
        goto_row(new_row);
    }
}

void PlaylistView::resizeEvent(QResizeEvent *e)
{

    this->set_delegate_max_width(_model->rowCount());
    e->accept();

}

void PlaylistView::init_rc_menu()
{
    _rc_menu = new ContextMenu(this);

    connect(_rc_menu, SIGNAL(sig_info_clicked()), this, SLOT(info_clicked()));
    connect(_rc_menu, SIGNAL(sig_edit_clicked()), this, SLOT(edit_clicked()));
    connect(_rc_menu, SIGNAL(sig_remove_clicked()), this,
            SLOT(remove_clicked()));
    connect(_rc_menu, SIGNAL(sig_sort_tno_clicked()),
            this, SIGNAL(sig_sort_tno()));
    connect(_rc_menu, SIGNAL(sig_invert_selection_clicked()),
            this, SLOT(invert_selection()));
}

void PlaylistView::set_context_menu_actions(int actions)
{
    if (!_rc_menu) {
        init_rc_menu();
    }
    _rc_menu->setup_entries(actions);
}


void PlaylistView::set_mimedata(MetaDataList& v_md, QString text)
{

    if (!_drag_allowed) {
        return;
    }
    if (_qDrag) {
        delete _qDrag;
    }

    CustomMimeData* mimedata = new CustomMimeData();

    QList<QUrl> urls;
    foreach(MetaData md, v_md) {
        QUrl url(QString("file://") + md.filepath);
        urls << url;
    }

    mimedata->setMetaData(v_md);
    mimedata->setText(text);
    mimedata->setUrls(urls);

    _qDrag = new QDrag(this);
    _qDrag->setMimeData(mimedata);

    connect(_qDrag, SIGNAL(destroyed()),
            this, SLOT(forbid_mimedata_destroyable()));

    _drag = true;
}

void PlaylistView::forbid_mimedata_destroyable()
{
    _qDrag = NULL;
}

void PlaylistView::set_drag_enabled(bool b)
{
    _drag_allowed = b;
}


int PlaylistView::get_num_rows()
{
    return _model->rowCount();
}

#define CAN_AUTOSCROLL (!(m_usertimer->isActive() ||                    \
                          horizontalScrollBar()->isSliderDown() ||      \
                          verticalScrollBar()->isSliderDown()))

void PlaylistView::set_current_track(int row)
{
    for (int i = 0; i < _model->rowCount(); i++) {
        QModelIndex idx = _model->index(i);
        MetaData md;
        QVariant v = _model->data(idx, Qt::WhatsThisRole);
        if (!MetaData::fromVariant(v, md)) {
            continue;
        }
        md.pl_playing = (row == i);

        _model->setData(idx, md.toVariant(), Qt::EditRole);
    }

    QModelIndex new_idx = _model->index(row);

    // Don't move the playlist around if the user is doing something
    if (CAN_AUTOSCROLL) {
        scrollTo(new_idx,  QListView::EnsureVisible);
    }
}


void PlaylistView::edit_clicked()
{
    emit sig_edit_clicked();
}


void PlaylistView::info_clicked()
{
    emit sig_info_clicked();
}

void PlaylistView::remove_clicked()
{
    remove_cur_selected_rows();
}

void PlaylistView::clear()
{
    clear_selection();
    _model->removeRows(0, _model->rowCount());
}


void PlaylistView::fill(MetaDataList& v_metadata, int cur_play_idx)
{

    this->set_delegate_max_width((int) v_metadata.size());

    _model->removeRows(0, _model->rowCount());
    if (v_metadata.size() == 0) {
        return;
    }

    _model->insertRows(0, v_metadata.size());
    _cur_selected_rows.clear();

    QModelIndex idx_cur_playing = _model->index(0);
    for (uint i = 0; i < v_metadata.size(); i++) {
        MetaData md = v_metadata[i];

        QModelIndex model_idx = _model->index(i, 0);

        md.pl_playing = (cur_play_idx == int(i));
        if (md.pl_playing) {
            idx_cur_playing = model_idx;
        }

        if (md.pl_selected) {
            _cur_selected_rows << i;
        }

        _model->setData(model_idx, md.toVariant(), Qt::EditRole);
    }

    _model->set_selected(_cur_selected_rows);
    this->select_rows(_cur_selected_rows);

    // Don't move the playlist around if the user is doing something
    if (CAN_AUTOSCROLL) {
        this->scrollTo(idx_cur_playing, QListView::EnsureVisible);
    }
}

void PlaylistView::row_pressed(const QModelIndex&)
{

    QList<int> selected_rows = calc_selections();
    _inner_drag_drop = true;

    MetaDataList v_md;
    foreach(int row, selected_rows) {
        QVariant mdvariant = _model->data(_model->index(row), Qt::WhatsThisRole);

        MetaData md;

        if (!MetaData::fromVariant(mdvariant, md)) {
            continue;
        }

        v_md.push_back(md);
    }

    if (QApplication::mouseButtons() == Qt::LeftButton) {
        set_mimedata(v_md, "tracks");
    }
    emit sig_selection_changed(v_md);
}

void PlaylistView::row_released(const QModelIndex&)
{
    calc_selections();
    _inner_drag_drop = false;
}

void PlaylistView::row_double_clicked(const QModelIndex& idx)
{
    _inner_drag_drop = false;

    if (idx.isValid()) {
        emit sig_double_clicked(idx.row());
    }
}

void PlaylistView::clear_selection()
{
    this->selectionModel()->clearSelection();
    this->clearSelection();
    calc_selections();
}

void PlaylistView::select_rows(QList<int> lst)
{
    QItemSelectionModel* sm = this->selectionModel();
    QItemSelection sel;

    foreach(int row, lst) {
        QModelIndex idx = _model->index(row);

        sm->select(idx, QItemSelectionModel::Select);
        sel.merge(sm->selection(), QItemSelectionModel::Select);
    }

    sm->clearSelection();
    sm->select(sel, QItemSelectionModel::Select);

    _cur_selected_rows = calc_selections();
}

void PlaylistView::invert_selection()
{
    for (int row = 0; row < model()->rowCount(); row++) {
        this->selectionModel()->select(_model->index(row),
                                       QItemSelectionModel::Toggle);
    }
    calc_selections();
}

void PlaylistView::select_all()
{
    selectAll();
    calc_selections();
}

QList<int> PlaylistView::calc_selections()
{

    QList<int> selections;

    QModelIndexList idx_list = this->selectionModel()->selectedRows();

    foreach(QModelIndex model_idx, idx_list) {
        selections.push_back(model_idx.row());
    }

    _model->set_selected(selections);

    _cur_selected_rows = selections;

    if (selections.empty()) {
        emit sig_selection_min_row(-1);
    } else {
        emit sig_selection_min_row(get_min_selected());
    }
    return selections;
}


// remove the black line under the titles
void  PlaylistView::clear_drag_lines(int row)
{

    for (int i = row - 3; i <= row + 3; i++) {

        QModelIndex idx = _model->index(i, 0);
        if (!idx.isValid() || idx.row() < 0 || idx.row() >= _model->rowCount()) {
            continue;
        }

        QVariant mdVariant = _model->data(idx, Qt::WhatsThisRole);
        MetaData md;
        if (MetaData::fromVariant(mdVariant, md)) {

            md.pl_dragged = false;
            _model->setData(idx, md.toVariant(), Qt::EditRole);
        }

    }
}



int PlaylistView::calc_dd_line(QPoint pos)
{

    if (pos.y() < 0) {
        return -1;
    }
    int row = this->indexAt(pos).row();

    if (row <= -1) {
        row = _model->rowCount() - 1;
    }
    return row;
}


// the drag comes, if there's data --> accept it
void PlaylistView::dragEnterEvent(QDragEnterEvent* event)
{
    event->accept();
}

void PlaylistView::dragMoveEvent(QDragMoveEvent* event)
{

    if (!event->mimeData()) {
        return;
    }
    event->accept();

    int row = calc_dd_line(event->pos());

    _last_known_drag_row = row;
    clear_drag_lines(row);

    // paint line
    QModelIndex cur_idx = _model->index(row, 0);
    QVariant mdVariant = _model->data(cur_idx, Qt::WhatsThisRole);
    MetaData md;
    if (!MetaData::fromVariant(mdVariant, md)) {
        return;
    }

    md.pl_dragged = true;
    _model->setData(cur_idx, md.toVariant(), Qt::EditRole);
}


// we start the drag action, all lines has to be cleared
void PlaylistView::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();

    clear_drag_lines(_last_known_drag_row);
}


void PlaylistView::dropEventFromOutside(QDropEvent* event)
{
    if (event->pos().y() < this->y()) {
        handle_drop(event, true);
    }

}

// finally drop it
void PlaylistView::dropEvent(QDropEvent* event)
{

    event->accept();

    if (!event->mimeData()) {
        return;
    }
    handle_drop(event, false);

}

void PlaylistView::handle_drop(QDropEvent* event, bool from_outside)
{
    if (_cur_selected_rows.empty()) {
        return;
    }

    QList<int> affected_rows;

    QPoint pos = event->pos();
    if (from_outside) {
        pos.setY(pos.y() - this->y());
    }

    // where did i drop?
    int row = calc_dd_line(pos);

    if (_inner_drag_drop) {
        _inner_drag_drop = false;
        if (_cur_selected_rows.contains(row)) {
            event->ignore();
            clear_drag_lines(row);
            return;
        }

        if (_cur_selected_rows.first() < row) {
            row -= _cur_selected_rows.size();
        }

        remove_cur_selected_rows(false);
    }

    const CustomMimeData* d = (const CustomMimeData*) event->mimeData();

    MetaDataList v_metadata;

    QString text = "";
    if (d->hasText()) {
        text = d->text();
    }

    // extern
    if (d->hasUrls() && text.compare("tracks", Qt::CaseInsensitive)) {

        QStringList filelist;
        foreach(QUrl url, d->urls()) {
            QString path;
            QString url_str = url.toString();
            path =  url_str.right(url_str.length() - 7).trimmed();
            path = path.replace("%20", " ");
            filelist.push_back(path);

        } // end foreach

#if 0
        CDirectoryReader reader;
        reader.setFilter(Helper::get_soundfile_extensions());

        reader.getMetadataFromFileList(filelist, v_metadata);
#endif
        if (v_metadata.size() == 0) {
            return;
        }
    } else if (d->hasHtml()) {
    } else if (d->hasImage()) {
    } else if (d->hasText() && d->hasMetaData()) {
        uint sz = d->getMetaData(v_metadata);
        if (sz == 0) {
            return;
        }
    } else if (d->hasText()) {
    } else {
    }

    for (uint i = 0; i < v_metadata.size(); i++) {
        affected_rows << i + row + 1;
    }
    if (!affected_rows.empty()) {
        emit sig_metadata_dropped(v_metadata, row);
    }
}


void PlaylistView::scrollUp()
{
    QPoint p(5, 5);
    int cur_row = this->indexAt(p).row();
    if (cur_row <= 0) {
        return;
    }

    this->scrollTo(_model->index(cur_row - 1));
}

void PlaylistView::scrollDown()
{
    QPoint p(5, this->y() + this->height() - 5);
    int cur_row = this->indexAt(p).row();
    if (cur_row <= 0) {
        return;
    }

    this->scrollTo(_model->index(cur_row - 1));
}

void PlaylistView::remove_cur_selected_rows(bool select_next_row)
{
    emit sig_rows_removed(_cur_selected_rows, select_next_row);
}


void PlaylistView::show_big_items(bool big)
{

    if (_delegate) {
        delete _delegate;
    }
    _delegate = new PlaylistItemDelegate(this, !big);

    this->set_delegate_max_width(_model->rowCount());
    this->setItemDelegate(_delegate);

    this->reset();
}


void PlaylistView::set_delegate_max_width(int n_items)
{
    bool scrollbar_visible = ((n_items * _delegate->rowHeight()) >=
                              this->height());

    int max_width = width();
    if (scrollbar_visible) {
        max_width -= verticalScrollBar()->width();
    }

    _delegate->setMaxWidth(max_width);
}
