#include "headposeindicator.h"
#include "QPainter"
#include "iostream"

using namespace std;

HeadPoseIndicator::HeadPoseIndicator(QWidget *parent) :
    QLabel(parent)
{

}

HeadPoseIndicator::~HeadPoseIndicator()
{

}

void HeadPoseIndicator::paintEvent(QPaintEvent *event){
    QPainter p(this);
    p.fillRect(0,0,200,250,Qt::SolidPattern);

    QPen pen(Qt::red, 2);
    p.setPen(pen);
    QBrush brush(Qt::red);
    p.setBrush(brush);
    p.drawEllipse(x, y, 10, 10);

}

void HeadPoseIndicator::coord(double x, double y)
{
    this->x = 100-200*x;
    this->y = -250*y;

    repaint();
}
