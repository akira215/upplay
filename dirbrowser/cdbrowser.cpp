/* Copyright (C) 2005 J.F.Dockes
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

#include <iostream>
#include <functional>
#include <time.h>

#include "libupnpp/config.h"

#include <QUrl>
#ifdef USING_WEBENGINE
// Notes for WebEngine
// - All links must begin with http:// for acceptNavigationRequest to be
//   called. 
// - The links passed to acceptNav.. have the host part 
//   lowercased -> we change S0 to http://h/S0, not http://S0
// As far as I can see, 2016-09, two relatively small issues remaining:
//  - No way to know when page load is done (see mySetHtml())
//  - No way to save/restore the scroll position, all the rest works ok.
 
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QLoggingCategory>
#else
#include <QWebFrame>
#include <QWebSettings>
#include <QWebElement>
#include <QScriptEngine>
#endif

#include <QMessageBox>
#include <QMenu>
#include <QApplication>
#include <QByteArray>
#include <QProgressDialog>

#include "HelperStructs/Helper.h"
#include "HelperStructs/CSettingsStorage.h"

#include "libupnpp/log.hxx"
#include "libupnpp/control/discovery.hxx"

#include "upqo/cdirectory_qo.h"
#include "upadapt/upputils.h"
#include "upadapt/md5.hxx"
#include "cdbrowser.h"
#include "dirbrowser.h"
#include "rreaper.h"


using namespace std;
using namespace std::placeholders;
using namespace UPnPP;
using namespace UPnPClient;

static const string minimFoldersViewPrefix("0$folders");

static void msleep(int millis)
{
    struct timespec spec;
    spec.tv_sec = millis / 1000;
    spec.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&spec, 0);
}

void CDWebPage::javaScriptConsoleMessage(
#ifdef USING_WEBENGINE
    JavaScriptConsoleMessageLevel,
#endif
    const QString& msg, int lineNum, const QString&)
{
    Q_UNUSED(msg);
    Q_UNUSED(lineNum);
    LOGDEB("JAVASCRIPT: "<< qs2utf8s(msg) << " at line " << lineNum << endl);
}

static const QString html_top_orig = QString::fromUtf8(
    "<html><head>"
    "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">"
    );

// The const part + the js we need for webengine
static QString html_top_js;
// The const part + the js + changeable style, computed as needed.
static QString html_top;

CDBrowser::CDBrowser(QWidget* parent)
    : QWEBVIEW(parent), m_reader(0), m_reaper(0), m_progressD(0),
      m_browsers(0), m_lastbutton(Qt::LeftButton), m_sysUpdId(0)
{
    setPage(new CDWebPage(this));
    
#ifdef USING_WEBENGINE
    if (html_top_js.isEmpty()) {
        QLoggingCategory("js").setEnabled(QtDebugMsg, true);
        QString jsfn = Helper::getSharePath() + "/cdbrowser/containerscript.js";
        QString js = "<script type=\"text/javascript\">\n";
        js += QString::fromUtf8(Helper::readFileToByteArray(jsfn));
        js += "</script>\n";
        html_top_js = html_top + js;
    }
#else
    html_top_js = html_top;
    connect(this, SIGNAL(linkClicked(const QUrl &)), 
            this, SLOT(onLinkClicked(const QUrl &)));
    // Not available and not sure that this is needed with webengine ?
    connect(page()->mainFrame(), SIGNAL(contentsSizeChanged(const QSize&)),
            this, SLOT(onContentsSizeChanged(const QSize&)));
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
#endif

    // This sets html_top
    setStyleSheet(CSettingsStorage::getInstance()->getPlayerStyle(), false);
    
    settings()->setAttribute(QWEBSETTINGS::JavascriptEnabled, true);
    if (parent) {
        settings()->setFontSize(QWEBSETTINGS::DefaultFontSize, 
                              parent->font().pointSize()+4);
	settings()->setFontFamily(QWEBSETTINGS::StandardFont, 
			       parent->font().family());
    }
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(createPopupMenu(const QPoint&)));
    m_timer.setSingleShot(1);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(initialPage()));
    m_timer.start(0);
}

CDBrowser::~CDBrowser()
{
    deleteReaders();
}

void CDBrowser::mySetHtml(const QString& html)
{
    // Tried to wait using an event loop, but it seems that the
    // loadFinished() signal is only ever triggered once, even if we
    // replace with a new page ???

    // QEventLoop pause;
    // setPage(new CDWebPage(this));
    // connect(page(), SIGNAL(loadFinished(bool)), &pause, SLOT(quit()));
    setHtml(html);
    msleep(100);
    //pause.exec();
}

void CDBrowser::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "CDBrowser::mouseReleaseEvent";
    m_lastbutton = event->button();
    QWEBVIEW::mouseReleaseEvent(event);
}

// Insert style from on-disk config data to our static constant
// top-of-page. 
void CDBrowser::setStyleSheet(bool dark, bool redisplay)
{
    QString cssfn = Helper::getSharePath() + "/cdbrowser/cdbrowser.css";
    QString cssdata = QString::fromUtf8(Helper::readFileToByteArray(cssfn));

    if (dark) {
        cssfn = Helper::getSharePath() + "/cdbrowser/dark.css";
        cssdata +=  Helper::readFileToByteArray(cssfn);
    } else {
        cssfn = Helper::getSharePath() + "/cdbrowser/standard.css";
    }
    cssdata +=  Helper::readFileToByteArray(cssfn);

    html_top = html_top_js;
    html_top += "<style>\n";
    html_top += cssdata;
    html_top += "</style>\n";
    if (redisplay) {
        if (m_curpath.size()) {
            curpathClicked(m_curpath.size() - 1);
        } else {
            initialPage(true);
        }
    }
}

void CDBrowser::onContentsSizeChanged(const QSize&)
{
    //qDebug() << "CDBrowser::onContentsSizeChanged: scrollpos " <<
    // page()->mainFrame()->scrollPosition();
#ifndef USING_WEBENGINE
    // No way to do this with webengine ?
    page()->mainFrame()->setScrollPosition(m_savedscrollpos);
#endif
}

#ifdef USING_WEBENGINE
static QString base64_encode(QString string)
{
    QByteArray ba;
    ba.append(string);
    return ba.toBase64();
}
#endif

void CDBrowser::appendHtml(const QString& elt_id, const QString& html)
{
    LOGDEB1("CDBrowser::appendHtml: elt_id [" << qs2utf8s(elt_id) <<
           "] html "<< qs2utf8s(html) << endl);

#ifdef USING_WEBENGINE
    // With webengine, we can't access qt object from the page, so we
    // do everything in js. The data is encoded as base64 to avoid
    // quoting issues
    QString js;
    js = "var morehtml = '" + base64_encode(html) + "';\n";
    if (elt_id.isEmpty()) {
        js += QString("document.body.innerHTML += window.atob(morehtml);\n");
    } else {
        js += QString("document.getElementById(\"%1\").innerHTML += "
                     "window.atob(morehtml);\n").arg(elt_id);
    }
    page()->runJavaScript(js, [] (QVariant res) {
            Q_UNUSED(res);
        });
#else
    QWebFrame *mainframe = page()->mainFrame();
    StringObj morehtml(html);

    mainframe->addToJavaScriptWindowObject("morehtml", &morehtml, 
                                           SCRIPTOWNERSHIP);
    QString js;
    if (elt_id.isEmpty()) {
        js = QString("document.body.innerHTML += morehtml.text");
    } else {
        js = QString("document.getElementById(\"%1\").innerHTML += "
                     "morehtml.text").arg(elt_id);
    }
    mainframe->evaluateJavaScript(js);
#endif
}

bool CDBrowser::newCds(int cdsidx)
{
    if (cdsidx > int(m_msdescs.size())) {
        LOGERR("CDBrowser::newCds: bad link index: " << cdsidx 
               << " cd count: " << m_msdescs.size() << endl);
        return false;
    }
    MSRH ms = MSRH(new MediaServer(m_msdescs[cdsidx]));
    if (!ms) {
        qDebug() << "MediaServer connect failed";
        return false;
    }
    CDSH cds = ms->cds();
    if (!cds) {
        LOGERR("CDBrowser::newCds: null cds" << endl);
        return false;
    }
    m_cds = std::shared_ptr<ContentDirectoryQO>(new ContentDirectoryQO(cds));
    m_sysUpdId = 0;
    connect(m_cds.get(), SIGNAL(systemUpdateIDChanged(int)),
            this, SLOT(onSysUpdIdChanged(int)));
    return true;
}

void CDBrowser::onSysUpdIdChanged(int id)
{
    qDebug() << "CDBrowser::onSysUpdIdChanged: mine " << m_sysUpdId <<
        "server" << id;

    if (!QSettings().value("monitorupdateid").toBool()) {
        return;

    }
    // 1st time is free
    if (!m_sysUpdId) {
        m_sysUpdId = id;
        return;
    }

    // We should try to use the containerUpdateIDs to make sure of
    // what needs to be done, instead of dumping the problem on the
    // user.
    if (m_sysUpdId != id) {

        m_sysUpdId = id;

        // CDSKIND_BUBBLE, CDSKIND_MEDIATOMB,
        // CDSKIND_MINIDLNA, CDSKIND_MINIM, CDSKIND_TWONKY
        ContentDirectory::ServiceKind kind = m_cds->srv()->getKind();
        switch (kind) {
            // Not too sure which actually invalidate their
            // tree. Pretty sure that Minim does not (we might just
            // want to reload the current dir).
        case ContentDirectory::CDSKIND_MINIDLNA: break;
        case ContentDirectory::CDSKIND_MEDIATOMB: break;
            // By default, don't do anything by default because some
            // cds keep changing their global updateid. We'd need to
            // check the containerUpdateID.
        case ContentDirectory::CDSKIND_MINIM:
        default: return;
        }

        QMessageBox::Button rep = 
            QMessageBox::question(0, "Upplay",
                                  tr("Content Directory Server state changed, "
                                     "some references may be invalid. (some "
                                     "playlist elements may be invalid too). "
                                     "<br>Reset browse state ?"),
                                  QMessageBox::Ok | QMessageBox::Cancel);
        if (rep == QMessageBox::Ok) {
            curpathClicked(1);
            m_sysUpdId = 0;
        }
    }
}

void CDBrowser::onLinkClicked(const QUrl &url)
{
    m_timer.stop();
    string scurl = qs2utf8s(url.toString());
    qDebug() << "CDBrowser::onLinkClicked: " << url.toString() << 
        " button " <<  m_lastbutton << " mid " << Qt::MidButton;
    // Get rid of http://
    if (scurl.find("http://h/") != 0) {
        qDebug() << "CDBrowser::onLinkClicked: bad link ! : " << url.toString();
        return;
    }
    scurl = scurl.substr(9);
    LOGDEB("CDBrowser::onLinkClicked: corrected url: [" << scurl << "]" <<endl);

    int what = scurl[0];

    switch (what) {

    case 'S':
    {
        // Servers page server link click: browse clicked server root
	unsigned int cdsidx = atoi(scurl.c_str()+1);
        m_curpath.clear();
        m_curpath.push_back(CtPathElt("0", "(servers)"));
        if (!newCds(cdsidx)) {
            return;
        }
        m_cds->srv()->getSearchCapabilities(m_searchcaps);
        emit sig_searchcaps_changed();

	browseContainer("0", m_msdescs[cdsidx].friendlyName);
    }
    break;

    case 'I':
    {
        // Directory listing item link clicked: add to playlist
	unsigned int i = atoi(scurl.c_str()+1);
        if (i > m_entries.size()) {
	    LOGERR("CDBrowser::onLinkClicked: bad objid index: " << i 
                   << " id count: " << m_entries.size() << endl);
	    return;
	}
        MetaDataList mdl;
        mdl.resize(1);
        udirentToMetadata(&m_entries[i], &mdl[0]);
        emit sig_tracks_to_playlist(mdl);
    }
    break;

    case 'C':
    {
        // Directory listing container link clicked: browse subdir.
#ifdef USING_WEBENGINE
#warning tobedone
#else
        m_curpath.back().scrollpos = page()->mainFrame()->scrollPosition();
#endif
	unsigned int i = atoi(scurl.c_str()+1);
        if (i > m_entries.size()) {
	    LOGERR("CDBrowser::onLinkClicked: bad objid index: " << i 
                   << " id count: " << m_entries.size() << endl);
	    return;
	}
        if (m_lastbutton == Qt::MidButton) {
            // Open in new tab
            vector<CtPathElt> npath(m_curpath);
            npath.push_back(CtPathElt(m_entries[i].m_id, m_entries[i].m_title));
            emit sig_browse_in_new_tab(u8s2qs(m_cds->srv()->getDeviceId()),
                                       npath);
        } else {
            browseContainer(m_entries[i].m_id, m_entries[i].m_title);
        }
        return;
    }
    break;

    case 'L':
    {
        // Click in curpath section.
	unsigned int i = atoi(scurl.c_str()+1);
        curpathClicked(i);
        return;
    }
    break;

    default:
        LOGERR("CDBrowser::onLinkClicked: bad link type: " << what << endl);
        return;
    }
}

void CDBrowser::curpathClicked(unsigned int i)
{
    //qDebug() << "CDBrowser::curpathClicked: " << i << " pathsize " << 
    // m_curpath.size();
    if (i >= m_curpath.size()) {
        LOGERR("CDBrowser::curPathClicked: bad curpath index: " << i 
               << " path count: " << m_curpath.size() << endl);
        return;
    }

    if (i == 0) {
        m_curpath.clear();
        m_msdescs.clear();
        m_cds.reset();
        initialPage();
    } else {
        string objid = m_curpath[i].objid;
        string title = m_curpath[i].title;
        string ss = m_curpath[i].searchStr;
        QPoint scrollpos = m_curpath[i].scrollpos;
        m_curpath.erase(m_curpath.begin() + i, m_curpath.end());
        if (m_lastbutton == Qt::MidButton) {
            vector<CtPathElt> npath(m_curpath);
            npath.push_back(CtPathElt(objid, title, ss));
            emit sig_browse_in_new_tab(u8s2qs(m_cds->srv()->getDeviceId()),
                                       npath);
        } else {
            if (ss.empty()) {
                browseContainer(objid, title, scrollpos);
            } else {
                search(objid, ss, scrollpos);
            }
        }
    }
}

void CDBrowser::browseIn(QString UDN, vector<CtPathElt> path)
{
    //qDebug() << "CDBrowser::browsein: " << UDN;

    m_curpath = path;

    if (m_msdescs.size() == 0) {
        // If the servers list is not ready yet, just set m_initUDN to tell
        // initialPage() to do the right thing later when it gets the servers
        // qDebug() << "CDBrowser::browsein: no servers";
        m_initUDN = UDN;
        return;
    } 

    m_lastbutton = Qt::LeftButton;

    // Find UDN in msdescs, create a MediaServer object, and read the
    // specified container (last in input curpath)
    string sudn = qs2utf8s(UDN);
    for (unsigned int i = 0; i < m_msdescs.size(); i++) {
        if (m_msdescs[i].UDN == sudn) {
            newCds(i);
            curpathClicked(path.size() - 1);
            return;
        }
    }
    qDebug() << "CDBrowser::browsein: " << UDN << " not found";
}

static const QString init_container_pagemid = QString::fromUtf8(
#ifdef USING_WEBENGINE
    "</head><body onload=\"addEventListener('contextmenu', saveLoc)\">\n"
#else
    "</head><body>\n"
#endif
    );
static const QString init_container_pagebot = QString::fromUtf8(
    "<table id=\"entrylist\">"
    "<colgroup>"
    "<col class=\"coltracknumber\">"
    "<col class=\"coltitle\">"
    "<col class=\"colartist\">"
    "<col class=\"colalbum\">"
    "<col class=\"colduration\">"
    "</colgroup>"
    "</table>"
    "</body></html>"
    );

void CDBrowser::initContainerHtml(const string& ss)
{
    LOGDEB1("CDBrowser::initContainerHtml\n");
    QString htmlpath("<div id=\"browsepath\"><ul>");
    bool current_is_search = false;
    for (unsigned i = 0; i < m_curpath.size(); i++) {
        QString title = QString::fromUtf8(m_curpath[i].title.c_str());
        QString objid = QString::fromUtf8(m_curpath[i].objid.c_str());
        QString sep("&gt;");
        if (!m_curpath[i].searchStr.empty()) {
            if (current_is_search) {
                // Indicate that searches are not nested by changing
                // the separator
                sep = "&lt;&gt;";
            }
            current_is_search = true;
        }
        if (i == 0)
            sep = "";
        htmlpath += QString("<li class=\"container\" objid=\"%3\">"
                            " %4 <a href=\"http://h/L%1\">%2</a></li>").
            arg(i).arg(title).arg(objid).arg(sep);
    }
    htmlpath += QString("</ul></div><br clear=\"all\"/>");
    if (!ss.empty()) {
        htmlpath += QString("Search results for: ") + 
            QString::fromUtf8(ss.c_str()) + "<br/>";
    }
    QString html = html_top + init_container_pagemid +
        htmlpath + init_container_pagebot;
    LOGDEB1("initContainerHtml: initial content: " << qs2utf8s(html) << endl);
    mySetHtml(html);
    LOGDEB1("CDBrowser::initContainerHtml done\n");
}

// Re-browse (because sort criteria changed probably)
void CDBrowser::refresh()
{
    if (m_curpath.size() >= 1) {
        curpathClicked(m_curpath.size() - 1);
    }
}

void CDBrowser::browseContainer(string ctid, string cttitle, QPoint scrollpos)
{
    LOGDEB("CDBrowser::browseContainer: " << " ctid " << ctid << endl);
    deleteReaders();

    emit sig_now_in(this, QString::fromUtf8(cttitle.c_str()));

    m_savedscrollpos = scrollpos;
    if (!m_cds) {
        LOGERR("CDBrowser::browseContainer: server not set" << endl);
        return;
    }
    m_entries.clear();

    m_curpath.push_back(CtPathElt(ctid, cttitle));

    initContainerHtml();
    
    m_reader = new CDBrowseQO(m_cds->srv(), ctid, string(), this);

    connect(m_reader, SIGNAL(sliceAvailable(UPnPClient::UPnPDirContent*)),
            this, SLOT(onSliceAvailable(UPnPClient::UPnPDirContent *)));
    connect(m_reader, SIGNAL(done(int)), this, SLOT(onBrowseDone(int)));
    m_reader->start();
}

void CDBrowser::search(const string& iss)
{
    search(m_curpath.back().objid, iss);
}

void CDBrowser::search(const string& objid, const string& iss, QPoint scrollpos)
{
    deleteReaders();
    if (iss.empty())
        return;
    if (!m_cds) {
        LOGERR("CDBrowser::search: server not set" << endl);
        return;
    }
    m_savedscrollpos = scrollpos;
    m_entries.clear();
    m_curpath.push_back(CtPathElt(objid, "(search)", iss));
    initContainerHtml(iss);

    m_reader = new CDBrowseQO(m_cds->srv(), m_curpath.back().objid, iss, this);

    connect(m_reader, SIGNAL(sliceAvailable(UPnPClient::UPnPDirContent*)),
            this, SLOT(onSliceAvailable(UPnPClient::UPnPDirContent *)));
    connect(m_reader, SIGNAL(done(int)), this, SLOT(onBrowseDone(int)));
    m_reader->start();
}


static QString CTToHtml(unsigned int idx, const UPnPDirObject& e)
{
    QString out;
    out += QString("<tr class=\"container\" objid=\"%1\" objidx=\"\">"
                   "<td></td><td>").arg(e.m_id.c_str());
    out += QString("<a class=\"ct_title\" href=\"http://h/C%1\">").arg(idx);
    out += QString::fromUtf8(Helper::escapeHtml(e.m_title).c_str());
    out += "</a></td>";
    string val;
    e.getprop("upnp:artist", val);
    QSettings settings;
    if (!val.empty() && settings.value("showartwithalb").toBool()) {
        int maxlen = settings.value("artwithalblen").toInt();
        if (maxlen && int(val.size()) > maxlen) {
            int len = maxlen-3 >= 0 ? maxlen-3 : 0;
            val = val.substr(0,len) + "...";
        }
        out += "<td class=\"ct_artist\">";
        out += QString::fromUtf8(Helper::escapeHtml(val).c_str());
        out += "</td>";
    }
    out += "</tr>";
    return out;
}

/** @arg idx index in entries array
    @arg e array entry
*/
static QString ItemToHtml(unsigned int idx, const UPnPDirObject& e,
                          int maxartlen)
{
    QString out;
    string val;

    out = QString("<tr class=\"item\" objid=\"%1\" objidx=\"%2\">").
        arg(e.m_id.c_str()).arg(idx);

    e.getprop("upnp:originalTrackNumber", val);
    out += QString("<td class=\"tk_tracknum\">");
    if (val.size() == 0) {
        out += "&nbsp;";
    } else {
        out += QString::fromUtf8(Helper::escapeHtml(val).c_str());
    }
    out += "</td>";

    out += "<td class=\"tk_title\">";
    if (e.m_title.size() == 0) {
        out += "&nbsp;";
    } else {
        out += QString("<a href=\"http://h/I%1\">").arg(idx);
        out += QString::fromUtf8(Helper::escapeHtml(e.m_title).c_str());
        out += "</a>";
    }
    out += "</td>";

    val.clear();
    e.getprop("upnp:artist", val);
    out += "<td class=\"tk_artist\">";
    if (maxartlen > 0 && int(val.size()) > maxartlen) {
        int len = maxartlen-3 >= 0 ? maxartlen-3 : 0;
        val = val.substr(0,len) + "...";
    }
    if (val.size() == 0) {
        out += "&nbsp;";
    } else {
        out += QString::fromUtf8(Helper::escapeHtml(val).c_str());
    }
    out += "</td>";
    
    val.clear();
    e.getprop("upnp:album", val);
    out += "<td class=\"tk_album\">";
    out += QString::fromUtf8(Helper::escapeHtml(val).c_str());
    out += "</td>";
    
    val.clear();
    e.getrprop(0, "duration", val);
    int seconds = upnpdurationtos(val);
    // Format as mm:ss
    int mins = seconds / 60;
    int secs = seconds % 60;
    char sdur[100];
    sprintf(sdur, "%02d:%02d", mins, secs);
    out += "<td class=\"tk_duration\">";
    out += sdur;
    out += "</td>";

    out += "</tr>";

    return out;
}

void CDBrowser::deleteReaders() 
{
    //qDebug() << "deleteReaders()";
    if (m_reader) {
        m_reader->setCancel();
        m_reader->wait();
        delete m_reader;
        m_reader = 0;
    }
    if (m_reaper) {
        m_reaper->setCancel();
        m_reaper->wait();
        delete m_reaper;
        m_reaper = 0;
    }
    // Give a chance to queued signals (a thread could have queued
    // something before/while we're cancelling it). Else, typically
    // onSliceAvailable could be called at a later time and mess the
    // display.
    qApp->processEvents();
}

void CDBrowser::onSliceAvailable(UPnPDirContent *dc)
{
    //qDebug() << "CDBrowser::onSliceAvailable";
    if (!m_reader) {
        qDebug() << "CDBrowser::onSliceAvailable: no reader";
        // Cancelled.
        delete dc;
        return;
    }
    QString html;

    m_entries.reserve(m_entries.size() + dc->m_containers.size() + 
                      dc->m_items.size());
    for (std::vector<UPnPDirObject>::iterator it = dc->m_containers.begin();
         it != dc->m_containers.end(); it++) {
        //qDebug() << "Container: " << it->dump().c_str();;
        m_entries.push_back(*it);
        html += CTToHtml(m_entries.size()-1, *it);
    }
    QSettings settings;
    int maxartlen = 0;
    if (settings.value("truncateartistindir").toBool()) {
        maxartlen = settings.value("truncateartistlen").toInt();
    }

    for (std::vector<UPnPDirObject>::iterator it = dc->m_items.begin();
         it != dc->m_items.end(); it++) {
        //qDebug() << "Item: " << it->dump().c_str();;
        m_entries.push_back(*it);
        html += ItemToHtml(m_entries.size()-1, *it, maxartlen);
    }
    appendHtml("entrylist", html);
    delete dc;
    // On very big lists (thousands), this can be quite late against
    // the dir reading threads, with a long queue of slice events
    // accumulated. The UI is frozen while we get called for each
    // event in the queue. We'd like to give a chance to user
    // interaction (e.g. scrolling)here, but it seems that there is no
    // way to process only the user events, so processEvents does not help.
    // Probably the only workable approach would be to do the
    // appending from another thread?
    // (by the way, processEvent if it worked must be called at the
    // bottom of the func else we risk onBrowseDone getting called and
    // deleting the reader before we can use it).
    //qApp->processEvents();
}

class DirObjCmp {
public:
    DirObjCmp(const vector<string>& crits)
        : m_crits(crits) {}
    bool operator()(const UPnPDirObject& o1, const UPnPDirObject& o2) {
        int rel;
        string s1, s2;
        for (unsigned int i = 0; i < m_crits.size(); i++) {
            const string& crit = m_crits[i];
            rel = dirobgetprop(o1, crit, s1).compare(
                dirobgetprop(o2, crit, s2));
            if (rel < 0)
                return true;
            else if (rel > 0)
                return false;
        }
        return false;
    }
    const string& dirobgetprop(const UPnPDirObject& o, const string& nm,
        string& storage) {
        if (!nm.compare("dc:title")) {
            return o.m_title;
        } else if (!nm.compare("uri")) {
            if (o.m_resources.size() == 0)
                return nullstr;
            return o.m_resources[0].m_uri;
        } 
        const string& prop = o.getprop(nm);
        if (!nm.compare("upnp:originalTrackNumber")) {
            char num[30];
            int i = atoi(prop.c_str());
            sprintf(num, "%010d", i);
            storage = num;
            return storage;
        } else if (!nm.compare("upplay:ctpath")) {
            //qDebug() << "TTPATH: " <<
            //    RecursiveReaper::ttpathPrintable(prop).c_str();
            return prop;
        } else {
            return prop;
        }
    }
    vector<string> m_crits;
    static string nullstr;
};
string DirObjCmp::nullstr;

void CDBrowser::onBrowseDone(int)
{
    //qDebug() <<"CDBrowser::onBrowseDone";
    if (!m_reader) {
        qDebug() << "CDBrowser::onBrowseDone(int) no reader: cancelled";
        return;
    }

    vector<string> sortcrits;
    int sortkind = CSettingsStorage::getInstance()->getSortKind();
    if (sortkind == CSettingsStorage::SK_MINIMFNORDER && 
        m_reader->getKind() == ContentDirectory::CDSKIND_MINIM && 
        m_curpath.back().searchStr.empty() &&
        m_reader->getObjid().compare(0, minimFoldersViewPrefix.size(),
                                     minimFoldersViewPrefix) == 0) {
        sortcrits.push_back("uri");
    } else if (sortkind == CSettingsStorage::SK_CUSTOM) {
        QStringList qcrits = CSettingsStorage::getInstance()->getSortCrits();
        for (int i = 0; i < qcrits.size(); i++) {
            sortcrits.push_back(qs2utf8s(qcrits[i]));
        }
    }

    if (!sortcrits.empty()) {
        DirObjCmp cmpo(sortcrits);

        sort(m_entries.begin(), m_entries.end(), cmpo);
        initContainerHtml();
        QString html;
        int maxartlen = 0;
        QSettings settings;
        if (settings.value("truncateartistindir").toBool()) {
            maxartlen = settings.value("truncateartistlen").toInt();
        }
        for (unsigned i = 0; i < m_entries.size(); i++) {
            if (m_entries[i].m_type == UPnPDirObject::container) {
                html += CTToHtml(i, m_entries[i]);
            } else {
                html += ItemToHtml(i, m_entries[i], maxartlen);
            }
        }
        appendHtml("entrylist", html);
    }

    m_reader->wait();
#ifdef USING_WEBENGINE
#warning tobedone
#else
    page()->mainFrame()->setScrollPosition(m_savedscrollpos);
#endif
    deleteReaders();

    //qDebug() << "CDBrowser::onBrowseDone done";
}


static const QString init_server_page_bot = QString::fromUtf8(
    "</head><body>"
    "<h2 id=\"cdstitle\">Content Directory Services</h2>"
    "</body></html>"
    );

static QString DSToHtml(unsigned int idx, const UPnPDeviceDesc& dev)
{
    QString out;
    out += QString("<p class=\"cdserver\" cdsid=\"%1\">").arg(idx);
    out += QString("<a href=\"http://h/S%1\">").arg(idx);
    out += QString::fromUtf8(dev.friendlyName.c_str());
    out += QString("</a></p>");
    return out;
}

void CDBrowser::initialPage(bool redisplay)
{
    deleteReaders();
    emit sig_now_in(this, tr("Servers"));

    vector<UPnPDeviceDesc> msdescs;
    int secs = UPnPDeviceDirectory::getTheDir()->getRemainingDelay();
    if (secs > 1) {
        mySetHtml(html_top + init_server_page_bot);
        appendHtml("", QString::fromUtf8("</head><body><p>Looking for "
                                         "servers...</p></body></html>"));
        qDebug() << "CDBrowser::initialPage: waiting " << secs;
        m_timer.start(secs * 1000);
        return;
    }
    m_searchcaps.clear();
    emit sig_searchcaps_changed();
    if (!MediaServer::getDeviceDescs(msdescs)) {
        LOGERR("CDBrowser::initialPage: getDeviceDescs failed" << endl);
        return;
    }
    //qDebug() << "CDBrowser::initialPage: " << msdescs.size() << " servers";
    
    bool same = msdescs.size() == m_msdescs.size();
    if (same) {
        for (unsigned i = 0; i < msdescs.size(); i++) {
            if (msdescs[i].UDN.compare(m_msdescs[i].UDN)) {
                same = false;
                break;
            }
        }
    }
    if (same && !redisplay) {
        //LOGDEB("CDBrowser::initialPage: no change" << endl);
        m_timer.start(5000);
        return;
    }

    m_msdescs = msdescs;

    if (!m_initUDN.isEmpty()) {
        // "Browse in new tab": show appropriate container
        QString s = m_initUDN;
        m_initUDN = "";
        browseIn(s, m_curpath);
    } else {
        // Show servers list
        QString html = html_top + init_server_page_bot;
        mySetHtml(html);
        for (unsigned i = 0; i < msdescs.size(); i++) {
            appendHtml("", DSToHtml(i, msdescs[i]));
        }
        m_timer.start(5000);
    }
}

enum PopupMode {
    PUP_ADD,
    PUP_ADD_ALL,
    PUP_ADD_FROMHERE,
    PUP_BACK,
    PUP_OPEN_IN_NEW_TAB,
    PUP_RAND_PLAY_TRACKS,
    PUP_RAND_PLAY_GROUPS,
    PUP_RAND_STOP,
    PUP_SORT_ORDER,
};

void CDBrowser::onLoadFinished(bool)
{
    LOGDEB("CDBrowser::onLoadFinished\n");
}

void CDBrowser::onPopupJsDone(const QVariant &jr)
{
    QString qs(jr.toString());
    LOGDEB("onPopupJsDone: parameter: " << qs2utf8s(qs) << endl);
    QStringList qsl = qs.split("\n", QString::SkipEmptyParts);
    for (int i = 0 ; i < qsl.size(); i++) {
        int eq = qsl[i].indexOf("=");
        if (eq > 0) {
            QString nm = qsl[i].left(eq).trimmed();
            QString value = qsl[i].right(qsl[i].size() - (eq+1)).trimmed();
            if (!nm.compare("objid")) {
                m_popupobjid = qs2utf8s(value);
            } else if (!nm.compare("title")) {
                m_popupobjtitle = qs2utf8s(value);
            } else if (!nm.compare("objidx")) {
                m_popupidx = value.toInt();
            } else if (!nm.compare("otype")) {
                m_popupotype = qs2utf8s(value);
            } else {
                LOGERR("onPopupJsDone: unknown key: " << qs2utf8s(nm) << endl);
            }
        }
    }
    LOGDEB1("onPopupJsDone: class [" << m_popupotype <<
           "] objid [" << m_popupobjid <<
           "] title [" <<  m_popupobjtitle <<
           "] objidx [" << m_popupidx << "]\n");
    doCreatePopupMenu();
}

void CDBrowser::createPopupMenu(const QPoint& pos)
{
    if (!m_browsers || m_browsers->insertActive()) {
        LOGDEB("CDBrowser::createPopupMenu: no popup: insert active\n");
        return;
    }
    LOGDEB("CDBrowser::createPopupMenu\n");
    m_popupobjid = m_popupobjtitle = m_popupotype = "";
    m_popupidx = -1;
    m_popupos = pos;
    
#ifdef USING_WEBENGINE
    Q_UNUSED(pos);
    QString js("window.locDetails;");
    CDWebPage *mypage = dynamic_cast<CDWebPage*>(page());
    QEventLoop pause;
    connect(this, SIGNAL(sig_js_done()), &pause, SLOT(quit()));
    mypage->runJavaScript(js, bind(&CDBrowser::onPopupJsDone, this, _1));
#else
    QWebHitTestResult htr = page()->mainFrame()->hitTestContent(pos);
    if (htr.isNull()) {
        qDebug() << "CDBrowser::createPopupMenu: no popup: no hit";
	return;
    }
    QWebElement el = htr.enclosingBlockElement();
    while (!el.isNull() && !el.hasAttribute("objid"))
	el = el.parent();
    if (!el.isNull()) {
        m_popupobjid = qs2utf8s(el.attribute("objid"));
        m_popupobjtitle = qs2utf8s(el.toPlainText());
        if (el.hasAttribute("objidx"))
            m_popupidx = el.attribute("objidx").toInt();
        else
            m_popupidx = -1;
        m_popupotype = qs2utf8s(el.attribute("class"));
        LOGDEB("Popup: " << " class " << m_popupotype << " objid " << 
               m_popupobjid << endl);
    }
    doCreatePopupMenu();
#endif
}

void CDBrowser::doCreatePopupMenu()
{
    QMenu *popup = new QMenu(this);
    QAction *act;
    QVariant v;
    if (m_curpath.size() != 0) {
        // Back action
        act = new QAction(tr("Back"), this);
        v = QVariant(int(PUP_BACK));
        act->setData(v);
        popup->addAction(act);
    }

    // Click in blank area, or no playlist the only entry is Back
    if (m_popupobjid.empty() || (m_browsers && !m_browsers->have_playlist())) {
        popup->connect(popup, SIGNAL(triggered(QAction *)),
                       this, SLOT(back(QAction *)));
        popup->popup(mapToGlobal(m_popupos));
        return;
    }

    if (m_popupidx == -1 && m_popupotype.compare("container")) {
        // All path elements should be containers !
        qDebug() << "Not container and no objidx??";
        return;
    }

    act = new QAction(tr("Send to playlist"), this);
    v = QVariant(int(PUP_ADD));
    act->setData(v);
    popup->addAction(act);

    if (!m_popupotype.compare("item")) {
        act = new QAction(tr("Send all to playlist"), this);
        v = QVariant(int(PUP_ADD_ALL));
        act->setData(v);
        popup->addAction(act);

        act = new QAction(tr("Send all from here to playlist"), this);
        v = QVariant(int(PUP_ADD_FROMHERE));
        act->setData(v);
        popup->addAction(act);
    }
   
    // Connect to either recursive add or simpleAdd depending on entry type.
    if (!m_popupotype.compare("container")) {
        act = new QAction(tr("Open in new tab"), this);
        v = QVariant(int(PUP_OPEN_IN_NEW_TAB));
        act->setData(v);
        popup->addAction(act);

        act = new QAction(tr("Random play by tracks"), this);
        v = QVariant(int(PUP_RAND_PLAY_TRACKS));
        act->setData(v);
        popup->addAction(act);

        act = new QAction(tr("Random play by groups"), this);
        v = QVariant(int(PUP_RAND_PLAY_GROUPS));
        act->setData(v);
        popup->addAction(act);

        popup->connect(popup, SIGNAL(triggered(QAction *)), this, 
                       SLOT(recursiveAdd(QAction *)));
    } else if (!m_popupotype.compare("item")) {
        popup->connect(popup, SIGNAL(triggered(QAction *)), this, 
                       SLOT(simpleAdd(QAction *)));
    } else {
        // ??
        LOGDEB("CDBrowser::createPopup: obj type neither ct nor it: " <<
               m_popupotype << endl);
        return;
    }

    // Like BACK, RAND_STOP must be processed by both simple and
    // recursiveAdd()
    act = new QAction(tr("Stop random play"), this);
    v = QVariant(int(PUP_RAND_STOP));
    act->setData(v);
    popup->addAction(act);
    act->setEnabled(m_browsers->randPlayActive());

    act = new QAction(tr("Sort order"), this);
    v = QVariant(int(PUP_SORT_ORDER));
    act->setData(v);
    popup->addAction(act);

    popup->popup(mapToGlobal(m_popupos));
}

void CDBrowser::back(QAction *)
{
    if (m_curpath.size() >= 2)
        curpathClicked(m_curpath.size() - 2);
}

bool CDBrowser::popupOther(QAction *act)
{
    m_popupmode = act->data().toInt();
    switch (m_popupmode) {
    case PUP_BACK:
        back(0);
        break;
    case PUP_RAND_STOP:
        emit sig_rand_stop();
        break;
    case PUP_SORT_ORDER:
        emit sig_sort_order();
        break;
    default:
        return false;
    }
    return true;
}

// Add a single track or a section of the current container. This
// maybe triggered by a link click or a popup on an item entry
void CDBrowser::simpleAdd(QAction *act)
{
    //qDebug() << "CDBrowser::simpleAdd";
    if (popupOther(act)) {
        // Not for us
        return;
    }
    if (m_popupidx < 0 || m_popupidx > int(m_entries.size())) {
        LOGERR("CDBrowser::simpleAdd: bad obj index: " << m_popupidx
               << " id count: " << m_entries.size() << endl);
        return;
    }
    MetaDataList mdl;
    unsigned int starti = 0;
    switch (m_popupmode) {
    case PUP_ADD_FROMHERE: 
        starti = m_popupidx;
        /* FALLTHROUGH */
    case PUP_ADD_ALL:
        for (unsigned int i = starti; i < m_entries.size(); i++) {
            if (m_entries[i].m_type == UPnPDirObject::item && 
                m_entries[i].m_iclass == 
                UPnPDirObject::ITC_audioItem_musicTrack) {
                mdl.resize(mdl.size()+1);
                udirentToMetadata(&m_entries[i], &mdl[mdl.size()-1]);
            }
        }
        break;
    default:
        mdl.resize(1);
        udirentToMetadata(&m_entries[m_popupidx], &mdl[0]);
    }

    emit sig_tracks_to_playlist(mdl);
}

// Recursive add. This is triggered popup on a container
void CDBrowser::recursiveAdd(QAction *act)
{
    //qDebug() << "CDBrowser::recursiveAdd";

    deleteReaders();

    if (popupOther(act)) {
        // Not for us
        return;
    }

    if (!m_cds) {
        qDebug() << "CDBrowser::recursiveAdd: server not set" ;
        return;
    }

    if (m_popupmode == PUP_OPEN_IN_NEW_TAB) {
        vector<CtPathElt> npath(m_curpath);
        npath.push_back(CtPathElt(m_popupobjid, m_popupobjtitle));
        emit sig_browse_in_new_tab(u8s2qs(m_cds->srv()->getDeviceId()), npath);
        return;
    }

    if (m_browsers)
        m_browsers->setInsertActive(true);
    ContentDirectory::ServiceKind kind = m_cds->srv()->getKind();
    if (kind == ContentDirectory::CDSKIND_MINIM &&
        m_popupmode != PUP_RAND_PLAY_GROUPS) {
        // Use search() rather than a tree walk for Minim, it is much
        // more efficient, except for rand play albums, where we want
        // to preserve the paths (for discrimination)
        UPnPDirContent dirbuf;
        string ss("upnp:class = \"object.item.audioItem.musicTrack\"");
        int err = m_cds->srv()->search(m_popupobjid, ss, dirbuf);
        if (err != 0) {
            LOGERR("CDBrowser::recursiveAdd: search failed, code " << err);
            return;
        }

        m_recwalkentries = dirbuf.m_items;

        // If we are inside the folders view tree, and the option is
        // set, sort by url (minim sorts by tracknum tag even inside
        // folder view)
        if (CSettingsStorage::getInstance()->getSortKind() == 
            CSettingsStorage::SK_MINIMFNORDER && 
            m_popupobjid.compare(0, minimFoldersViewPrefix.size(),
                                 minimFoldersViewPrefix) == 0) {
            vector<string> crits;
            crits.push_back("uri");
            DirObjCmp cmpo(crits);
            sort(m_recwalkentries.begin(), m_recwalkentries.end(), cmpo);
        }

        rreaperDone(0);

    } else {
        delete m_progressD;
        m_progressD = new QProgressDialog("Exploring Directory          ",
                                          tr("Cancel"), 0, 0, this);
        // can't get setMinimumDuration to work
        m_progressD->setMinimumDuration(2000);
        time(&m_progstart);
        
        m_recwalkentries.clear();
        m_recwalkdedup.clear();
        m_reaper = new RecursiveReaper(m_cds->srv(), m_popupobjid, this);
        connect(m_reaper, 
                SIGNAL(sliceAvailable(UPnPClient::UPnPDirContent *)),
                this, 
                SLOT(onReaperSliceAvailable(UPnPClient::UPnPDirContent *)));
        connect(m_reaper, SIGNAL(done(int)), this, SLOT(rreaperDone(int)));
        m_reaper->start();
    }
}

void CDBrowser::onReaperSliceAvailable(UPnPClient::UPnPDirContent *dc)
{
    qDebug() << "CDBrowser::onReaperSliceAvailable: got " << 
        dc->m_items.size() << " items";
    if (!m_reaper) {
        qDebug() << "CDBrowser::onReaperSliceAvailable: cancelled";
        delete dc;
        return;
    }
    for (unsigned int i = 0; i < dc->m_items.size(); i++) {
        if (dc->m_items[i].m_resources.empty()) {
            LOGDEB("CDBrowser::onReaperSlice: entry has no resources??"<< endl);
            continue;
        }
        //LOGDEB1("CDBrowser::onReaperSlice: uri: " << 
        //       dc->m_items[i].m_resources[0].m_uri << endl);
        string md5;
        MD5String(dc->m_items[i].m_resources[0].m_uri, md5);
        pair<std::unordered_set<std::string>::iterator, bool> res = 
            m_recwalkdedup.insert(md5);
        if (res.second) {
            m_recwalkentries.push_back(dc->m_items[i]);
        } else {
            LOGDEB("CDBrowser::onReaperSlice: dup found" << endl);
        }
    }
    delete dc;
    if (m_progressD) {
        if (m_progressD->wasCanceled()) {
            delete m_progressD;
            m_progressD = 0;
            m_reaper->setCancel();
        } else {
            if (time(0) - m_progstart >= 1) {
                m_progressD->show();
            }
            m_progressD->setLabelText(
                tr("Exploring Directory: %1 tracks").
                arg(m_recwalkentries.size()));
        }
    }
}

void CDBrowser::rreaperDone(int status)
{
    LOGDEB("CDBrowser::rreaperDone: status: " << status << ". Entries: " <<
           m_recwalkentries.size() << endl);
    deleteReaders();
    delete m_progressD;
    m_progressD = 0;
    if (m_popupmode == PUP_RAND_PLAY_TRACKS ||
        m_popupmode == PUP_RAND_PLAY_GROUPS) {
        if (m_browsers)
            m_browsers->setInsertActive(false);
        // Sort list
        if (m_popupmode == PUP_RAND_PLAY_GROUPS) {
            vector<string> sortcrits;
            sortcrits.push_back("upplay:ctpath");
            sortcrits.push_back("upnp:album");
            sortcrits.push_back("upnp:originalTrackNumber");
            DirObjCmp cmpo(sortcrits);
            sort(m_recwalkentries.begin(), m_recwalkentries.end(), cmpo);
        }
        RandPlayer::PlayMode mode = m_popupmode == PUP_RAND_PLAY_TRACKS ?
            RandPlayer::PM_TRACKS : RandPlayer::PM_GROUPS;
        LOGDEB("CDBrowser::rreaperDone: emitting sig_tracks_to_randplay, mode "
               << mode << endl);
        emit sig_tracks_to_randplay(mode, m_recwalkentries);
    } else {
        int sortkind = CSettingsStorage::getInstance()->getSortKind();
        if (sortkind == CSettingsStorage::SK_CUSTOM) {
            QStringList qcrits =
                CSettingsStorage::getInstance()->getSortCrits();
            vector<string> sortcrits;
            for (int i = 0; i < qcrits.size(); i++) {
                sortcrits.push_back(qs2utf8s(qcrits[i]));
            }
            DirObjCmp cmpo(sortcrits);
            sort(m_recwalkentries.begin(), m_recwalkentries.end(), cmpo);
        }
        MetaDataList mdl;
        mdl.resize(m_recwalkentries.size());
        for (unsigned int i = 0; i <  m_recwalkentries.size(); i++) {
            udirentToMetadata(&m_recwalkentries[i], &mdl[i]);
        }
        emit sig_tracks_to_playlist(mdl);
    }
}
