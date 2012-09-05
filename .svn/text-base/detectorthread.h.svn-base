#ifndef DETECTORTHREAD_H
#define DETECTORTHREAD_H

#include <QThread>
#include <cv.h>
#include <highgui.h>
#include <mainwindow.h>
#include <capturethread.h>
#include <QMutex>
#include <fit.h>
#include <QWaitCondition>
#include "structures.h"


class DetectorThread : public QThread
{
    Q_OBJECT
    CvMemStorage            *storage_face;
    IplImage                image;
    IplImage                *frame, *frameToShow;
    int                     detectframes;
    QMutex                  processingMutex, fitMutex;
    DeMoLib_fit_gui         *guiA, *guiB;
    bool                    black, source_Ready;
    bool                    imageReceived;
    bool                    newParameters;
    params                  modelA, modelB;
    int                     contrastSize;
    bool                    processing;
    bool                    waitForImage;

private:
    void sendImage();

public:
    explicit DetectorThread();
    ~DetectorThread();
    void run();
    CvRect detectFaces(IplImage *img);
    void detectSmile(IplImage *img);
    double distance(CvPoint pt1, CvPoint pt2);
    void enhanceContrast(IplImage* img, int x, int y, int width, int height);
    void drawAnnotation(IplImage*, vnl_vector<double>, CvScalar);
    void drawAnnotationLines(IplImage* img, vnl_vector<double> s);
    double calculateSmile(vnl_vector<double> s);
    void processFrame();

public slots:
    void setBlack(bool);
    void writeShape();
    void newCapturedImage(IplImage*);
    void paramsChanged(params);
    void loadModelA(QString);
    void loadModelB(QString);
    void contrastSizeChanged(int);

signals:
    void smileDetected();
    void smilePercentage(int);
    void needImage(int,int);
    void imageProcessed(IplImage*);
    void faceDetected(bool);
    void initParams(params,params);

};

#endif // DETECTORTHREAD_H
