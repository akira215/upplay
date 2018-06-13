#include "sortprefs.h"

#include <map>
#include <string>
#include "HelperStructs/Helper.h"
#include "HelperStructs/CSettingsStorage.h"
#include "upadapt/upputils.h"

using namespace std;

static map<string, string>  allSortCrits;
static map<string, string>  allSortCritsRev;

void SortprefsW::loadValues()
{
    qDebug() << "SortprefsW::loadValues()";

    if (allSortCrits.empty()) {
        allSortCrits["Track Number"] = "upnp:originalTrackNumber";
        allSortCrits["Track Title"] = "dc:title";
        allSortCrits["Date"] = "dc:date";
        allSortCrits["Artist"] = "upnp:artist";
        allSortCrits["Album Title"] = "upnp:album";
        allSortCrits["URI"] = "uri";

        for (map<string, string>::iterator it = allSortCrits.begin(); 
             it != allSortCrits.end(); it++) {
            allSortCritsRev[it->second] = it->first;
        }
    }

    QStringList qcrits = CSettingsStorage::getInstance()->getSortCrits();
    vector<string> crits;
    if (qcrits.size() == 0) {
        qcrits.push_back("upnp:artist");
        qcrits.push_back("upnp:album");
        qcrits.push_back("upnp:originalTrackNumber");
        qcrits.push_back("dc:title");
        qcrits.push_back("dc:date");
        qcrits.push_back("uri");
    }

    for (int i = 0; i < qcrits.size(); i++) {
        string nm = allSortCritsRev[qs2utf8s(qcrits[i])];
        if (nm == "") {
            // Bummer. Limp along and hope for the best
            nm = qs2utf8s(qcrits[i]);
        }
        crits.push_back(nm);
    }

    critsLW->clear();
    for (unsigned int i = 0; i < crits.size(); ++i)
        critsLW->addItem(new QListWidgetItem(crits[i].c_str()));

    int sortkind = CSettingsStorage::getInstance()->getSortKind();

    switch (sortkind) {
    case CSettingsStorage::SK_NOSORT:
    default:
        noSortRB->setChecked(true);
        break;
    case CSettingsStorage::SK_MINIMFNORDER:
        minimfnRB->setChecked(true);
        break;
    case CSettingsStorage::SK_CUSTOM:
        sortRB->setChecked(true);
        break;
    }
}

void SortprefsW::storeValues()
{
    qDebug() << "SortprefsW::storeValues()";

    int sortkind = CSettingsStorage::SK_NOSORT;
    if (minimfnRB->isChecked()) {
        sortkind = CSettingsStorage::SK_MINIMFNORDER;
    } else if (sortRB->isChecked()) {
        sortkind = CSettingsStorage::SK_CUSTOM;
    }
    CSettingsStorage::getInstance()->setSortKind(sortkind);
    QStringList qcrits;
    for (int i = 0; i < critsLW->count(); i++) {
        QString val = 
            critsLW->item(i)->data(Qt::DisplayRole).toString();
        //qDebug() << "Sort nm: " << val;
        val = u8s2qs(allSortCrits[qs2utf8s(val)]);
        if (val != "") {
            qcrits += val;
            //qDebug() << "Sort crit: " << val;
        }
    }
    CSettingsStorage::getInstance()->setSortCrits(qcrits);
}

