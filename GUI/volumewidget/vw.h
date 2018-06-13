#ifndef _VW_H_INCLUDED_
#define _VW_H_INCLUDED_
#include <iostream>
#include <QWidget>

class TestReceiver : public QWidget {
Q_OBJECT
public slots:
    void onVolumeChanged(int value) {
        std::cerr << "Volume Changed: " << value << std::endl;
    }
    void onMuteChanged(bool value) {
        std::cerr << "Mute Changed: " << value << std::endl;
    }
};

#endif /* _VW_H_INCLUDED_ */
