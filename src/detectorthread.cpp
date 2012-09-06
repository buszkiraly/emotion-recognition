/*

    Ahol ezeket a sorokat látod:

        vul_arg<const char*> dmFile(0,"Deformable model sequence fname","/home/zoltan/DeMoLib_v1_1_1/model/mentett/test-level_#.aam_di_linear");
        vul_arg<const char*> imFile(0,"Image file to fit in","/home/zoltan/DeMoLib_v1_1_1/data/images/im01.jpg");

    ott meg kell adni a modellt, amit használni akarunk, illetve az init kép elérési útvonalát

  */
#include "detectorthread.h"
#include <capturethread.h>
#include <controlthread.h>
#include <iostream>
#include <QApplication>
#include <QMutex>
#include <QSemaphore>
#include <QWaitCondition>

/* demolib*/
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

using namespace std;

extern CvHaarClassifierCascade  *cascade_face, *cascade_eye;
extern char                     *filename_face;
extern char                     *filename_smile;


CvRect                          *face;


DetectorThread::DetectorThread()
{
    vul_arg<const char*> dmFile(0,"Deformable model sequence fname","/home/zoltan/DeMoLib_v1_1_1/model/mentett/test-level_#.aam_di_linear");
    vul_arg<const char*> imFile(0,"Image file to fit in","/home/zoltan/DeMoLib_v1_1_1/data/images/im01.jpg");
    vul_arg<int>         nLevel("-l","Number of pyramid levels",1);
    vul_arg<int>         filter("-f","Image filter (0=raw,1=greyscale)",1);
    vul_arg<int>         maxIter("-i","Maximum iterations/level",20);
    vul_arg<int>         flag("--flag","Fitting flag",0);
    vul_arg<bool>        ascii("--ascii","Files are in ASCII format",false);
    vul_arg<double>      iterDec("-d","Iteration decrement",1.0);

    //get filenames for deformable models
    vul_sequence_filename_map dm_map(dmFile(),0,nLevel()-1);

    vcl_vector<vcl_string> dmFiles(nLevel());

    for(int i = 0; i < nLevel(); i++){
        dmFiles[i] = dm_map.image_name(i);
    }

    //create gui object

    vcl_string imFname = imFile();


    guiA = new DeMoLib_fit_gui(dmFiles,imFname,filter(),ascii());

    guiA->SetMaxIter(maxIter());
    guiA->SetDecIter(iterDec());
    guiA->SetFitFlag(flag());

    guiB = new DeMoLib_fit_gui(dmFiles,imFname,filter(),ascii());

    guiB->SetMaxIter(maxIter());
    guiB->SetDecIter(iterDec());
    guiB->SetFitFlag(flag());


    frame = NULL;
    frameToShow = NULL;
    black = false;
    source_Ready = false;
    imageReceived = false;
    newParameters = false;
    processing = false;
    waitForImage = true;

    modelA.id              = 0;
    modelA.inUse           = false;
    modelA.xShift          = 0;
    modelA.yShift          = 30;
    modelA.modelScale      = 1;
    modelA.modelRot        = 0;
    modelA.modelIterations = 3;
    modelA.fit             = true;
    modelA.memo            = false;

    modelB.id              = 1;
    modelB.inUse           = false;
    modelB.xShift          = 0;
    modelB.yShift          = 30;
    modelB.modelScale      = 1;
    modelB.modelRot        = 0;
    modelB.modelIterations = 3;
    modelB.fit             = true;
    modelB.memo            = false;

    contrastSize = 40;

}

DetectorThread::~DetectorThread(){
    cvReleaseHaarClassifierCascade( &cascade_face );
}

void DetectorThread::run(){
    cout<<"Hello from DetectorThread!"<<endl;

    emit initParams(modelA,modelB);

    while(true){

        newParameters = false;

        while ((waitForImage) && (!newParameters)) {
            msleep(5);
        }

        newParameters = false;

        processing = true;
        processFrame();
        sendImage();
        processing = false;

    }
}

void DetectorThread::sendImage(){
    if (!frameToShow) return;
    IplImage *frameToSend = cvCreateImage( cvSize(frameToShow->width , frameToShow->height ), frameToShow->depth, frameToShow->nChannels );
    cvCopy(frameToShow, frameToSend);
    cvReleaseImage(&frameToShow);
    waitForImage = true;
    emit imageProcessed(frameToSend);
}

CvRect DetectorThread::detectFaces(IplImage *img){

    if (storage_face){
        cvReleaseMemStorage(&storage_face);
    }
    storage_face = cvCreateMemStorage( 0 );

    if (!storage_face){
        exit(0);
    }

    CvSeq *faces = cvHaarDetectObjects(
            img,
            cascade_face,
            storage_face,
            1.1,
            3,
            0 ,
            cvSize( 60, 60 ) );

    int max = 0, maxwidth = 0;

    for(int i = 0 ; i < ( faces ? faces->total : 0 ) ; i++ ) {
        CvRect *r = ( CvRect* )cvGetSeqElem( faces, i );
        if (r->width > maxwidth){
            maxwidth = r->width;
            max = i;
        }
        /*cvRectangle( img,
                     cvPoint( r->x, r->y ),
                     cvPoint( r->x + r->width, r->y + r->height ),
                     CV_RGB( 255, 0, 0 ), 1, 8, 0 );*/
    }

    CvRect *maxFace = (CvRect*)cvGetSeqElem( faces, max );

    if (faces->total != 0){
        return cvRect(maxFace->x, maxFace->y, maxFace->width, maxFace->height);
    }else{
        return cvRect(0,0,0,0);
    }

}

void DetectorThread::setBlack(bool state){
    black = state;
    newParameters = true;
}

void DetectorThread::writeShape(){
   /* cout<<"writeshape"<<endl;

    QString pts,jpg;
    QDateTime myQDateTime;
    QString DateString = myQDateTime.currentDateTime().toString("yyMMdd_hhmmss");
    jpg = "/home/zoltan/DeMoLib_v1_1_1/data/sajat/" + DateString + ".jpg";
    pts = "/home/zoltan/DeMoLib_v1_1_1/data/sajat/" + DateString + ".pts";
    QByteArray _jpg = jpg.toLocal8Bit();
    QByteArray _pts = pts.toLocal8Bit();

    gui->WriteShape(_pts.data());
    cvSaveImage(_jpg.data(),frameToShow,0);

    /*

        el kell menteni a pts filet is

     */

}

double DetectorThread::distance(CvPoint pt1, CvPoint pt2){
    return sqrt(pow(pt1.x-pt2.x,2) + pow(pt1.y-pt2.y,2));
}

void DetectorThread::newCapturedImage(IplImage* img){
    if (!img) return;
    if (processing) {
        cvReleaseImage(&img);
        return;
    }

    if (frame) cvReleaseImage(&frame);

    frame = cvCreateImage( cvSize(img->width,img->height), img->depth, img->nChannels );
    cvCopy(img, frame);
    cvReleaseImage(&img);

    waitForImage = false;
}

void DetectorThread::enhanceContrast(IplImage* img, int x, int y, int width, int height){

    IplImage *red, *green, *blue;

    red=cvCreateImage( cvGetSize(img), 8, 1 );
    green=cvCreateImage( cvGetSize(img), 8, 1 );
    blue=cvCreateImage( cvGetSize(img), 8, 1 );

    cvSplit(img, blue, green, red, NULL);

        cvSetImageROI(red,cvRect(x,y,width,height));
        cvSetImageROI(green,cvRect(x,y,width,height));
        cvSetImageROI(blue,cvRect(x,y,width,height));

        cvEqualizeHist(red,red);
        cvEqualizeHist(green,green);
        cvEqualizeHist(blue,blue);

        cvResetImageROI(red);
        cvResetImageROI(green);
        cvResetImageROI(blue);

    cvMerge(blue, green, red, NULL, img);

    cvReleaseImage(&red);
    cvReleaseImage(&green);
    cvReleaseImage(&blue);
}

void DetectorThread::drawAnnotation(IplImage* img, vnl_vector<double> s,CvScalar color){

    for (int i = 0; i< 68; i++){
        cvCircle(img,cvPoint( s[2*i], s[2*i+1] ),3,color,-1,8,0);
    }

}

void DetectorThread::drawAnnotationLines(IplImage* img, vnl_vector<double> s){
    cvLine(img, cvPoint( s[2*JOBB_SZEMSZEL], s[2*JOBB_SZEMSZEL+1] ),
           cvPoint( s[2*JOBB_SZAJSZEL], s[2*JOBB_SZAJSZEL+1] ), cvScalar(0, 255, 0, 0), 2, 8, 0);

    cvLine(img, cvPoint( s[2*BAL_SZEMSZEL], s[2*BAL_SZEMSZEL+1] ),
           cvPoint( s[2*BAL_SZAJSZEL], s[2*BAL_SZAJSZEL+1] ), cvScalar(0, 255, 0, 0), 2, 8, 0);

    cvLine(img, cvPoint( s[2*ALL_KEZDET], s[2*ALL_KEZDET+1] ),
           cvPoint( s[2*BAL_SZAJSZEL], s[2*BAL_SZAJSZEL+1] ), cvScalar(0, 255, 0, 0), 2, 8, 0);

    cvLine(img, cvPoint( s[2*JOBB_SZAJSZEL], s[2*JOBB_SZAJSZEL+1] ),
           cvPoint( s[2*ALL_VEG], s[2*ALL_VEG+1] ), cvScalar(0, 255, 0, 0), 2, 8, 0);

    cvLine(img, cvPoint( s[2*FELSO_AJAK], s[2*FELSO_AJAK+1] ),
           cvPoint( s[2*ALSO_AJAK], s[2*ALSO_AJAK+1] ), cvScalar(0, 255, 0, 0), 2, 8, 0);

    cvLine(img, cvPoint( s[2*BAL_SZAJSZEL], s[2*BAL_SZAJSZEL+1] ),
           cvPoint( s[2*JOBB_SZAJSZEL], s[2*JOBB_SZAJSZEL+1] ), cvScalar(0, 255, 0, 0), 2, 8, 0);
}



double DetectorThread::calculateSmile(vnl_vector<double> s){
    double smile = 0;
    double felsoajak_alsoajak = distance(cvPoint( s[2*FELSO_AJAK], s[2*FELSO_AJAK+1] ), cvPoint( s[2*ALSO_AJAK], s[2*ALSO_AJAK+1] ));
    double balszajszel_jobbszajszel = distance(cvPoint( s[2*BAL_SZAJSZEL], s[2*BAL_SZAJSZEL+1] ), cvPoint( s[2*JOBB_SZAJSZEL], s[2*JOBB_SZAJSZEL+1] ));
    double allkezdet_balszajszel = distance(cvPoint( s[2*ALL_KEZDET], s[2*ALL_KEZDET+1] ), cvPoint( s[2*BAL_SZAJSZEL], s[2*BAL_SZAJSZEL+1] ));
    double jobbszajszel_allveg = distance(cvPoint( s[2*JOBB_SZAJSZEL], s[2*JOBB_SZAJSZEL+1] ), cvPoint( s[2*ALL_VEG], s[2*ALL_VEG+1] ) );
    double balszemszel_balszajszel = distance(cvPoint( s[2*BAL_SZEMSZEL], s[2*BAL_SZEMSZEL+1] ), cvPoint( s[2*BAL_SZAJSZEL], s[2*BAL_SZAJSZEL+1] ));
    double jobbszajszel_jobbszemszel = distance(cvPoint( s[2*JOBB_SZAJSZEL], s[2*JOBB_SZAJSZEL+1] ), cvPoint( s[2*JOBB_SZEMSZEL], s[2*JOBB_SZEMSZEL+1] ));
    double balszemszel_jobbszemszel = distance(cvPoint( s[2*BAL_SZEMSZEL], s[2*BAL_SZEMSZEL+1] ), cvPoint( s[2*JOBB_SZEMSZEL], s[2*JOBB_SZEMSZEL+1] ));
    double allkezdet_allveg = distance(cvPoint( s[2*ALL_KEZDET], s[2*ALL_KEZDET+1] ), cvPoint( s[2*ALL_VEG], s[2*ALL_VEG+1] ) );

    smile = 10 *
            felsoajak_alsoajak *
            balszajszel_jobbszajszel /
            allkezdet_balszajszel /
            jobbszajszel_allveg /
            balszemszel_balszajszel /
            jobbszajszel_jobbszemszel *
            balszemszel_jobbszemszel *
            balszemszel_jobbszemszel;

    if ((allkezdet_allveg/balszemszel_jobbszemszel < 1.6) )
      return smile;
    else
      return 0;
}

void DetectorThread::processFrame(){


    if (!frame) {
        cout<<"nincs frame a process fuggvenyben"<<endl;
        return;
    }

    cout<<"process"<<endl;

    IplImage* frameToAAM = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);

    if (frameToShow) cvReleaseImage(&frameToShow);
    frameToShow = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);

    cvCopy(frame, frameToShow);

    CvRect r = detectFaces(frame);

    if (!r.width){

        emit faceDetected(false);
        emit smilePercentage(0);

    }else{

        fitMutex.lock();
        cout<<"process begin"<<endl;

        emit faceDetected(true);

        enhanceContrast(frameToShow, r.x - contrastSize, r.y - contrastSize, r.width + 2*contrastSize, r.height + 2*contrastSize);

        cvCopy( frameToShow, frameToAAM, NULL );

        if (black){
            cvSetZero(frameToShow);
        }

        vnl_vector<double> s;

        if (modelA.inUse){

            if (modelA.memo){
                guiA->LoadImageMemo(frameToAAM,1);
            }else{
                guiA->LoadImage(frameToAAM,1,r.x + r.width/2 + modelA.xShift*r.width/100,r.y + r.height/2 + modelA.yShift*r.height/100, (double)modelA.modelScale*r.width/100, modelA.modelRot);
            }

            if (modelA.fit){
               for (int i = 0; i < modelA.modelIterations; i++) guiA->Fit();
            }

            guiA->__model.GetShape(s,0);
            drawAnnotation(frameToShow,s,cvScalar(255, 0, 0, 0));

        }

        if (modelB.inUse){
            if (modelB.memo){
                guiB->LoadImageMemo(frameToAAM,1);
            }else{
                guiB->LoadImage(frameToAAM,1,r.x + r.width/2 + modelB.xShift*r.width/100,r.y + r.height/2 + modelB.yShift*r.height/100, (double)modelB.modelScale*r.width/100, modelB.modelRot);
            }

            if (modelB.fit){
               for (int i = 0; i < modelB.modelIterations; i++) guiB->Fit();
            }

            guiB->__model.GetShape(s,0);
            drawAnnotation(frameToShow,s,cvScalar(0, 0, 255, 0));

        }

/*
            double smile = 0;

            smile = calculateSmile(s);

            emit smilePercentage((int)smile);
            if (smile > 30) emit smileDetected();
            */

            //drawAnnotationLines(frameToShow,s);
        }

          cvReleaseImage(&frameToAAM);

          cout<<"process end"<<endl;
          fitMutex.unlock();

}

void DetectorThread::paramsChanged(params newParams){
    switch (newParams.id){
            case 0:{
                        modelA.inUse           = newParams.inUse;
                        modelA.xShift          = newParams.xShift;
                        modelA.yShift          = newParams.yShift;
                        modelA.modelScale      = newParams.modelScale;
                        modelA.modelRot        = newParams.modelRot;
                        modelA.modelIterations = newParams.modelIterations;
                        modelA.fit             = newParams.fit;
                        modelA.memo            = newParams.memo;

                        break;
                }
            case 1:{

                        modelB.inUse           = newParams.inUse;
                        modelB.xShift          = newParams.xShift;
                        modelB.yShift          = newParams.yShift;
                        modelB.modelScale      = newParams.modelScale;
                        modelB.modelRot        = newParams.modelRot;
                        modelB.modelIterations = newParams.modelIterations;
                        modelB.fit             = newParams.fit;
                        modelB.memo            = newParams.memo;


                        break;
                    }
    }

    newParameters = true;
}

void DetectorThread::loadModelA(QString model){

    cout<<"LoadModelA"<<endl;

    fitMutex.lock();

    try{
        free(guiA);
        }catch(...){
            std::cout<<"load model hiba"<<std::endl;
        }


    model.replace("level_0","level_#");

    const char* fileName = model.toStdString().c_str();

    vul_arg<const char*> dmFile(0,"Deformable model sequence fname",fileName);
    vul_arg<const char*> imFile(0,"Image file to fit in","/home/zoltan/DeMoLib_v1_1_1/data/images/im01.jpg");
    vul_arg<int>         nLevel("-l","Number of pyramid levels",1);
    vul_arg<int>         filter("-f","Image filter (0=raw,1=greyscale)",1);
    vul_arg<int>         maxIter("-i","Maximum iterations/level",20);
    vul_arg<int>         flag("--flag","Fitting flag",0);
    vul_arg<bool>        ascii("--ascii","Files are in ASCII format",false);
    vul_arg<double>      iterDec("-d","Iteration decrement",1.0);

    //get filenames for deformable models
    vul_sequence_filename_map dm_map(dmFile(),0,nLevel()-1);

    vcl_vector<vcl_string> dmFiles(nLevel());

    for(int i = 0; i < nLevel(); i++){
        dmFiles[i] = dm_map.image_name(i);
    }

    //create gui object

    vcl_string imFname = imFile();


    guiA = new DeMoLib_fit_gui(dmFiles,imFname,filter(),ascii());

    guiA->SetMaxIter(maxIter());
    guiA->SetDecIter(iterDec());
    guiA->SetFitFlag(flag());

    fitMutex.unlock();
}

void DetectorThread::loadModelB(QString model){



    fitMutex.lock();

        cout<<"LoadModelB begin"<<endl;
try{
    free(guiB);
    }catch(...){
        std::cout<<"load model hiba"<<std::endl;
    }

    model.replace("level_0","level_#");

    const char* fileName = model.toStdString().c_str();

    vul_arg<const char*> dmFile(0,"Deformable model sequence fname",fileName);
    vul_arg<const char*> imFile(0,"Image file to fit in","/home/zoltan/DeMoLib_v1_1_1/data/images/im01.jpg");
    vul_arg<int>         nLevel("-l","Number of pyramid levels",1);
    vul_arg<int>         filter("-f","Image filter (0=raw,1=greyscale)",1);
    vul_arg<int>         maxIter("-i","Maximum iterations/level",20);
    vul_arg<int>         flag("--flag","Fitting flag",0);
    vul_arg<bool>        ascii("--ascii","Files are in ASCII format",false);
    vul_arg<double>      iterDec("-d","Iteration decrement",1.0);

    //get filenames for deformable models
    vul_sequence_filename_map dm_map(dmFile(),0,nLevel()-1);

    vcl_vector<vcl_string> dmFiles(nLevel());

    for(int i = 0; i < nLevel(); i++){
        dmFiles[i] = dm_map.image_name(i);
    }

    //create gui object

    vcl_string imFname = imFile();


    guiB = new DeMoLib_fit_gui(dmFiles,imFname,filter(),ascii());

    guiB->SetMaxIter(maxIter());
    guiB->SetDecIter(iterDec());
    guiB->SetFitFlag(flag());

    cout<<"LoadModelB end"<<endl;

    fitMutex.unlock();
}

void DetectorThread::contrastSizeChanged(int size){
    contrastSize = size;
    newParameters = true;
}







