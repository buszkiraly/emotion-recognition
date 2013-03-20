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
#include <QApplication>
#include <QObject>
#include <capturethread.h>
#include <detectorthread.h>
#include <controlthread.h>
#include "mainwindow.h"
#include "iostream"
#include "fstream"
#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include "structures.h"
#include "QTime"
#include "QTextCodec"


using namespace std;

const char                    *filename_face = "/home/zoltan/emorec/cascades/haarcascade_frontalface_alt.xml";
const char                    *filename_eye = "/home/zoltan/emorec/haarcascades/haarcascade_eye.xml";
//char                    *filename_hand = "/home/zoltan/HaarTraining/Classifier/hand3/mentett/cascade.xml";
const char                    *filename_hand = "/home/zoltan/emorec/cascades/haarcascade_eye.xml";
CvHaarClassifierCascade *cascade_face, *cascade_eye, *cascade_hand;

MainWindow              *w;
CaptureThread           *cap;
DetectorThread          *det;
ControlThread           *con;

// Time measurement
QTime *signalTime;
QTime *captureSignalTime;
QTime *toDetectorSignalTime;

cv::Mat scale_mtx;

int main(int argc, char *argv[])
{

     scale_mtx.create(132,2,CV_64F);

     ifstream scale_input;
     scale_input.open("/home/zoltan/emorec/models/xy_happy.range");
     if (!scale_input){
         cout << "Can't open scale file. Exiting." << endl;
     }

     string line,first, second;
     scale_input >> line;
     scale_input >> line;
     scale_input >> line;
     for (int i = 0; i < 132; i++){
         scale_input >> line;
         scale_input >> first;
         scale_input >> second;
         scale_mtx.at<double>(i,0) = atof(first.c_str());
         scale_mtx.at<double>(i,1) = atof(second.c_str());
         cout << i << ". " << scale_mtx.at<double>(i,0) << " " << scale_mtx.at<double>(i,1) << endl;
     }

    // Time measurement
    signalTime = new QTime();
    captureSignalTime = new QTime();
    toDetectorSignalTime = new QTime();

    qRegisterMetaType<params>("params");

    cap = new CaptureThread();
    det = new DetectorThread();
    con = new ControlThread();

    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    w = new MainWindow;

    QObject::connect( cap, SIGNAL( imageCaptured(IplImage*) ),
        w, SLOT( setCapturedImage(IplImage*) ));
    QObject::connect( cap, SIGNAL( capDevs(std::vector<std::string>)) ,
        w, SLOT( capDevs(std::vector<std::string>) ), Qt::BlockingQueuedConnection);
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
    QObject::connect( cap, SIGNAL( imageCaptured2(IplImage*,IplImage*) ),
        det, SLOT( newCapturedImage(IplImage*,IplImage*)) );


    QObject::connect( det, SIGNAL( imageProcessed(IplImage*) ),
        w, SLOT( setDetectedImage(IplImage*) ), Qt::BlockingQueuedConnection);
    QObject::connect( det, SIGNAL( imageProcessed(IplImage*) ),
        cap, SLOT( imageProcessed())/*, Qt::BlockingQueuedConnection*/);
    QObject::connect( det, SIGNAL( smileDetected() ),
        con, SLOT( smileDetected() ), Qt::QueuedConnection);
    QObject::connect( det, SIGNAL( faceDetected(bool) ),
        con, SLOT( faceDetected(bool) ));
    QObject::connect( det, SIGNAL( smilePercentage(int)) ,
        w, SLOT( smilePercentage(int)));
    QObject::connect( det, SIGNAL( initParams(params,params) ) ,
        w, SLOT( initParams(params,params)));
    QObject::connect( det, SIGNAL( plotPointsSignal(std::vector<int>,std::vector<int>)) ,
        w, SLOT( incomingPoints(std::vector<int>,std::vector<int>)), Qt::BlockingQueuedConnection);
    QObject::connect( det, SIGNAL( volumes(std::vector<int>)) ,
        w, SLOT( incomingVolumes(std::vector<int>)), Qt::BlockingQueuedConnection);
    QObject::connect( det, SIGNAL( coord(double,double)) ,
        w, SLOT( incomingHeadPoseCoords(double,double)), Qt::BlockingQueuedConnection);
    QObject::connect( det, SIGNAL( coord(double,double)),
        con, SLOT( lookedSlot(double, double) ), Qt::QueuedConnection);
    QObject::connect( det, SIGNAL( surpriseSelected(double,double)) ,
        con, SLOT( readyToSmileFromSurprise(double,double) ));
    QObject::connect( det, SIGNAL( handSelected(double,double)) ,
        con, SLOT( readyToSmileFromHand(double,double) ));


    QObject::connect( con, SIGNAL( progressBarValue(int ) ),
        w, SLOT( setProgressBar(int ) ));
    QObject::connect( con, SIGNAL( selected(QString)) ,
            w, SLOT( selectedSlot(QString)) );
    QObject::connect( con, SIGNAL( lookedAt(QString)) ,
            w, SLOT( lookedAtSlot(QString)) );
    QObject::connect( con, SIGNAL( smileValue(int ) ),
        w, SLOT( smileValue(int ) ));
    QObject::connect( con, SIGNAL( recordVideo(bool) ),
        cap, SLOT( setState(bool) ));
    QObject::connect( con, SIGNAL( waitingForSmile(bool) ),
        w, SLOT( waitingForSmile(bool) ));

    QObject::connect( w, SIGNAL( pointsAnnotations(bool)) ,
        det, SLOT( pointsAnnotations(bool) ));
    QObject::connect( w, SIGNAL( paramsChanged(params)) ,
        det, SLOT( paramsChanged(params) ));
    QObject::connect( w, SIGNAL( smileCutOff(int)) ,
        det, SLOT( smileCutOff(int) ));
    QObject::connect( w, SIGNAL( resetModel()) ,
                      det, SLOT( resetModel() ));
    QObject::connect( w, SIGNAL( contrastSizeChanged(int)) ,
        det, SLOT( contrastSizeChanged(int) ));
    QObject::connect( w, SIGNAL( loadModelA(QString)) ,
        det, SLOT( loadModelA(QString)));
    QObject::connect( w, SIGNAL( loadModelB(QString)) ,
        det, SLOT( loadModelB(QString)));
    QObject::connect( w, SIGNAL( readyToSmile(QString)) ,
        con, SLOT( readyToSmile(QString) ));
    QObject::connect( w, SIGNAL( resetModel()) ,
        con, SLOT( reset() ));
    QObject::connect( w, SIGNAL( selectionAlgorithm(int)) ,
        con, SLOT( selectionAlgorithm(int)));
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
    QObject::connect( w, SIGNAL( selectedDevice(int)) ,
        cap, SLOT( selectedDevice(int)));

/*
    cascade_hand.load( filename_hand );
    cascade_face.load( filename_face );
*/

    cascade_face = ( CvHaarClassifierCascade* )cvLoad( filename_face, 0, 0, 0 );
    cascade_eye = ( CvHaarClassifierCascade* )cvLoad( filename_eye, 0, 0, 0 );
    cascade_hand = ( CvHaarClassifierCascade* )cvLoad( filename_hand, 0, 0, 0 );

    w->show();

    cap->start();
    det->start();
    con->start();

    a.exec();

    cap->closeVideo();


    return 0;
}
