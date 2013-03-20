#include "plot.h"
#include "QPainter"
#include "QPaintEngine"
#include "stdio.h"
#include "iostream"

using namespace std;

Plot::Plot(QWidget *parent) :
    QLabel(parent)
{
    cout<<"Plot created"<<endl;
}

Plot::~Plot()
{

}

void Plot::paintEvent(QPaintEvent* event)
{
  cout<<"painter"<<endl;
  QPainter painter(this);
  painter.fillRect(0,0,450,250,Qt::SolidPattern);

  painter.setPen(Qt::white);
  painter.drawLine(QPoint(0,125),QPoint(450,125));

  painter.setPen(Qt::gray);
  painter.drawLine(QPoint(0,110),QPoint(450,110));
  painter.drawLine(QPoint(0,235),QPoint(450,235));
  painter.drawLine(QPoint(30,0),QPoint(30,250));
  painter.drawText(QPoint(0,235),"0");
  painter.drawText(QPoint(0,110),"0");
  painter.drawText(QPoint(0,12),"100");
  painter.drawText(QPoint(0,137),"100");

  painter.setPen(Qt::red);
  std::vector<QPoint>   QPlotPoints;
  for (std::vector<int>::iterator it = plotPoints.begin(); it < plotPoints.end(); ++it){
      QPlotPoints.push_back(QPoint(QPlotPoints.size(),110-(*it)));
  }
  painter.drawPolyline(QPlotPoints.data(), static_cast<int>(plotPoints.size()));

  painter.setPen(Qt::cyan);
  QPlotPoints.clear();
  for (std::vector<int>::iterator it = plotPoints2.begin(); it < plotPoints2.end(); ++it){
      QPlotPoints.push_back(QPoint(QPlotPoints.size(),235-(*it)));
  }
  painter.drawPolyline(QPlotPoints.data(), static_cast<int>(plotPoints2.size()));
}

void Plot::points(vector<int> points,vector<int> points2)
{
    plotPoints = points;
    plotPoints2 = points2;
    repaint();
}
