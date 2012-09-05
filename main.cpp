/*
 *
 *      a Haar-cascade-okat kell beállítani
 *
 *
 *
 *
 *
 *
 */


#include <Qt>
#include <QtGui>
#include <QtGui/QApplication>
#include <QObject>
#include <capturethread.h>
#include <detectorthread.h>
#include <controlthread.h>
#include "mainwindow.h"
#include "iostream"
#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include "structures.h"


using namespace std;

char                    *filename_face = "/home/zoltan/haarcascades/haarcascade_frontalface_alt.xml";
char                    *filename_eye = "/home/zoltan/haarcascades/haarcascade_eye.xml";

CvHaarClassifierCascade *cascade_face, *cascade_eye;

MainWindow              *w;
CaptureThread           *cap;
DetectorThread          *det;
ControlThread           *con;



int main(int argc, char *argv[])
{

    qRegisterMetaType<params>("params");

    cap = new CaptureThread();
    det = new DetectorThread();
    con = new ControlThread();

    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    w = new MainWindow;

    QObject::connect( cap, SIGNAL( imageCaptured(IplImage*) ),
        w, SLOT( setCapturedImage(IplImage*) ));
    QObject::connect( cap, SIGNAL( frameCount(double,double)) ,
        w, SLOT( frameCount(double,double) ));
    QObject::connect( cap, SIGNAL( noFrameError(char*)),
        w, SLOT( error(char*) ), Qt::BlockingQueuedConnection);
    QObject::connect( cap, SIGNAL( grabbingFromCamera()),
                      w, SLOT( disableFrameCounter()));
    QObject::connect( cap, SIGNAL( grabbingFromVideo()),
                      w, SLOT( enableFrameCounter()));
    QObject::connect( cap, SIGNAL( grabbingPicture()),
                      w, SLOT( disableFrameCounter()));
    QObject::connect( cap, SIGNAL( imageCaptured2(IplImage*) ),
        det, SLOT( newCapturedImage(IplImage*)) );


    QObject::connect( det, SIGNAL( imageProcessed(IplImage*) ),
        w, SLOT( setDetectedImage(IplImage*) ), Qt::BlockingQueuedConnection);
    QObject::connect( det, SIGNAL( imageProcessed(IplImage*) ),
        cap, SLOT( imageProcessed()), Qt::BlockingQueuedConnection);
    QObject::connect( det, SIGNAL( smileDetected() ),
        con, SLOT( smileDetected() ), Qt::QueuedConnection);
    QObject::connect( det, SIGNAL( faceDetected(bool) ),
        con, SLOT( faceDetected(bool) ));
    QObject::connect( det, SIGNAL( smilePercentage(int)) ,
        w, SLOT( smilePercentage(int)));
    QObject::connect( det, SIGNAL( initParams(params,params) ) ,
        w, SLOT( initParams(params,params)));


    QObject::connect( con, SIGNAL( progressBarValue(int ) ),
        w, SLOT( setProgressBar(int ) ));
    QObject::connect( con, SIGNAL( recordVideo(bool) ),
        cap, SLOT( setState(bool) ));


    QObject::connect( w, SIGNAL( paramsChanged(params)) ,
        det, SLOT( paramsChanged(params) ));
    QObject::connect( w, SIGNAL( contrastSizeChanged(int)) ,
        det, SLOT( contrastSizeChanged(int) ));
    QObject::connect( w, SIGNAL( loadModelA(QString)) ,
        det, SLOT( loadModelA(QString)));
    QObject::connect( w, SIGNAL( loadModelB(QString)) ,
        det, SLOT( loadModelB(QString)));
    QObject::connect( w, SIGNAL( readyToSmile(QString)) ,
        con, SLOT( readyToSmile(QString) ));
    QObject::connect( w, SIGNAL( blackToggled(bool)) ,
        det, SLOT( setBlack(bool) ));
    QObject::connect( w, SIGNAL( writeShape()) ,
        det, SLOT( writeShape()));
    QObject::connect( w, SIGNAL( videoSource(QString)) ,
        cap, SLOT( videoSource(QString)));
    QObject::connect( w, SIGNAL( pictureSource(QString)) ,
        cap, SLOT( pictureSource(QString)));
    QObject::connect( w, SIGNAL( cameraSource()) ,
        cap, SLOT( cameraSource()));
    QObject::connect( w, SIGNAL( stopCapture(bool)) ,
                      cap, SLOT( stopCapture(bool)));
    QObject::connect( w, SIGNAL( seek(int)) ,
                      cap, SLOT( seek(int)));
    QObject::connect( w, SIGNAL( rewind()) ,
        cap, SLOT( rewind()));
    QObject::connect( w, SIGNAL( forward()) ,
        cap, SLOT( forward()));


    cascade_face = ( CvHaarClassifierCascade* )cvLoad( filename_face, 0, 0, 0 );
    cascade_eye = ( CvHaarClassifierCascade* )cvLoad( filename_eye, 0, 0, 0 );

    w->show();

    cap->start();
    det->start();
    con->start();

    a.exec();

    cap->closeVideo(); // ezt kell tüntetni és minden szálat leállíthatóvá kell tenni

    return 0;
}
