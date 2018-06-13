/* Copyright (C) 2014 J.F.Dockes
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <QToolButton>
#include <QTabBar>
#include <QIcon>
#include <QTimer>
#include <QShortcut>
#include <QLineEdit>

#include "dirbrowser.h"
#include "HelperStructs/Helper.h"
#include "upadapt/upputils.h"
#include "GUI/prefs/sortprefs.h"
#include "HelperStructs/CSettingsStorage.h"

DirBrowser::DirBrowser(QWidget *parent, std::shared_ptr<Playlist> pl)
    : QWidget(parent), ui(new Ui::DirBrowser), m_pl(pl), m_insertactive(false),
      m_randplayer(0)
{
    QKeySequence seq;
    QShortcut *sc;

    ui->setupUi(this);
    ui->searchFrame->hide();
    ui->tabs->setTabsClosable(true);

    QToolButton *plusbut = new QToolButton(this);
    QIcon icon = QIcon(Helper::getIconPath("addtab.png"));
    plusbut->setIcon(icon);
    ui->tabs->setCornerWidget(plusbut, Qt::TopRightCorner);
    plusbut->setCursor(Qt::ArrowCursor);
    plusbut->setAutoRaise(true);
    plusbut->setToolTip(tr("Add tab"));
    connect(plusbut, SIGNAL(clicked()), this, SLOT(addTab()));
    seq = QKeySequence("Ctrl+t");
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL (activated()), this, SLOT(addTab()));
    seq = QKeySequence("Ctrl+w");
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL (activated()), this, SLOT(closeCurrentTab()));

    connect(ui->tabs, SIGNAL(currentChanged(int)), 
            this, SLOT(onCurrentTabChanged(int)));
    connect(ui->tabs, SIGNAL(tabCloseRequested(int)), 
            this, SLOT(closeTab(int)));
            
    icon = QIcon(Helper::getIconPath("cross.png"));
    ui->closeSearchTB->setIcon(icon);
    connect(ui->closeSearchTB, SIGNAL(clicked()), 
            this, SLOT(closeSearchPanel()));
    seq = QKeySequence("Ctrl+f");
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL (activated()), this, SLOT(openSearchPanel()));
    seq = QKeySequence("/");
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL (activated()), this, SLOT(openSearchPanel()));
    seq = QKeySequence("Esc");
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL (activated()), this, SLOT(closeSearchPanel()));

    seq = QKeySequence(Qt::Key_F3);
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL(activated()), this, SLOT(searchNext()));
    seq = QKeySequence(Qt::SHIFT + Qt::Key_F3);
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL(activated()), this, SLOT(searchPrev()));
    connect(ui->searchCMB->lineEdit(), SIGNAL(textChanged(const QString&)), 
            this, SLOT(onSearchTextChanged(const QString&)));
    connect(ui->nextTB, SIGNAL(clicked()), this, SLOT(searchNext()));
    connect(ui->prevTB, SIGNAL(clicked()), this, SLOT(searchPrev()));
    connect(ui->searchCMB->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(returnPressedInSearch()));
    seq = QKeySequence("Ctrl+s");
    sc = new QShortcut(seq, this);
    connect(sc, SIGNAL(activated()), this, SLOT(toggleSearchKind()));
    connect(ui->serverSearchCB, SIGNAL(stateChanged(int)), 
            this, SLOT(onSearchKindChanged(int)));
    connect(ui->execSearchPB, SIGNAL(clicked()), this, SLOT(serverSearch()));

    onSearchKindChanged(int(ui->serverSearchCB->checkState()));
    ui->execSearchPB->hide();
    setPlaylist(pl);
}

void DirBrowser::setPlaylist(std::shared_ptr<Playlist> pl)
{
    m_pl = pl;

    for (int i = 0; i < ui->tabs->count(); i++) {
        setupTabConnections(i);
    }
    if (m_pl)
        connect(m_pl.get(), SIGNAL(sig_insert_done()),
                this, SLOT(onInsertDone()));
}

void DirBrowser::setStyleSheet(bool dark)
{
    for (int i = 0; i < ui->tabs->count(); i++) {
        QWidget *tw = ui->tabs->widget(i);
        if (tw) {
	    CDBrowser *cdb = tw->findChild<CDBrowser*>();
            if (cdb) {
		cdb->setStyleSheet(dark);
            }
        }
    }
}

    
void DirBrowser::addTab()
{
    QWidget *tab = new QWidget(this);
    tab->setObjectName(QString::fromUtf8("tab"));
    QVBoxLayout* vl = new QVBoxLayout(tab);

    CDBrowser *cdb = new CDBrowser(tab);
    cdb->setObjectName(QString::fromUtf8("cdBrowser"));
    setupTabConnections(cdb);

    vl->addWidget(cdb);
    ui->tabs->addTab(tab, QString());
    ui->tabs->setCurrentIndex(ui->tabs->count() -1);
}

void DirBrowser::closeCurrentTab()
{
    closeTab(ui->tabs->currentIndex());
}

void DirBrowser::closeTab(int i)
{
    if (m_insertactive || ui->tabs->count() <= 1)
        return;
    QWidget *w = ui->tabs->widget(i);
    ui->tabs->removeTab(i);
    delete w;
}

static const QString UPnPSearchCheat(
    "Album/Artist etc.: just enter a value.<br>"
    "UPnP search string example:<br>"
    "upnp:artist contains \"bach\" and "
    "dc:title contains \"allegro\""
    );

// Adjust the search panel according the the server caps for the
// active tab
void DirBrowser::onCurrentTabChanged(int)
{
    CDBrowser *cdb = currentBrowser();
    set<string> caps;
    if (cdb)
        cdb->getSearchCaps(caps);
    QString strcaps = u8s2qs("<i></i>Server searchable fields: ") +
                             u8s2qs(stringsToString(caps));

    while(ui->propsCMB->count())
        ui->propsCMB->removeItem(0);
    if (caps.empty()) {
        ui->serverSearchCB->setCheckState(Qt::Unchecked);
        ui->serverSearchCB->setEnabled(false);
    } else {
        ui->serverSearchCB->setEnabled(true);
        QString scs;
        vector<pair<string, string> > props;
        props.push_back(pair<string,string>("upnp:artist", "Artist"));
        props.push_back(pair<string,string>("upnp:album", "Album"));
        props.push_back(pair<string,string>("dc:title", "Title"));
        props.push_back(pair<string,string>("upnp:genre", "Genre"));
        for (vector<pair<string, string> >::iterator it = props.begin(); 
             it != props.end(); it++) {
            if (caps.find("*") != caps.end() || 
                caps.find(it->first) != caps.end()) {
                ui->propsCMB->addItem(u8s2qs(it->second),
                                      QVariant(u8s2qs(it->first)));
            }
            scs += u8s2qs(it->first.c_str()) + " ";
        }
        ui->propsCMB->addItem(tr("UPnP search string"),
                              QVariant(u8s2qs("*")));
        ui->propsCMB->setToolTip(strcaps);
        ui->searchCMB->setToolTip(UPnPSearchCheat);
        onSearchKindChanged(ui->serverSearchCB->checkState());
    }
}

static QString escapeAmpersand(const QString& in)
{
    QString out(in);
    return out.replace(QChar('&'), QString::fromUtf8("&&"));
}

void DirBrowser::changeTabTitle(QWidget *w, const QString& tt)
{
    int i = ui->tabs->indexOf((QWidget*)w->parent());
    if (i >= 0) {
        ui->tabs->setTabText(i, escapeAmpersand(tt));
    } else {
        qDebug() << "changeTabTitle: Widget not found in tabs: " << w;
    }
}

// Any of the tabs searchcaps changed. Update the current one just in case.
void DirBrowser::onSearchcapsChanged()
{
    onCurrentTabChanged(-1);
}

void DirBrowser::openSearchPanel()
{
    onCurrentTabChanged(-1);
    ui->searchFrame->show();
    ui->searchCMB->lineEdit()->setFocus();
    ui->searchCMB->lineEdit()->selectAll();
}

void DirBrowser::closeSearchPanel()
{
    QString text = ui->searchCMB->lineEdit()->text();
    if (ui->searchCMB->findText(text))
        ui->searchCMB->insertItem(0, text);
    ui->searchFrame->hide();
}

void DirBrowser::showSearchPanel(bool yes)
{
    if (yes)
        openSearchPanel();
    else
        closeSearchPanel();
}

void DirBrowser::toggleSearchKind()
{
    if (!ui->serverSearchCB->isEnabled())
        return;
    if (ui->serverSearchCB->checkState()) {
        ui->serverSearchCB->setCheckState(Qt::Unchecked);
    } else {
        ui->serverSearchCB->setCheckState(Qt::Checked);
    }
    onSearchKindChanged(ui->serverSearchCB->checkState());
}

void DirBrowser::onSearchKindChanged(int state)
{
    if (state == Qt::Unchecked) {
//        ui->propsCMB->hide();
        ui->propsCMB->setEnabled(false);
        ui->execSearchPB->hide();
        ui->execSearchPB->setEnabled(false);
        ui->nextTB->show();
        ui->nextTB->setEnabled(true);
        ui->prevTB->show();
        ui->prevTB->setEnabled(true);
    } else {
//        ui->propsCMB->show();
        ui->propsCMB->setEnabled(true);
        ui->execSearchPB->show();
        ui->execSearchPB->setEnabled(true);
        ui->nextTB->hide();
        ui->nextTB->setEnabled(false);
        ui->prevTB->hide();
        ui->prevTB->setEnabled(false);
    }
}

void DirBrowser::onSearchTextChanged(const QString& text)
{
    if (!ui->propsCMB->isEnabled()) {
        if (!text.isEmpty()) {
            doSearch(text, false);
        }
    }
}

void DirBrowser::searchNext()
{
    doSearch(ui->searchCMB->currentText().trimmed(), false);
}

void DirBrowser::searchPrev()
{
    doSearch(ui->searchCMB->currentText().trimmed(), true);
}

void DirBrowser::returnPressedInSearch()
{
    if (ui->propsCMB->isEnabled()) {
        serverSearch();
    } else {
        QString text = ui->searchCMB->lineEdit()->text();
        if (ui->searchCMB->findText(text))
            ui->searchCMB->insertItem(0, text);
        searchNext();
    }
}

void DirBrowser::serverSearch()
{
    QString text = ui->searchCMB->lineEdit()->text().trimmed();
    int i = ui->propsCMB->currentIndex();
    string prop = qs2utf8s(ui->propsCMB->itemData(i).toString());
    if (text != "") {
        string iss = qs2utf8s(text);
        // prop can be either a field name (e.g. dc:date,
        // upnp:artist), or "*" in which case the user is responsible
        // for entering an appropriate search"
        string ss;
        if (prop == "*") {
            ss = iss;
        } else {
            ss.assign(prop);
            ss += " contains \"";
            for (unsigned i = 0; i < iss.size(); i++) {
                if (iss[i] == '"' || iss[i] == '\\')
                    ss += string("\\");
                ss += iss[i];
            }
            ss += '"';
        } 
        CDBrowser *cdb = currentBrowser();
        if (cdb)
            cdb->search(ss);
    }
}

void DirBrowser::onSortprefs()
{
    qDebug() << "DirBrowser::onSortprefs()";
    currentBrowser()->refresh();
}

// Perform text search in current tab. 
// @param next if true, we look for the next match of the
// current search, trying to advance and possibly wrapping around. Else is
// false, the search string has been modified, we search for the new string, 
// starting from the current position
void DirBrowser::doSearch(const QString& text, bool reverse)
{
    qDebug() << "DirBrowser::doSearch: text " << text << " reverse " << reverse;

    CDBrowser *cdb = currentBrowser();
    if (cdb == 0) {
	return;
    }
#ifdef USING_WEBENGINE
    QWebEnginePage::FindFlags options = 0;
    if (reverse) {
        options |= QWebEnginePage::FindBackward;
    }
    cdb->findText(text, options, [] (bool) {});
#else
    QWebPage::FindFlags options = QWebPage::FindWrapsAroundDocument;
    if (reverse)
        options |= QWebPage::FindBackward;

    cdb->findText(text, options);
#endif
}

void DirBrowser::setInsertActive(bool onoff)
{
    m_insertactive = onoff;
    if (!m_pl)
        m_insertactive = false;
}

bool DirBrowser::insertActive()
{
    return m_insertactive;
}

CDBrowser *DirBrowser::currentBrowser()
{
    QWidget *tw = ui->tabs->currentWidget();
    if (tw == 0)
        return 0;
    return tw->findChild<CDBrowser*>();
}

void DirBrowser::setupTabConnections(int i)
{
    QWidget *tw = ui->tabs->widget(i);
    CDBrowser *cdb = 0;
    if (tw) {
        cdb = tw->findChild<CDBrowser*>();
    } else {
        qDebug() << "Tab " << i << " not found";
        return;
    }
    if (!cdb) {
        qDebug() << "Tab " << i << " has no browser child";
        return;
    }
    setupTabConnections(cdb);
}

void DirBrowser::setupTabConnections(CDBrowser *cdb)
{
    cdb->setDirBrowser(this);

    disconnect(cdb, SIGNAL(sig_now_in(QWidget *, const QString&)), 0, 0);
    connect(cdb, SIGNAL(sig_now_in(QWidget *, const QString&)), 
            this, SLOT(changeTabTitle(QWidget *, const QString&)));

    disconnect(cdb, SIGNAL(sig_tracks_to_playlist(const MetaDataList&)), 0, 0);
    if (m_pl)
        connect(cdb, SIGNAL(sig_tracks_to_playlist(const MetaDataList&)),
                m_pl.get(), SLOT(psl_add_tracks(const MetaDataList&)));

    disconnect(cdb, SIGNAL(sig_searchcaps_changed()), 0, 0);
    connect(cdb, SIGNAL(sig_searchcaps_changed()), 
            this, SLOT(onSearchcapsChanged()));

    disconnect(cdb,
               SIGNAL(sig_browse_in_new_tab(QString,
                                            std::vector<CDBrowser::CtPathElt>)),
               0, 0);
    connect(cdb,
            SIGNAL(sig_browse_in_new_tab(QString,
                                         std::vector<CDBrowser::CtPathElt>)),
            this, SLOT(onBrowseInNewTab(QString, 
                                        std::vector<CDBrowser::CtPathElt>)));

    disconnect(cdb,
               SIGNAL(sig_tracks_to_randplay(RandPlayer::PlayMode,
                               const std::vector<UPnPClient::UPnPDirObject>&)),
               0, 0);
    connect(cdb,
            SIGNAL(sig_tracks_to_randplay(RandPlayer::PlayMode,
                                const std::vector<UPnPClient::UPnPDirObject>&)),
            this, SLOT(onEnterRandPlay(RandPlayer::PlayMode,
                              const std::vector<UPnPClient::UPnPDirObject>&)));

    disconnect(cdb, SIGNAL(sig_rand_stop()), 0, 0);
    connect(cdb, SIGNAL(sig_rand_stop()), this, SLOT(onRandStop()));
    disconnect(cdb, SIGNAL(sig_sort_order()), 0, 0);
    connect(cdb, SIGNAL(sig_sort_order()), this, SIGNAL(sig_sort_order()));
}

void DirBrowser::onEnterRandPlay(RandPlayer::PlayMode mode, const
                                 std::vector<UPnPClient::UPnPDirObject>& ents)
{
    if (!m_pl)
        return;
    qDebug() << "DirBrowser::onEnterRandPlay: " << ents.size() << " tracks";
    if (m_randplayer) {
        delete m_randplayer;
        m_randplayer = 0;
    }
    if ((m_randplayer = new RandPlayer(mode, ents, this)) == 0) {
        qDebug() << "Out of memory";
        return;
    }
    connect(m_randplayer, SIGNAL(sig_tracks_to_playlist(const MetaDataList&)),
            this, SLOT(onRandTracksToPlaylist(const MetaDataList&)));
    connect(m_pl.get(), SIGNAL(sig_playlist_done()),
            m_randplayer, SLOT(playNextSlice()));
    connect(m_randplayer, SIGNAL(sig_randplay_done()),
            this, SLOT(onRandDone()));
    connect(m_randplayer, SIGNAL(sig_next_group_html(QString)),
            this, SIGNAL(sig_next_group_html(QString)));
    m_randplayer->playNextSlice();
}

void DirBrowser::onRandTracksToPlaylist(const MetaDataList& mdl)
{
    if (!m_pl)
        return;

    // We'll get a playlist_done when the old list is replaced, don't
    // want this to trigger a new slice... We should distinguish a
    // playlist clear from the ui (which we want to trigger a new
    // slice) from this one, but temporarily disconnecting is just
    // simpler.
    disconnect(m_pl.get(), SIGNAL(sig_playlist_done()),
               m_randplayer, SLOT(playNextSlice()));
    
    Playlist_Mode mode = m_pl->mode();
    bool replace_saved = mode.replace;
    mode.replace = true;
    m_pl->psl_change_mode(mode);

    m_pl->psl_add_tracks(mdl);

    mode.replace = replace_saved;
    m_pl->psl_change_mode(mode);
    // not necessary for upmpdcli but helps mediaplayer
    m_pl->psl_change_track(0);
    m_pl->psl_play();

    connect(m_pl.get(), SIGNAL(sig_playlist_done()),
            m_randplayer, SLOT(playNextSlice()));
}

void DirBrowser::onRandDone()
{
    // Is it a good idea to delete randplay from here as we're
    // connected to its signal ? Could use a QTimer
    onRandStop();
}

void DirBrowser::onRandStop()
{
    delete m_randplayer;
    m_randplayer = 0;
}

void DirBrowser::onBrowseInNewTab(QString UDN,
                                  vector<CDBrowser::CtPathElt> path)
{
    //qDebug() << "onBrowseInNewTab(): " << UDN;
    addTab();
    CDBrowser *cdb = currentBrowser();
    if (cdb) 
        cdb->browseIn(UDN, path);
}
