/*************************************************************************
** Form Initially created with designer, then hand-modified. The ui file may
** still be useful to help produce changes.
****************************************************************************/

#ifndef UI_MDATAWIDGET_H
#define UI_MDATAWIDGET_H

#include <QLocale>
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

QT_BEGIN_NAMESPACE

class Ui_MDataWidget
{
public:
    QHBoxLayout *horizontalLayout_2;
    QLayout *actualLayout;
    QLabel *lab_title;
    QLabel *lab_artist;
    QLabel *lab_album;

    void setupUi(QWidget *MDataWidget, bool horiz)
    {
        if (MDataWidget->objectName().isEmpty())
            MDataWidget->setObjectName(QString::fromUtf8("MDataWidget"));
        MDataWidget->resize(300, 40);
        
        horizontalLayout_2 = new QHBoxLayout(MDataWidget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));

        int titlehmin = 18;
        int otherhmin = titlehmin;
        int titlehmax = titlehmin;
        int otherhmax = titlehmin;
        Qt::Alignment align(Qt::AlignHCenter|Qt::AlignTop);
        if (horiz) {
            actualLayout = new QHBoxLayout();
        } else {
            titlehmin = 50;
            otherhmin = 30;
            titlehmax = 50;
            otherhmax = 30;
            align = Qt::AlignCenter;
            actualLayout = new QVBoxLayout();
        }

        actualLayout->setObjectName(QString::fromUtf8("actualLayout"));

        lab_title = new QLabel(MDataWidget);
        lab_title->setObjectName(QString::fromUtf8("lab_title"));
        lab_title->setMinimumSize(QSize(0, titlehmin));
        lab_title->setMaximumSize(QSize(16777215, titlehmax));
        lab_title->setFrameShape(QFrame::NoFrame);
        lab_title->setFrameShadow(QFrame::Plain);
        lab_title->setInputMethodHints(Qt::ImhNone);
        lab_title->setText(QString::fromUtf8(""));
        lab_title->setTextFormat(Qt::AutoText);
        lab_title->setScaledContents(true);
        lab_title->setAlignment(Qt::AlignCenter);
        lab_title->setWordWrap(true);
        lab_title->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);

        actualLayout->addWidget(lab_title);

        lab_artist = new QLabel(MDataWidget);
        lab_artist->setObjectName(QString::fromUtf8("lab_artist"));
        lab_artist->setMinimumSize(QSize(0, otherhmin));
        lab_artist->setMaximumSize(QSize(16777215, otherhmax));
        lab_artist->setFrameShape(QFrame::NoFrame);
        lab_artist->setFrameShadow(QFrame::Plain);
        lab_artist->setText(QString::fromUtf8(""));
        lab_artist->setTextFormat(Qt::AutoText);
        lab_artist->setAlignment(Qt::AlignCenter);
        lab_artist->setWordWrap(true);
        lab_artist->setOpenExternalLinks(false);

        actualLayout->addWidget(lab_artist);

        lab_album = new QLabel(MDataWidget);
        lab_album->setObjectName(QString::fromUtf8("lab_album"));
        lab_album->setMinimumSize(QSize(0, otherhmin));
        lab_album->setMaximumSize(QSize(16777215, otherhmax));
        lab_album->setText(QString::fromUtf8(""));
        lab_album->setAlignment(Qt::AlignCenter);
        lab_album->setWordWrap(true);

        actualLayout->addWidget(lab_album);


        horizontalLayout_2->addLayout(actualLayout);

        retranslateUi(MDataWidget);

        QMetaObject::connectSlotsByName(MDataWidget);
    } // setupUi

    void retranslateUi(QWidget *MDataWidget)
    {
        Q_UNUSED(MDataWidget);
    } // retranslateUi

};

namespace Ui {
    class MDataWidget: public Ui_MDataWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MDATAWIDGET_H
