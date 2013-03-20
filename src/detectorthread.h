#ifndef DETECTORTHREAD_H
#define DETECTORTHREAD_H

#include <QThread>
#include <cv.h>
#include <highgui.h>
#include <mainwindow.h>
#include <capturethread.h>
#include <QMutex>
#include <QWaitCondition>
#include "structures.h"
#include "QPoint"
#include <algorithm>    // std::rotate

typedef struct{
    double x;
    double y;
}point;

class DetectorThread : public QThread
{
    Q_OBJECT
    CvMemStorage            *storage_face, *storage_hand;
    IplImage                image;
    IplImage                *frame, *frame_right, frameToShow, *previousFrame, *prevPyr, *currPyr;
    int                     detectframes;
    QMutex                  processingMutex, fitMutex;
    bool                    black, source_Ready;
    bool                    imageReceived;
    bool                    newParameters;
    params                  modelA, modelB;
    int                     contrastSize;
    bool                    processing;
    bool                    waitForImage;
    bool                    featuresCaptured;
    CvPoint2D32f*           points[2];
    std::vector<int>        plotPoints;
    std::vector<int>        plotPoints2;
    std::vector<int>        blinkPoints;
    std::vector<int>        headPointsRaw;
    std::vector<int>        headPointsFiltered;
    std::vector<int>        eyeBrowsRaw;
    std::vector<int>        eyeBrowsFiltered;
    std::vector<int>        roundness_vec;
    std::vector<int>        roundness_vec_filtered;
    int                     cutoff;
    bool                    drawNumbers;


private:
    void sendImage();

public:
    explicit DetectorThread();
    ~DetectorThread();
    void run();
    CvRect detectFaces(IplImage *img);
    void detectSmile(IplImage *img);
    double distance(CvPoint pt1, CvPoint pt2);
    bool processFrame();
    float distance(point p1, point p2);
    CvRect detectHands(IplImage *img);

public slots:
    void setBlack(bool);
    void newCapturedImage(IplImage*,IplImage*);
    void contrastSizeChanged(int);
    void smileCutOff(int);
    void resetModel();
    void pointsAnnotations(bool);

signals:
    void smileDetected();
    void smilePercentage(int);
    void needImage(int,int);
    void imageProcessed(IplImage*);
    void faceDetected(bool);
    void initParams(params,params);
    void plotPointsSignal(std::vector<int> points,std::vector<int> points2);
    void volumes(std::vector<int> spectrumVolumes);
    void coord(double,double);
    void handSelected(double,double);
    void surpriseSelected(double,double);

};

#endif // DETECTORTHREAD_H
