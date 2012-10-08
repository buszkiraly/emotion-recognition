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
    int         algorithm;


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
    void smileValue(int);
    void lookedAt(QString);
    void selected(QString);
    void waitingForSmile(bool);

public slots:
    void smileDetected();
    void faceDetected(bool face);
    void readyToSmile(QString qstring);
    void readyToSmileFromSurprise(double, double);
    void readyToSmileFromHand(double, double);
    void reset();
    void lookedSlot(double, double);
    void selectionAlgorithm(int);
};

#endif // CONTROLTHREAD_H
