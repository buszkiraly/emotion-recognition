#include "capturethread.h"
#include "iostream"
#include <QTime>
#include <QMutex>
#include <QtGui/QApplication>
#include <QDateTime>
#include <QTime>

using namespace std;

extern CvHaarClassifierCascade  *cascade_face;

CaptureThread::CaptureThread(){
    frame = NULL;
    stream = NULL;
    saveStream = NULL;

    waitForProcessing = false;
    processed = false;

    changeToVideo = false;
    changeToPicture = false;
    changeToCamera = false;
    camera = false;
    video = false;
    picture = false;

    captureVid = NULL;
    captureCam = NULL;

    framePos = 0;

    timer = new QTime();
    state = 0;
    writer = 0;

    stopped = false;
    manuallyStopped = false;
}

void CaptureThread::run(){
    cout<<"Hello from CaptureThread!"<<endl;

    while(true) {

        if (changeToVideo){
            changingToVideo();
        }

        if (changeToCamera){
            changingToCamera();
        }

        if (changeToPicture){
            changingToPicture();
            if (!frame){
                cout<<"Failed to load picture"<<endl;
                continue;
            }
        }

        if (!video && !camera){
            msleep(5);
            continue;
        }

        grabFrame();

        if (!frame) {
            continue;
        }

        if (video || camera) {
            sendImage();
        }

        if (waitForProcessing){
            stopped = true;
        }

        while (stopped || manuallyStopped){
            msleep(1);
        }

    }

    cvReleaseCapture( &captureCam );
    cvReleaseCapture( &captureVid );
    cvReleaseHaarClassifierCascade( &cascade_face );
}

void CaptureThread::videoSource(QString address){

    cout<<"video"<<endl;

    videoSourceAddress = address;
    changeToVideo = true;

    stopped = false;
    manuallyStopped = false;

}

void CaptureThread::pictureSource(QString address){

    cout<<"picture"<<endl;

    pictureSourceAddress = address;
    changeToPicture = true;

    stopped = false;
    manuallyStopped = false;

}

void CaptureThread::cameraSource(){

    cout<<"camera"<<endl;

    changeToCamera = true;

    stopped = false;
    manuallyStopped = false;

}

void CaptureThread::changingToVideo(){

    changeToVideo = false;
    camera = false;
    picture = false;

    if (captureVid){
        cvReleaseCapture(&captureVid);
    }

    captureVid = NULL;
    captureVid = cvCaptureFromAVI( videoSourceAddress.toStdString().c_str() );
    cvReleaseCapture(&captureCam);

    if (!captureVid) {
        cout<<"Couldn't grab video file"<<endl;
        return;
    }

    video = true;
    waitForProcessing = true;

    emit grabbingFromVideo();

     // várjon a feldolgozásra !!!

}

void CaptureThread::changingToCamera(){

    picture = false;
    video = false;
    changeToCamera = false;

    if (captureCam){
        cvReleaseCapture(&captureCam);
    }

    captureCam = NULL;
    captureCam = cvCaptureFromCAM( CV_CAP_ANY );
    cvReleaseCapture(&captureVid);
    if (!captureCam){
        cout<<"Coudn't grab camera"<<endl;
        return;
    }

    camera = true;
    waitForProcessing = false;

    emit frameCount(0,0);
    emit grabbingFromCamera();

    // ne várjon a feldolgozásra
}

void CaptureThread::changingToPicture(){

    if (captureCam){
        cvReleaseCapture(&captureCam);
    }

    changeToPicture = false;
    camera = false;
    video = false;
    picture = false;

    frame = NULL; //hogy lássam, hogy, ha nem sikerült a grab
    frame = cvLoadImage(pictureSourceAddress.toStdString().c_str(),-1);

    waitForProcessing = false;

    emit grabbingPicture();
    emit frameCount(0,0);
    sendImage();
}

void CaptureThread::grabFrame(){
    if (camera){
        frame = NULL; //hogy lássam, hogy, ha nem sikerült a grab

        seekMutex.lock();
        frame = cvQueryFrame( captureCam );
        seekMutex.unlock();

        if (!frame) {
            emit noFrameError("No frame captured!");
            camera = false;
            return;
        }


    }else if (video){
        frame = NULL; //hogy lássam, hogy, ha nem sikerült a grab
        frame = cvQueryFrame( captureVid );
        if (!frame) {
            changingToVideo(); // újrakezdi a videót, de még kezelni kell, ha tényleg nem tud olvasni egy fájlból
            return;
        }
        emit frameCount(cvGetCaptureProperty(captureVid,CV_CAP_PROP_POS_FRAMES),cvGetCaptureProperty(captureVid,CV_CAP_PROP_FRAME_COUNT));
    }
}

void CaptureThread::saveFrame(){
    if (state == 0){
        timer->start();
    }

    if (state == 1){
        QString DateString = myQDateTime.currentDateTime().toString("yyMMdd_hhmmss");
        DateString.append(".avi");
        QByteArray ba = DateString.toLocal8Bit();

        if (writer) cvReleaseVideoWriter(&writer);
        writer = cvCreateVideoWriter(ba.data(), CV_FOURCC('X','V','I','D'),
                                25,cvSize(frame->width,frame->height),1);

        if (!writer) cout<<"writer is null"<<endl;
        else cout<<"writer opened"<<endl;

        state = 2;
    }

    if (state == 2){
        //cout<<"writing"<<endl;
        cvWriteFrame( writer, frame );
        timer->restart();
    }

    if (state == 3){
        if (timer->elapsed() < 1000) cvWriteFrame( writer, frame );
        else {
            state = 0;
            cvReleaseVideoWriter(&writer);
            cout<<"writer released"<<endl;
        }
    }
}

void CaptureThread::setState(bool faceDetected){

    if (faceDetected){
        if (state == 0) state = 1;
        else if (state == 2) state = 2;
        else if (state == 3) state = 2;
    }
    else{
        if (state == 0) state = 0;
        else if (state == 1) state = 3;
        else if (state == 2) state = 3;
        else if (state == 3) state = 3;
    }
}

void CaptureThread::closeVideo(){
    cout<<"writer released"<<endl;
    cvReleaseVideoWriter(&writer);
}

void CaptureThread::imageProcessed(){
    stopped = false;
}

void CaptureThread::stopCapture(bool stopped){
    this->manuallyStopped = stopped;
}

void CaptureThread::seek(int pos){

    if (video){
        framePos = pos;
        seekMutex.lock();
        cvSetCaptureProperty(captureVid,CV_CAP_PROP_POS_FRAMES, (int)cvGetCaptureProperty(captureVid,CV_CAP_PROP_FRAME_COUNT)/100*pos);
        seekMutex.unlock();
    }
}

void CaptureThread::rewind(){
    if (video){
        int current = cvGetCaptureProperty(captureVid,CV_CAP_PROP_POS_FRAMES);
        cout<<"rewind: "<<current<<endl;
        if (current == 0){
            return;
        }
        cvSetCaptureProperty(captureVid,CV_CAP_PROP_POS_FRAMES, current-2);
        grabFrame();
        sendImage();

    }
}

void CaptureThread::forward(){
    if (video){
        int current = cvGetCaptureProperty(captureVid,CV_CAP_PROP_POS_FRAMES);
        cout<<"rewind: "<<current<<endl;
        if (current == cvGetCaptureProperty(captureVid,CV_CAP_PROP_FRAME_COUNT)){
            return;
        }
        cvSetCaptureProperty(captureVid,CV_CAP_PROP_POS_FRAMES, current);
        grabFrame();
        sendImage();
    }
}

void CaptureThread::sendImage(){

    IplImage *frameToSend = new IplImage;
    frameToSend = cvCreateImage( cvSize(frame->width , frame->height ), frame->depth, frame->nChannels );
    cvCopy(frame, frameToSend);
    emit imageCaptured(frameToSend);

    IplImage *frameToSend2 = new IplImage;
    frameToSend2 = cvCreateImage( cvSize(frame->width , frame->height ), frame->depth, frame->nChannels );
    cvCopy(frame, frameToSend2);
    emit imageCaptured2(frameToSend2);
}


