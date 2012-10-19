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

/* demolib*//*
#include <fit.h>
#include <DeMoLib_demo.h>

#include <cv.h>
#include <highgui.h>

#include <vcl_iostream.h>
#include <vcl_cstdlib.h>
#include <vcl_cstring.h>
#include <vnl/vnl_math.h>
#include <vil/vil_load.h>
#include <vil/vil_new.h>
#include <vul/vul_arg.h>
#include <vul/vul_printf.h>
#include <vul/vul_sequence_filename_map.h>
#include <vcl_string.h>    // string
#include <vcl_iostream.h>  // cout
#include <vcl_vector.h>    // vector
#include <vcl_algorithm.h> // copy
#include <vcl_iterator.h>  // ostream_iterator

#define JOBB_SZEMSZEL   13
#define BAL_SZEMSZEL    21
#define JOBB_SZAJSZEL   43
#define BAL_SZAJSZEL    39
#define ALL_KEZDET       0
#define ALL_KOZEP        6
#define ALL_VEG         12
#define FELSO_AJAK      41
#define ALSO_AJAK       45


/*demolib end*/

typedef struct{
    float x;
    float y;
}point;

class DetectorThread : public QThread
{
    Q_OBJECT
    CvMemStorage            *storage_face, *storage_hand;
    IplImage                image;
    IplImage                *frame, frameToShow, *previousFrame, *prevPyr, *currPyr;
    int                     detectframes;
    QMutex                  processingMutex, fitMutex;
    //DeMoLib_fit_gui         *guiA, *guiB;
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
    //void enhanceContrast(IplImage* img, int x, int y, int width, int height);
    //void drawAnnotation(IplImage*, vnl_vector<double>, CvScalar);
    //void drawAnnotationLines(IplImage* img, vnl_vector<double> s);
    //double calculateSmile(vnl_vector<double> s);
    bool processFrame();
    float distance(point p1, point p2);
    CvRect detectHands(IplImage *img);

public slots:
    void setBlack(bool);
    void writeShape();
    void newCapturedImage(IplImage*);
    void paramsChanged(params);
    void loadModelA(QString);
    void loadModelB(QString);
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
