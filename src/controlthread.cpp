#include "controlthread.h"
#include <Qt>
#include <QObject>
#include <QThread>
#include <mainwindow.h>
#include <QtGui/QApplication>
#include "QMutex"

extern "C"{
#include <hid.h>
}


using namespace std;


extern CaptureThread    *cap;

ControlThread::ControlThread(){

    cout<<"Hello from ControlThread"<<endl;

    value = 0;
    smile = false;
    ready = false;
    face  = false;

    machineConnected = false;
    machineNeeded = false;

    timer = new QTime();
    timer->start();

    initMachine();
}

void ControlThread::run(){

    int nosmile = 0;


    while (true){

        msleep(1);

        if (timer->elapsed() > 50){

            if (ready){
                if (smile){
                    value++;
                }else{
                    nosmile++;
                    if (nosmile == 100){
                        value = 0;
                        nosmile = 0;
                    }
                }
            }

            if (value == 10){
                toMachine(toSend);
                cout<<"csokit kapsz"<<endl;
                ready = false;
                value = 0;
            }

            timer->start();
            emit progressBarValue(value);

            smile = false;
        }

    }
}

void ControlThread::readyToSmile(QString qstring){
    ready = true;
    toSend = qstring;
}

void ControlThread::toMachine(QString toSend){

    if (connected){
        char buf[64];

        buf[0] = '9';
        buf[1] = '4';
        buf[2] = toSend.toStdString()[0];
        cout<<buf[2]<<endl;
        buf[3] = toSend.toStdString()[1];
        cout<<buf[3]<<endl;
        buf[4] = '9';
        for (int i=5; i<64; i++) {
            buf[i] = 0;
        }
        //rawhid_send(0, buf, 64, 100);
    }else{
        cout<<toSend.toStdString()[0]<<endl;
    }

}

void ControlThread::smileDetected(){
    smile = true;
}

void ControlThread::faceDetected(bool face){
    this->face = face;
    emit recordVideo(face);
}

void ControlThread::initMachine(){

    // TODO
    // megoldani az automatavezérlést

    /*
    int rawhid;
    rawhid = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);
    if (rawhid <= 0) {
        cout<<"machine not connected"<<endl;
        machineConnected = false;
    }else{
        cout<<"machine connected"<<endl;
        machineConnected = true;
    }
    */
}

