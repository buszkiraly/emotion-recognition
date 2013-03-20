#ifndef PLOT_H
#define PLOT_H

#include <QLabel>
#include "QPoint"

class Plot;


class Plot : public QLabel
{
    Q_OBJECT

    std::vector<int> plotPoints;
    std::vector<int> plotPoints2;
    
public:
    explicit Plot(QWidget *parent = 0);
    ~Plot();

public slots:
    void paintEvent(QPaintEvent *event);
    void points(std::vector<int> points,std::vector<int> points2);

    
private:

};

#endif // PLOT_H
