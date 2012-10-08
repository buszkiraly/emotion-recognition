#ifndef HEADPOSEINDICATOR_H
#define HEADPOSEINDICATOR_H

#include <QLabel>

class HeadPoseIndicator : public QLabel
{
    Q_OBJECT
    
    int x,y;

public:
    explicit HeadPoseIndicator(QWidget *parent = 0);
    ~HeadPoseIndicator();

public slots:
    void paintEvent(QPaintEvent* event);
    void coord(double x, double y);
};

#endif // HEADPOSEINDICATOR_H
