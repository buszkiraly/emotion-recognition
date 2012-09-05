#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QObject>
#include <QThread>
#include "cv.h"
#include "highgui.h"
#include <mainwindow.h>
#include <QMutex>
#include <QDateTime>
#include <QTime>

class CaptureThread : public QThread
{
    Q_OBJECT

    IplImage                *frame;
    QMutex					seekMutex;
    CvVideoWriter			*writer;
    QDateTime				myQDateTime;
    QTime					*timer;
    int                     key;
    int                     state;
    bool                    stream;
    bool                    saveStream;
    char*                   imageSource;
    bool                    processed;
    bool                    waitForProcessing;
    bool                    stopped, manuallyStopped;
    int                     framePos;
    bool                    changeToVideo, changeToPicture, changeToCamera;
    QString                 videoSourceAddress, pictureSourceAddress;
    bool                    camera, video, picture;
    CvCapture               *captureCam, *captureVid;

public:

    explicit CaptureThread();
    void saveFrame();
    void closeVideo();
    void grabFrame();

private:
    void run();
    void changingToVideo();
    void changingToCamera();
    void changingToPicture();
    void sendImage();

signals:
    void capturedImage(IplImage*);
    void imageCaptured(IplImage*);
    void imageCaptured2(IplImage*);
    void sourceReady(bool);
    void frameCount(double,double);
    void noFrameError(char*);
    void grabbingFromCamera();
    void grabbingFromVideo();
    void grabbingPicture();

public slots:
    void setState(bool faceDetected);
    void imageProcessed();
    void stopCapture(bool);
    void seek(int);
    void videoSource(QString address);
    void cameraSource();
    void pictureSource(QString address);
    void rewind();
    void forward();

};

#endif // CAPTURETHREAD_H
