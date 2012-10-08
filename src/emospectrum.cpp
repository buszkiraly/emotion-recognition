#include "emospectrum.h"
#include "QPainter"
#include <iostream>
#include "QPaintEvent"

using namespace std;

EmoSpectrum::EmoSpectrum(QWidget *parent) : QLabel(parent)
{
    cout<<"EmoSpectrum created"<<endl;
}

EmoSpectrum::~EmoSpectrum()
{

}

void EmoSpectrum::volumes(std::vector<int> volumes){
    spectrumVolumes.clear();
    spectrumVolumes = volumes;
    repaint();
}

void EmoSpectrum::paintEvent(QPaintEvent *event){
    QPainter painter(this);

    painter.setBrush(QBrush(QColor(0, 0, 0, 255),Qt::SolidPattern));
    painter.drawRect(0,0,450,250);

    for(vector<int>::iterator it = spectrumVolumes.begin(); it<spectrumVolumes.end(); ++it){
        int i = std::distance(spectrumVolumes.begin(), it);

        QString str = QString::number(min(*it,100));
        str.append("%");

        painter.setPen(Qt::white);
        painter.drawText(QPoint(10+45*i,200-min(*it,100)*1.75),str);

        switch (i){
        case 0: painter.setBrush(QBrush(QColor(0, 255, 0, min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; // happy
        case 1: painter.setBrush(QBrush(QColor(255, 0, 0, min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; // angry
        case 2: painter.setBrush(QBrush(QColor(136, 136, 136, min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; // contempt
        case 3: painter.setBrush(QBrush(QColor(128, 128, 0, min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; // disgust
        case 4: painter.setBrush(QBrush(QColor(139, 0, 0,min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; // fear
        case 5: painter.setBrush(QBrush(QColor(148, 0, 211, min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; // sadness
        case 6: painter.setBrush(QBrush(QColor(255, 127, 0, min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break; //surprise
        default: painter.setBrush(QBrush(QColor(255, 255, 255,min(abs((int)(*it*2.05+50)),255)),Qt::SolidPattern)); break;
        }

        painter.setPen(Qt::transparent);
        painter.drawRect(10+45*i,215-min(*it,100)*1.75,15,min(*it,100)*1.75);

        switch (i){
        case 0: painter.setBrush(QBrush(QColor(0, 255, 0, 255),Qt::SolidPattern)); break;
        case 1: painter.setBrush(QBrush(QColor(255, 0, 0, 255),Qt::SolidPattern)); break;
        case 2: painter.setBrush(QBrush(QColor(136, 136, 136, 255),Qt::SolidPattern)); break;
        case 3: painter.setBrush(QBrush(QColor(128, 128, 0, 255),Qt::SolidPattern)); break;
        case 4: painter.setBrush(QBrush(QColor(139, 0, 0, 255),Qt::SolidPattern)); break;
        case 5: painter.setBrush(QBrush(QColor(148, 0, 211, 255),Qt::SolidPattern)); break;
        case 6: painter.setBrush(QBrush(QColor(255, 127, 0, 255),Qt::SolidPattern)); break;
        default: painter.setBrush(QBrush(QColor(255, 255, 255, 255),Qt::SolidPattern)); break;
        }

        painter.drawRect(350,40 + i*15 ,10,10);
        painter.setPen(Qt::white);

        switch (i){
        case 0: str = "Mosoly"; break;
        case 1: str = "Harag"; break;
        case 2: str = "Megvetés"; break;
        case 3: str = "Undor"; break;
        case 4: str = "Félelem"; break;
        case 5: str = "Szomorúság"; break;
        case 6: str = "Meglepetés"; break;
        }


        painter.drawText(QPoint(370,50+i*15),str);

    }

}
