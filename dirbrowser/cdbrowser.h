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

#ifndef _CDBROWSER_H_INCLUDED_
#define _CDBROWSER_H_INCLUDED_
#include "libupnpp/config.h"

/** CDBrowser displays Content Directory data inside a DirBrowser Tab */
#include <time.h>

#include <vector>
#include <iostream>
#include <unordered_set>

#ifdef USING_WEBENGINE
#include <QWebEngineView>
#define QWEBVIEW QWebEngineView
#define QWEBPAGE QWebEnginePage
#define QWEBSETTINGS QWebEngineSettings
#else
#include <QWebView>
#define QWEBVIEW QWebView
#define QWEBPAGE QWebPage
#define QWEBSETTINGS QWebSettings
#endif

#include <QVariant>
#include <QTimer>
#include <QPoint>
#include <QMouseEvent>

#include "libupnpp/control/description.hxx"
#include "libupnpp/control/mediaserver.hxx"
#include "libupnpp/control/cdirectory.hxx"

#include "HelperStructs/MetaData.h"
#include "HelperStructs/globals.h"

#include "randplayer.h"

class CDBrowseQO;
class RecursiveReaper;
class DirBrowser;
class QProgressDialog;
class ContentDirectoryQO;

class CDBrowser : public QWEBVIEW
{
    Q_OBJECT;

 public:
    CDBrowser(QWidget* parent = 0);
    virtual ~CDBrowser();
    void setDirBrowser(DirBrowser *db) {m_browsers = db;}
    void getSearchCaps(std::set<std::string>& caps) {caps = m_searchcaps;}
    void search(const std::string& iss);

    // Entry for path inside current server: for going back later
    struct CtPathElt {
        CtPathElt() : scrollpos(0,0) {}
        CtPathElt(const std::string& id, const std::string& tt, 
                  const std::string ss = std::string())
            : objid(id), title(tt), searchStr(ss), scrollpos(0,0) {}
        std::string objid;
        std::string title;
        std::string searchStr;
        QPoint scrollpos;
    };

    // This is called when a new tab is created to browse a
    // container. We have no server link yet.
    void browseIn(QString UDN, vector<CtPathElt> path);

 public slots:
    virtual void initialPage(bool redisplay = false);
    void onBrowseDone(int);
    void onSliceAvailable(UPnPClient::UPnPDirContent *);
    void onReaperSliceAvailable(UPnPClient::UPnPDirContent *);
    void setStyleSheet(bool dark, bool redisplay=true);
    void refresh();
    void onSysUpdIdChanged(int id);
    
 signals:
    void sig_tracks_to_playlist(const MetaDataList&);
    void sig_tracks_to_randplay(RandPlayer::PlayMode,
                                const std::vector<UPnPClient::UPnPDirObject>&);
    void sig_now_in(QWidget *, const QString&);
    void sig_searchcaps_changed();
    void sig_browse_in_new_tab(QString UDN,
                               std::vector<CDBrowser::CtPathElt>);
    void sig_rand_stop();
    void sig_sort_order();

 public slots:
    virtual void appendHtml(const QString&, const QString& html);
    virtual void onLinkClicked(const QUrl &);
    virtual void createPopupMenu(const QPoint&);
    virtual void simpleAdd(QAction *);
    virtual void recursiveAdd(QAction *);
    virtual void back(QAction *);
    virtual void rreaperDone(int);
    virtual void onContentsSizeChanged(const QSize&);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void onPopupJsDone(const QVariant&);
    virtual void onLoadFinished(bool);
    
 private:
    bool popupOther(QAction *act);
    void initContainerHtml(const std::string& ss=string());
    void browseContainer(std::string, std::string, QPoint scrollpos = QPoint());
    void search(const string& objid, const string& iss, QPoint scrollpos = 
                QPoint());
    void curpathClicked(unsigned int i);
    void waitForPage();
    void mySetHtml(const QString&);
    void doCreatePopupMenu();
    
    // When displaying the servers list, we periodically check the
    // server pool state. The last seen Media Server descriptions are
    // stored in m_msdescs for diffing with the new ones and deciding
    // if we need to redraw. Timer and servers list are only used
    // while we are displaying the servers page, the timer is stopped
    // as soon as a link is clicked.
    QTimer m_timer;
    std::vector<UPnPClient::UPnPDeviceDesc> m_msdescs;

    // Handle for the currently active media server
    std::shared_ptr<ContentDirectoryQO>  m_cds;

    // Search caps of current server
    std::set<std::string> m_searchcaps;

    std::vector<CtPathElt> m_curpath;

    // Threaded objects to perform directory reads and recursive walks
    CDBrowseQO *m_reader;
    RecursiveReaper    *m_reaper;
    void deleteReaders();
    bool newCds(int cdsidx);

    // Busy dialog for lengthy ops
    QProgressDialog *m_progressD;
    time_t           m_progstart;
    
    // Content of the currently visited container or search
    std::vector<UPnPClient::UPnPDirObject> m_entries;
    // Scroll position to be restored when we're done reading. This is
    // so that the user can scroll while we insert?
    QPoint m_savedscrollpos;

    // Recursive explore contents, for possible sorting before sending to pl
    std::vector<UPnPClient::UPnPDirObject> m_recwalkentries;
    // URL hashes for deduplication while walking the tree
    std::unordered_set<std::string> m_recwalkdedup;

    // Pointer to parent tabbed object for access to shared state (insertActive)
    DirBrowser *m_browsers;

    // Objid and index in entries for the last popup menu click
    std::string m_popupobjid;
    std::string m_popupobjtitle;
    std::string m_popupotype;
    int m_popupidx;
    QPoint m_popupos;
    int m_popupmode; // now, next, at end

    // Remember last click kind for detecting midclick
    Qt::MouseButton m_lastbutton;

    // State for init browsing in subdir instead of servers page (middle-click)
    QString m_initUDN;

    // SystemUpdateID for the connected content directory server
    int m_sysUpdId;
};

// A QObject to hold a QString. Maybe there would be a simpler way to
// do this but anyway... We use javascript to append html to the
// window content when new directory entries arrive. We could include
// the html in a javascript statement:
//     innerhtml += "html text"
// But then we'd have to escape all the double quotes. Instead we store
// the text inside the QObject text property and make the QObject
// available to javascript. The javascript statement becomes 
//       innerhtml += newtext.text
class StringObj : public QObject {
    Q_OBJECT;
    Q_PROPERTY(QString text READ text);
public:
    StringObj(const QString& s)
        : m_s(s)
        { }
    QString text()
        {
            return m_s;
        }
    QString m_s;
};



#ifdef USING_WEBENGINE

class CDWebPage: public QWebEnginePage {
    Q_OBJECT

public:
    CDWebPage(CDBrowser *parent) 
    : QWebEnginePage((QWidget *)parent), m_browser(parent) {
    }

protected:

    virtual bool acceptNavigationRequest(const QUrl& url, 
                                         NavigationType, 
                                         bool) {
        m_browser->onLinkClicked(url);
        return false;
    }

    virtual void javaScriptConsoleMessage(
        JavaScriptConsoleMessageLevel level,
        const QString& msg, int lineNum, const QString & sourceID);

private:
    CDBrowser *m_browser;
};

#else // Using Qt Webkit

// We only subclass QWebPage in order to display the JS error messages
class CDWebPage: public QWebPage {
    Q_OBJECT;

 public:
    CDWebPage(QWidget* parent = 0) : QWebPage(parent) {
    }
    virtual void javaScriptConsoleMessage(const QString& msg, int lineNum, 
					  const QString & sourceID);
};

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define  SCRIPTOWNERSHIP QWebFrame::ScriptOwnership
#else
#define  SCRIPTOWNERSHIP QScriptEngine::ScriptOwnership
#endif

#endif

#endif /* _CDBROWSER_H_INCLUDED_ */
