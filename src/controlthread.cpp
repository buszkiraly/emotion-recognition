#include "controlthread.h"
#include <Qt>
#include <QObject>
#include <QThread>
#include <mainwindow.h>
#include <QApplication>
#include "QMutex"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sys/ioctl.h>
#include <termios.h>

#include "automata.h"

#define BUTTONS     1
#define SURPRISE    2
#define HAND        3


using namespace std;


extern CaptureThread    *cap;

ControlThread::ControlThread(){

    cout<<"Hello from ControlThread"<<endl;

    value = 0;
    algorithm = 0;
    smile = false;
    ready = false;
    face  = false;

    timer = new QTime();
    timer->start();

    Automata::Init();
}

void ControlThread::run(){

    int nosmile = 0;


    while (true){

        msleep(1);

        if (timer->elapsed() > 50){

            if (ready){
                if (smile){
                    value+=5;
                    emit smileValue(value);
                }else{
                    nosmile++;
                    if (nosmile == 100){
                        value = 0;
                        nosmile = 0;
                    }
                }
            }

            if (value == 100){
                Automata::getChocolate();
                cout<<"csokit kapsz"<<endl;
                ready = false;
                emit waitingForSmile(ready);
                value = 0;
            }

            timer->start();
            emit progressBarValue(value);

            smile = false;
        }

    }
}

void ControlThread::readyToSmile(QString qstring){
    if (algorithm != BUTTONS) return;

    ready = true;
    emit waitingForSmile(ready);

    Automata::setSelected(qstring);
    emit selected(qstring);
}

void ControlThread::lookedSlot(double linex, double liney){
    Automata::setLookedAt(linex,liney);
    emit lookedAt(Automata::getLookedAt());
}

void ControlThread::readyToSmileFromSurprise(double linex, double liney){
    if (algorithm != SURPRISE) return;

    ready = true;
    emit waitingForSmile(ready);

    Automata::setSelected(linex, liney);

    emit selected(Automata::getSelected());

}

void ControlThread::readyToSmileFromHand(double linex, double liney){
    if (algorithm != HAND) return;

    ready = true;
    emit waitingForSmile(ready);

    Automata::setSelected(linex, liney);

    emit selected(Automata::translateCoord(linex,liney));

}


void ControlThread::smileDetected(){
    smile = true;
}

void ControlThread::faceDetected(bool face){
    this->face = face;
    emit recordVideo(face);
}


void ControlThread::reset(){
    value = 0;
    ready = false;
    emit waitingForSmile(ready);
}

void ControlThread::selectionAlgorithm(int index){
    if (algorithm == index) return;

    ready = false;
    emit waitingForSmile(ready);

    algorithm = index;
    value = 0;
}



