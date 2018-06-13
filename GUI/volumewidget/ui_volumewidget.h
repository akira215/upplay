/*****************************************************************************
** Form generated from reading UI file 'volumewidget.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** Then hand-modified to allow easy choosing horizontal/vertical
*****************************************************************************/

#ifndef UI_VOLUMEWIDGET_H
#define UI_VOLUMEWIDGET_H

#include <qglobal.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#define QStringLiteral(X) X
#endif

#include "GUI/widgets/directslider.h"
#include "GUI/volumewidget/soundslider.h"

QT_BEGIN_NAMESPACE
static const char *qt_slider_colours =
    "153;210;153;20;210;20;255;199;15;245;39;29";

class Ui_VolumeWidget
{
public:
    QVBoxLayout *topLayout;
    QBoxLayout *actualLayout;
    QPushButton *btn_mute;
    QAbstractSlider *volumeSlider;
    
    void setupUi(QWidget *VolumeWidget, bool horiz = false)
    {
        Qt::AlignmentFlag alignment;
        Qt::Orientation sliderorient;
        if (horiz) {
            actualLayout = new QHBoxLayout();
            alignment = Qt::AlignVCenter;
            sliderorient = Qt::Horizontal;
        } else {
            actualLayout = new QVBoxLayout();
            alignment = Qt::AlignHCenter;
            sliderorient = Qt::Vertical;
        }        

        if (VolumeWidget->objectName().isEmpty())
            VolumeWidget->setObjectName(QStringLiteral("VolumeWidget"));
        VolumeWidget->resize(94, 300);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(VolumeWidget->sizePolicy().hasHeightForWidth());
        VolumeWidget->setSizePolicy(sizePolicy);
        topLayout = new QVBoxLayout(VolumeWidget);
        topLayout->setSpacing(0);
        topLayout->setContentsMargins(0, 0, 0, 0);
        topLayout->setObjectName(QStringLiteral("topLayout"));

        actualLayout->setSpacing(0);
        actualLayout->setObjectName(QStringLiteral("actualLayout"));

        btn_mute = new QPushButton(VolumeWidget);
        btn_mute->setObjectName(QStringLiteral("btn_mute"));
        btn_mute->setMaximumSize(QSize(28, 30));
        btn_mute->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addFile(Helper::getIconPath("vol_mute.png"), QSize(),
                     QIcon::Normal, QIcon::Off);
        btn_mute->setIcon(icon);
        btn_mute->setIconSize(QSize(24, 24));
        btn_mute->setFlat(true);

        actualLayout->addWidget(btn_mute, 0, alignment);

        if (horiz) {
            // Use the vlc widget
            volumeSlider = new SoundSlider(VolumeWidget,
                                           12.8, strdup(qt_slider_colours));
            // But this alsow works with a normal slider
            //volumeSlider = new DirectSlider(VolumeWidget);
        } else {
            volumeSlider = new DirectSlider(VolumeWidget);
        }

        volumeSlider->setObjectName(QStringLiteral("volumeSlider"));
        volumeSlider->setFocusPolicy(Qt::NoFocus);
        volumeSlider->setMaximum(100);
        volumeSlider->setValue(50);
        volumeSlider->setTracking(true);
        volumeSlider->setInvertedAppearance(false);
        volumeSlider->setInvertedControls(false);
        volumeSlider->setOrientation(sliderorient);

        actualLayout->addWidget(volumeSlider, 1, alignment);


        topLayout->addLayout(actualLayout);


        retranslateUi(VolumeWidget);

        QMetaObject::connectSlotsByName(VolumeWidget);
    } // setupUi

    void retranslateUi(QWidget *VolumeWidget)
    {
        btn_mute->setText(QString());
        Q_UNUSED(VolumeWidget);
    } // retranslateUi

};

namespace Ui {
    class VolumeWidget: public Ui_VolumeWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VOLUMEWIDGET_H
