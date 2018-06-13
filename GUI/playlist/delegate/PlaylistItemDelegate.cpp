/* Copyright (C) 2012  Lucio Carreras
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

#include <QObject>
#include <QLabel>
#include <QListView>
#include <QPainter>
#include "GUI/playlist/delegate/PlaylistItemDelegate.h"
#include "GUI/playlist/entry/GUI_PlaylistEntryBig.h"
#include "GUI/playlist/entry/GUI_PlaylistEntrySmall.h"

static QString get_fg_color(int val_bg)
{
    if (val_bg > 160) {
        return  QString(" color: #202020; ");
    } else {
        return QString(" color: #D8D8D8; ");
    }
}

PlaylistItemDelegate::PlaylistItemDelegate(QListView* parent, bool compact)
    : _max_width(100), _compact(compact), _parent(parent)
{
    // There is no way to apply the style sheet and compute the row
    // height here, the actual font metrics are only known during the
    // paint event (apparently...)
    if (compact) {
        _pl_entry = new GUI_PlaylistEntrySmall();
    } else {
        _pl_entry = new GUI_PlaylistEntryBig();
    }

}

PlaylistItemDelegate::~PlaylistItemDelegate()
{
    delete _pl_entry;
}

void PlaylistItemDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    if (!index.isValid()) {
        return;
    }

    QVariant mdVariant = index.model()->data(index, Qt::WhatsThisRole);
    MetaData md;
    if (!MetaData::fromVariant(mdVariant, md)) {
        return;
    }
    _pl_entry->setContent(md, index.row() + 1);

    // This keeps the time always visible
    _pl_entry->setMaximumWidth(_max_width);
    _pl_entry->setMinimumWidth(_max_width);
    _pl_entry->resize(_max_width, rowHeight());
    QString style;
    QPalette palette = _parent->palette();

    QColor col_background = palette.color(QPalette::Active,
                                          QPalette::Background);
    QColor col_highlight = palette.color(QPalette::Active, QPalette::Highlight);
    QColor col_highlight_lighter = col_highlight.darker(140);

    int highlight_val = col_highlight.lightness();
    int playing_val = col_highlight_lighter.lightness();
    int background_val = col_background.lightness();

    if (md.pl_playing) {
        style += QString("background-color: ") +
                 col_highlight_lighter.name() + "; " +
                 get_fg_color(playing_val);
        _pl_entry->setProperty("play_state", "playing");
    } else if (md.is_disabled) {
        style += QString("color: #A0A0A0; background-color: transparent;");
        _pl_entry->setProperty("play_state", "disabled");
    } else if (!md.pl_selected) {
        style += QString("background-color: transparent; ") +
                 get_fg_color(background_val);
        _pl_entry->setProperty("play_state", "selected");
    } else {
        // standard selected
        style += QString("background-color: ") +
                 col_highlight.name() + ";" +
                 get_fg_color(highlight_val);
        _pl_entry->setProperty("play_state", "default");
    }

    QRect rect(option.rect);
    int y = rect.topLeft().y() +  rowHeight() - 1;

    _pl_entry->setStyleSheet(style);

    if (md.is_disabled) {
        _pl_entry->setDisabled(true);
    }

    painter->save();
    painter->translate(0, 0);

    _pl_entry->render(painter, rect.topLeft());

    if (md.pl_dragged) {
        painter->drawLine(QLine(0, y, _max_width, y));
    }

    painter->restore();
}

QSize PlaylistItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(_max_width, rowHeight());
}

void PlaylistItemDelegate::setMaxWidth(int w)
{
    _max_width = w;
}

int PlaylistItemDelegate::rowHeight() const
{
    int h  = _pl_entry->fontheight();
    if (_compact) {
        h = h  + 3;
    } else {
        h = 2 * h  + 3;
    }
    //qDebug() << "PlaylistItemDelegate::rowHeight(): "<< h;
    return h;
}
