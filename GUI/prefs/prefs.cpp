
#include <string>

#include <QSettings>
#include <QDebug>

#include <libupnpp/upnpputils.hxx>

#include "upadapt/upputils.h"
#include "prefs.h"
#include "utils/confgui.h"
#include "sortprefs.h"

using namespace std;
using namespace confgui;

class ConfLinkQS : public confgui::ConfLinkRep {
public:
    ConfLinkQS(const QString& nm)
	: m_nm(nm) {
    }
    virtual ~ConfLinkQS() {
    }

    virtual bool set(const std::string& val) {
        QSettings settings;
        settings.setValue(m_nm, QString::fromUtf8(val.c_str()));
	return true;
    }
    virtual bool get(std::string& val) {
        QSettings settings;
	bool ret = settings.contains(m_nm);
        if (ret) {
            QString qv = settings.value(m_nm).toString();
            val = string((const char*)qv.toUtf8());
        }
	return ret;
    }
private:
    const QString m_nm;
};

class MyConfLinkFactQS : public confgui::ConfLinkFact {
public:
    MyConfLinkFactQS() {
    }
    virtual ConfLink operator()(const QString& nm) {
        ConfLinkRep *lnk = new ConfLinkQS(nm);
        return ConfLink(lnk);
    }
};

static MyConfLinkFactQS lnkfact;

void UPPrefs::onShowPrefs()
{
    qDebug() << "UPPrefs::onShowPrefs(). m_w: " << (void *)m_w;

    if (m_w == 0) {
        m_w = new ConfTabsW(m_parent, "UPPlay Preferences", &lnkfact);
        int idx = m_w->addPanel("Application");

        // Close to tray ?
        m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "min2tray", "Close to tray",
                   "Minimize to tray instead of exiting when the main window "
                   "is closed");

        // Show notifications ?
        ConfParamW *wsn =
            m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "shownotifications",
                          "Show notifications",
                          "Show pop-up message in notification area when a "
                          "track starts playing");
        QSettings settings;
#ifndef _WIN32
        ConfParamW *wuncmd =
            m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "usenotificationcmd",
                          "Use notification command (else use Qt)",
                          "Choose whether to use the Qt notification mechanism "
                          "or to execute a desktop command");
        if (!settings.contains("notificationcmd")) {
            settings.setValue("notificationcmd",
                              "notify-send --expire-time=1000 --icon=upplay");
        }
        ConfParamW *wncmd =
            m_w->addParam(idx, ConfTabsW::CFPT_STR, "notificationcmd",
                          "Notification command",
                          "Command to execute to notify. The message argument "
                          " will be appended");
        m_w->enableLink(wsn, wuncmd);
        m_w->enableLink(wuncmd, wncmd);
#endif
        
        // Truncate artist information in directory listings?
        ConfParamW *b1 =
            m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "truncateartistindir", 
                          "Truncate artist information in track lists",
                          "Truncate very long artist info so that the table "
                          "does not become weird");

        QString pname("truncateartistlen");
        if (!settings.contains(pname)) {
            settings.setValue(pname, 30);
        }
        ConfParamW *w1 =
            m_w->addParam(idx, ConfTabsW::CFPT_INT, pname, 
                          "Max artist info size in track lists.",
                          "", 0, 100);
        m_w->enableLink(b1, w1);


        // Show some artist information with albums ?
        b1 = m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "showartwithalb", 
                           "Show some artist information in album lists",
                           "List the beginning of artist info when listing "
                           "albums.\nThe exact amount can be adjusted with the "
                           "following entry");
        pname = "artwithalblen";
        if (!settings.contains(pname)) {
            settings.setValue(pname, 15);
        }
        w1 = m_w->addParam(idx, ConfTabsW::CFPT_INT, pname, 
                           "Max artist info size in album lists.",
                           "", 0, 100);
        m_w->enableLink(b1, w1);
        
        m_w->endOfList(idx);
        idx = m_w->addPanel("UPnP");
        vector<string> adapters;
        UPnPP::getAdapterNames(adapters);
        QStringList qadapters;
        qadapters.push_back("");
        for (unsigned int i = 0; i < adapters.size(); i++) {
            qadapters.push_back(u8s2qs(adapters[i]));
        }
        // Specify network interface ?
        m_w->addParam(idx, ConfTabsW::CFPT_CSTR, "netifname",
                      "Network interface name (needs restart)",
                      "Specify network interface to use, or leave blank to "
                      "use the first or only appropriate interface",
                      0, 0, &qadapters);
        
        // Filter out non-openhome renderers?
        m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "ohonly",
                      "Only show OpenHome renderers",
                      "Only show OpenHome-capable renderers in "
                      "the selection dialog. Avoids Bubble UPnP server dups.");

        // Notify content directory updateid changes
        m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "monitorupdateid",
                      "Notify on Content Directory update",
                      "Show dialog when content directory state changes.");

        // 
        m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "noavtsetnext",
                      "Do not use AVTransport gapless functionality",
                      "Disables use of setNextAVTransportUri/Metadata.<br>"
                      "Useful with some buggy AVT renderers. Irrelevant "
                      "with OpenHome.");

        
        idx = m_w->addPanel("Last.FM");

        m_w->addParam(idx, ConfTabsW::CFPT_BOOL, "lastfmscrobble",
                      "Send track information to Last.FM", "");
        m_w->addParam(idx, ConfTabsW::CFPT_STR, "lastfmusername",
                      "Last.FM user name", "");
        m_w->addParam(idx, ConfTabsW::CFPT_STR, "lastfmpassword",
                      "Last.FM password (md5)", "");


        m_w->endOfList(idx);
        idx = m_w->addForeignPanel(new SortprefsW(m_w), "Directory Sorting");
        m_w->endOfList(idx);

        connect(m_w, SIGNAL(sig_prefsChanged()), 
                this, SIGNAL(sig_prefsChanged()));
    }


    m_w->reloadPanels();
    m_w->show();
}

void UPPrefs::onShowPrefs(Tab tb)
{
    onShowPrefs();
    // The enum values and the tab order are currently identical, no
    // need for complication
    m_w->setCurrentIndex(tb);
}
