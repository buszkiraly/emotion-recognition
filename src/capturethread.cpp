#include "capturethread.h"
#include "iostream"
#include <QTime>
#include <QMutex>
#include <QApplication>
#include <QDateTime>
#include <QTime>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

using namespace std;

extern CvHaarClassifierCascade  *cascade_face;

// Time measurement
extern QTime *captureSignalTime;
extern QTime *toDetectorSignalTime;

CaptureThread::CaptureThread(){

    int fd;
    char name[100];

    string toOpen1 = "/sys/class/video4linux/video";
    int devNumber = 0;
    string toOpen2 = "/name";

    std::stringstream oss;

    oss<<toOpen1<<devNumber<<toOpen2;

    while((fd = open(oss.str().c_str(), O_RDONLY)) != -1){
        cout<<oss.str()<<" device opened"<<endl;


        memset(name,'\0',sizeof(name));
        read(fd,name,sizeof(name));

        cout<<name<<endl;

        close(fd);

        stringstream dev;
        std::string strName = name;
        strName.erase(strName.length()-1,strName.length());
        dev<<"video"<<devNumber<<" "<<strName;

        captureDevices.push_back(dev.str());


        devNumber++;
        oss.str("");
        oss<<toOpen1<<devNumber<<toOpen2;
    }

    cout<<oss.str()<<" device not opened"<<endl;


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
    captureCamLeft = NULL;
    captureCamRight = NULL;

    framePos = 0;

    timer = new QTime();
    state = 0;
    writer = 0;

    stopped = false;
    manuallyStopped = false;
}

void CaptureThread::run(){
    cout<<"Hello from CaptureThread!"<<endl;

    emit capDevs(captureDevices);

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

    cvReleaseCapture( &captureCamLeft );
    cvReleaseCapture( &captureCamRight );
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
    cvReleaseCapture(&captureCamLeft);
    cvReleaseCapture(&captureCamRight);

    if (!captureVid) {
        cout<<"Couldn't grab video file"<<endl;
        return;
    }

    video = true;
    waitForProcessing = true;

    emit grabbingFromVideo();

}

void CaptureThread::changingToCamera(){

    picture = false;
    video = false;
    changeToCamera = false;

    if (captureCamLeft){
        cvReleaseCapture(&captureCamLeft);
    }
    if (captureCamRight){
        cvReleaseCapture(&captureCamRight);
    }

    captureCamLeft = NULL;
    captureCamRight = NULL;


    captureCamLeft = cvCaptureFromCAM( 1 );
    captureCamRight = cvCaptureFromCAM( 2 );

    cvReleaseCapture(&captureVid);
    if (!captureCamLeft){
        cout<<"Coudn't grab camera"<<endl;
        return;
    }
    if (!captureCamRight){
        cout<<"Coudn't grab camera"<<endl;
        return;
    }

    cvSetCaptureProperty(captureCamLeft,CV_CAP_PROP_FRAME_WIDTH,640);
    cvSetCaptureProperty(captureCamLeft,CV_CAP_PROP_FRAME_HEIGHT,480);
    cvSetCaptureProperty(captureCamRight,CV_CAP_PROP_FRAME_WIDTH,640);
    cvSetCaptureProperty(captureCamRight,CV_CAP_PROP_FRAME_HEIGHT,480);

    camera = true;
    waitForProcessing = false;

    emit frameCount(0,0);
    emit grabbingFromCamera();

}

void CaptureThread::changingToPicture(){

    if (captureCamLeft){
        cvReleaseCapture(&captureCamLeft);
    }
    if (captureCamRight){
        cvReleaseCapture(&captureCamRight);
    }

    changeToPicture = false;
    camera = false;
    video = false;
    picture = false;

    frame = NULL;
    frame = cvLoadImage(pictureSourceAddress.toStdString().c_str(),-1);

    waitForProcessing = false;

    emit grabbingPicture();
    emit frameCount(0,0);
    sendImage();
}

void CaptureThread::grabFrame(){
    if (camera){
        frame = NULL;

        seekMutex.lock();
        frame = cvQueryFrame( captureCamLeft );
        frame_right = cvQueryFrame( captureCamRight );
        seekMutex.unlock();

        if (!frame) {
            emit noFrameError("No frame captured!");
            camera = false;
            return;
        }


    }else if (video){
        frame = NULL;
        frame = cvQueryFrame( captureVid );
        if (!frame) {
            changingToVideo();
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
    // Reading captureSignalTime
    cout<<"captureThread signal transition time was: "<<captureSignalTime->elapsed()<<"ms"<<endl;


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

    IplImage *frameToSend = cvCreateImage( cvSize(frame->width , frame->height ), frame->depth, frame->nChannels );
    cvCopy(frame, frameToSend);
    IplImage *frameToSendRight = cvCreateImage( cvSize(frame_right->width , frame_right->height ), frame_right->depth, frame_right->nChannels );
    cvCopy(frame_right, frameToSendRight);
    emit imageCaptured(frameToSend);

    IplImage *frameToSend2 = cvCreateImage( cvSize(frame->width , frame->height ), frame->depth, frame->nChannels );
    cvCopy(frame, frameToSend2);

    // Measuring signal transition time
    toDetectorSignalTime->start();


    emit imageCaptured2(frameToSend2, frameToSendRight);

}

void CaptureThread::selectedDevice(int dev){
    if (-1 == dev) camera = false;
    else {
        this->dev = dev;
        changeToCamera = true;
        stopped = false;
        manuallyStopped = false;
    }
}


