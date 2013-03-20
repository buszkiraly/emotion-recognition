#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "iostream"
#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include <detectorthread.h>
#include <controlthread.h>
#include <QTime>
#include "structures.h"
#include "plot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QTime               *det_timer, *cap_timer;
    QString             det_fps_str, cap_fps_str;
    int                 det_framenumber, cap_framenumber;
    int                 det_elapsed, cap_elapsed;
    double              det_fps, cap_fps;

public slots:
    void clicked_A();
    void clicked_B();
    void clicked_C();
    void clicked_1();
    void clicked_2();
    void clicked_3();
    void clicked_4();
    void clicked_5();
    void clicked_6();
    void clicked_Kesz();
    void setDetectedImage(IplImage *img);
    void setCapturedImage(IplImage *img);
    void setProgressBar(int value);
    void blackToggle(bool);
    void clickedWriteShape();
    void clickedCamera();
    void clickedVideo();
    void clickedPicture();
    void smilePercentage(int);
    void clickedStop();
    void frameCount(double,double);
    void framePos();
    void error(char*);
    void disableFrameCounter();
    void enableFrameCounter();
    void incomingPoints(std::vector<int> plotPoints,std::vector<int> plotPoints2);
    void incomingVolumes(std::vector<int>);
    void incomingHeadPoseCoords(double,double);
    void lookedAtSlot(QString);
    void selectedSlot(QString);
    void waitingForSmile(bool);
    void capDevs(std::vector<std::string>);

signals:
    void readyToSmile(QString);
    void blackToggled(bool);
    void writeShape();
    void videoSource(QString);
    void pictureSource(QString);
    void cameraSource();
    void saveHaarIEBM();
    void paramsChanged(params);
    void loadModelA(QString);
    void loadModelB(QString);
    void stopCapture(bool);
    void seek(int);
    void rewind();
    void forward();
    void contrastSizeChanged(int);
    void points(std::vector<int> points,std::vector<int> points2);
    void drawPlot();
    void emoVolumes(std::vector<int>);
    void outGoingHeadPoseCoords(double, double);
    void smileCutOff(int);
    void resetModel();
    void selectionAlgorithm(int);
    void pointsAnnotations(bool);
    void selectedDevice(int);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_rewindButton_clicked();

    void on_forwardButton_clicked();

    void on_contrastSizeA_valueChanged(int arg1);

    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_15_clicked();

    void on_checkBox_clicked();

    void on_checkBox_toggled(bool checked);

    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_2_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;

    void append(const char* );


};

#endif // MAINWINDOW_H

