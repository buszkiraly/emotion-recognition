#include <stdio.h>
#include <stdlib.h>
#include "iostream"

#include "automata.h"

extern "C"{
#include <hid.h>
}

#define LOOKEDAT  "0"
#define SELECTED  "1"
#define CHOCOLATE "2"

using namespace std;

QString Automata::lookedAt      = NULL;
QString Automata::selected      = NULL;
bool    Automata::connected     = false;
int     Automata::rawhid        = 0;

Automata::Automata()
{

}

void Automata::Init(){

    if (rawhid) return;

    rawhid = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);
    if (rawhid <= 0) {
        cout<<"machine not connected"<<endl;
        connected = false;
    }else{
        cout<<"machine connected"<<endl;
        connected = true;
    }
}

void Automata::close(){
    if (!rawhid) return;

    rawhid_close(rawhid);
}

bool Automata::setLookedAt(double linex, double liney){

    Automata::lookedAt = translateCoord(linex,liney);

    sendLookedAt();

    return true;
}

QString Automata::getLookedAt(){
    return lookedAt;
}

bool Automata::setSelected(double linex, double liney){

    Automata::selected = translateCoord(linex,liney);

    sendSelected();

    return true;
}

bool Automata::setSelected(QString selected){

    if (selected == NULL) return false;
    if (selected.length() != 2) return false;

    if ( (selected.toStdString()[0] != 'a') && (selected.toStdString()[0] != 'b') && (selected.toStdString()[0] != 'c') ){
        return false;
    }else if ( (selected.toStdString()[1] != '1') && (selected.toStdString()[1] != '2') && (selected.toStdString()[1] != '3') && (selected.toStdString()[1] != '4') && (selected.toStdString()[1] != '5') && (selected.toStdString()[1] != '6')){
        return false;
    }

    Automata::selected = selected;

    sendSelected();

    return true;
}

QString Automata::getSelected(){
    return selected;
}

bool Automata::sendLookedAt(){

    if (lookedAt == NULL) return false;

    return send(QString(LOOKEDAT).append(lookedAt));
}

bool Automata::sendSelected(){

    return send(QString(SELECTED).append(selected));
}

bool Automata::send(QString toSend){
    // Checking frame's length. It has to be 3.
    if (toSend.length() != 3) return false;

    // Trying to send it
    if (connected){
        char buf[64];

        for (int i=0; i<64; i++) {
            buf[i] = 0;
        }

        buf[0] = toSend.toStdString().c_str()[0];
        buf[1] = toSend.toStdString().c_str()[1];
        buf[2] = toSend.toStdString().c_str()[2];

        std::cout<<"Kikuldott byteok szama: "<<rawhid_send(0, buf, 64, 100)<<endl;

    }else{
        return false;
    }

    return true;

}

QString Automata::translateCoord(double linex, double liney){

    // if any of the arguments is outside of [0,1] then return
    if ( (linex<-1) || (linex>1) || (-liney<-1) || (-liney>1) ) return NULL;



    int ix = 50-linex*100;
    int iy = -liney*100;

        ix = ix*6 /100 + 1 ;
        iy = iy*3 /100 ;

        QString translated;

        switch (iy){
            case 0: translated.append("a"); break;
            case 1: translated.append("b"); break;
            case 2: translated.append("c"); break;
        default: return NULL;
        }

        if ( (ix < 1) || (ix > 6) ) return NULL;
        translated.append(QString::number(ix));

        return translated;

}

bool Automata::getChocolate(){
    if (selected == NULL) return false;
    return send(QString(CHOCOLATE).append(selected));
}

Automata::~Automata()
{

}
