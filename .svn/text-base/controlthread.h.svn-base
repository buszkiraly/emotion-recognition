#ifndef CONTROLTHREAD_H
#define CONTROLTHREAD_H

#include <QObject>
#include <QThread>
#include <QTime>
#include "cv.h"
#include "highgui.h"
#include "QMutex"

class ControlThread : public QThread
{

    Q_OBJECT

    int         value;
    bool        smile;
    bool        ready;
    bool        connected;
    bool        face;
    QTime       *timer;
    QString     toSend;
    bool        machineConnected, machineNeeded;
    QMutex      myMutex;


public:
    ControlThread();
    void run();
    void initMachine();
    void toMachine(QString toSend);

signals:
    void progressBarValue(int );
    void recordVideo(bool);
    void waitForProcessing(bool);
    void loadPicture(QString);

public slots:
    void smileDetected();
    void faceDetected(bool face);
    void readyToSmile(QString qstring);
};

#endif // CONTROLTHREAD_H
