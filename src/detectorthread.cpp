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
#include "QTime"
#include "QtConcurrentRun"

#include <cv.h>
#include <highgui.h>

#include "facetrackerfunctions.h"
/*

/* demolib*/
/*
#include <fit.h>
#include <DeMoLib_demo.h>
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

*/
#include <Tracker.h>
#include "svm.h"
#include <algorithm>

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



float DetectorThread::distance(point p1, point p2){
    return sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
}

using namespace std;

extern CvHaarClassifierCascade  *cascade_face, *cascade_eye, *cascade_hand;
extern char                     *filename_face;
extern char                     *filename_smile;
extern char                     *filename_hand;

CvRect                          *face;

svm_model *smileSVM,*angrySVM, *contemptSVM, *disgustSVM, *fearSVM, *sadnessSVM, *surpriseSVM;
svm_node nodes[67];

  char ftFile[256],conFile[256],triFile[256];
  bool fcheck;
  double scale;
  int fpd;
  bool show;

//set other tracking parameters
  std::vector<int> wSize1(1);
  std::vector<int> wSize2(3);
  int nIter;
  double clamp;
  double fTol;
  FACETRACKER::Tracker model;
  cv::Mat tri;
  cv::Mat kon;

  //initialize camera and display window
  cv::Mat frameMat,gray,im;
  double fps;
  char sss[256];
  std::string text;
  int64 t1,t0;
  int fnum;

  //loop until quit (i.e user presses ESC)
  bool failed;


    // Time measurement
    QTime *SVMTimer;
    QTime *HaarFace, *HaarHand;
    QTime *FaceTrackerTimer;
    QTime *processFrameTimer;
    QTime *sendImageTimer;
    QTime *incomingTimer;

    extern QTime *signalTime;
    extern QTime *captureSignalTime;
    extern QTime *toDetectorSignalTime;

    // variable for checking hand detection continuity
    short handCount = 0;



DetectorThread::DetectorThread()
{

    // Time measurement
    SVMTimer = new QTime();
    HaarFace = new QTime();
    HaarHand = new QTime();
    FaceTrackerTimer = new QTime();
    processFrameTimer = new QTime();
    sendImageTimer = new QTime();
    incomingTimer = new QTime();




      smileSVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/SmileTrainingAll.model");
      angrySVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/Angry.model");
      contemptSVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/Contempt.model");
      disgustSVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/Disgust.model");
      fearSVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/Fear.model");
      sadnessSVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/Sadness.model");
      surpriseSVM = svm_load_model("/home/zoltan/libsvm-3.12/tools/Surprise.model");


      fcheck = false;
      scale = 1;
      fpd = -1;
      show = true;
      parse_cmd(0,NULL,ftFile,conFile,triFile,fcheck,scale,fpd);

    //set other tracking parameters
      wSize1[0] = 7;
      wSize2[0] = 11;
      wSize2[1] = 9;
      wSize2[2] = 7;
      nIter = 5;
      clamp=3;
      fTol=0.01;

      model.Load(ftFile);
      tri=FACETRACKER::IO::LoadTri(triFile);
      kon=FACETRACKER::IO::LoadCon(conFile);

      //initialize camera and display window
      fps=0;
      sss[256];
      t0 = cvGetTickCount();
      fnum=0;

      //loop until quit (i.e user presses ESC)
      failed = true;

      for (int i = 0; i<450; i++){
          plotPoints.push_back(0);
          plotPoints2.push_back(0);
      }

/*
    vul_arg<const char*> dmFile(0,"Deformable model sequence fname","/home/zoltan/DeMoLib_v1_1_1/model/ckcolor/test-level_#.aam_di_linear");
    vul_arg<const char*> imFile(0,"Image file to fit in","/home/zoltan/Asztal/151");
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
*/

    cutoff = 50;

    frame = NULL;
    //frameToShow = NULL;
    previousFrame = NULL;
    black = false;
    source_Ready = false;
    imageReceived = false;
    newParameters = false;
    processing = false;
    waitForImage = true;
    featuresCaptured = false;

    drawNumbers = false;

    points[0] = (CvPoint2D32f*)cvAlloc(68*sizeof(CvPoint2D32f));
    points[1] = (CvPoint2D32f*)cvAlloc(68*sizeof(CvPoint2D32f));

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
    modelB.yShift          = 15;
    modelB.modelScale      = 0.60;
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
        if (processFrame()) sendImage();
        else imageProcessed(NULL);
        waitForImage = true;
        processing = false;

    }
}

void DetectorThread::sendImage(){
    if (!frame) {
        return;
    }

    // Starting time measurement
    sendImageTimer->start();

    IplImage *frameToSend = cvCreateImage( cvSize(frameToShow.width , frameToShow.height ), frameToShow.depth, frameToShow.nChannels );
    cvCopy(&frameToShow, frameToSend);

    // Starting signal time measurement
    signalTime->start();
    captureSignalTime->start();

    emit imageProcessed(frameToSend);

    // Reading execution time
    cout<<"sendImage execution time was: "<<sendImageTimer->elapsed()<<"ms"<<endl;

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
            cvSize( 150, 150 ) );

    int max = 0, maxwidth = 0;

    for(int i = 0 ; i < ( faces ? faces->total : 0 ) ; i++ ) {
        CvRect *r = ( CvRect* )cvGetSeqElem( faces, i );
        if (r->width > maxwidth){
            maxwidth = r->width;
            max = i;
        }

    }

    CvRect *maxFace = (CvRect*)cvGetSeqElem( faces, max );

    if (faces->total != 0){
        return cvRect(maxFace->x, maxFace->y, maxFace->width, maxFace->height);
    }else{
        return cvRect(0,0,0,0);
    }

}

CvRect DetectorThread::detectHands(IplImage *img){

    if (storage_hand){
        cvReleaseMemStorage(&storage_hand);
    }
    storage_hand = cvCreateMemStorage( 0 );

    if (!storage_hand){
        exit(0);
    }

    CvSeq *hands = cvHaarDetectObjects(
            img,
            cascade_hand,
            storage_hand,
            1.1,
            3,
            0 ,
            cvSize( 80, 80 ) );

    int max = 0, maxwidth = 0;

    for(int i = 0 ; i < ( hands ? hands->total : 0 ) ; i++ ) {
        CvRect *r = ( CvRect* )cvGetSeqElem( hands, i );
        if (r->width > maxwidth){
            maxwidth = r->width;
            max = i;
        }

    }

    CvRect *maxHand = (CvRect*)cvGetSeqElem( hands, max );

    if (hands->total != 0){
        return cvRect(maxHand->x, maxHand->y, maxHand->width, maxHand->height);
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

        pts file also needs to be saved

     */

}

double DetectorThread::distance(CvPoint pt1, CvPoint pt2){
    return sqrt(pow(pt1.x-pt2.x,2) + pow(pt1.y-pt2.y,2));
}

void DetectorThread::newCapturedImage(IplImage* img){

    // Reading signal transition time
    cout<<"CaptureThread to DetectorThread image signal transition time was: "<<toDetectorSignalTime->elapsed()<<"ms"<<endl;

    if (!img) return;
    if (processing) {
        cvReleaseImage(&img);
        return;
    }

    // Measurig function execution time
    incomingTimer->start();


    if (frame) cvReleaseImage(&frame);

    frame = cvCreateImage( cvSize(img->width,img->height), img->depth, img->nChannels );
    cvCopy(img, frame);
    cvReleaseImage(&img);

    waitForImage = false;

    // Reading function execution time
    cout<<"Incoming frame preprocessing time was: "<<incomingTimer->elapsed()<<"ms"<<endl;
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
/*
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
*/
bool DetectorThread::processFrame(){

    // Starting function execution time measuring
    processFrameTimer->start();

    if (!frame) {
        cout<<"There is no frame in the processing function"<<endl;
        return false;
    }

    // Measuring time for face detection
    HaarFace->start();

    CvRect r;

    QFuture<CvRect> future = QtConcurrent::run(this,&DetectorThread::detectFaces,frame);
    //CvRect r = detectFaces(frame);

    // Reading elapsed time for face detection
    cout<<"Face detection time was: "<<HaarFace->elapsed()<<"ms"<<endl;
/*
    if (!r.width){
        frameToShow = *frame;
        return true;
    }*/
    // Measuring time for hand detection
    HaarHand->start();


    // It has to find hands on multiple frames in a row to ensure the proper selection
    CvRect r2 = detectHands(frame);

    if (r2.width){
        handCount++;
    }else{
        handCount = 0;
    }



    // Reading elapsed time for hand detection
    cout<<"Hand detection time was: "<<HaarHand->elapsed()<<"ms"<<endl;


    //enhanceContrast(frame, 0, 0, 640, 480);

    //IplImage* frameToAAM = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);
    //cvCvtColor(frame,frameToAAM,CV_RGB2GRAY);


    //if (frameToShow) cvReleaseImage(&frameToShow);
    //frameToShow = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);

    // Startin time measurement for FaceTracker
    FaceTrackerTimer->start();

    //grab image, resize and flip
    frameMat = cv::Mat(frame,true);
    if(scale == 1)im = frameMat;
    else cv::resize(frameMat,im,cv::Size(scale*frameMat.cols,scale*frameMat.rows));
    //cv::flip(im,im,1);
    cv::cvtColor(im,gray,CV_BGR2GRAY);

    //track this image
    std::vector<int> wSize; if(failed)wSize = wSize2; else wSize = wSize1;
    if(model.Track(gray,wSize,fpd,nIter,clamp,fTol,fcheck) == 0){
      int idx = model._clm.GetViewIdx(); failed = false;
      Draw(im,model._shape,kon,tri,model._clm._visi[idx],drawNumbers);
    }else{
      if(show){cv::Mat R(im,cvRect(0,0,150,50)); R = cv::Scalar(0,0,255);}
      model.FrameReset(); failed = true;
    }

    frameToShow = im;

    future.waitForFinished();

    r = future.result();

    // Reading FaceTracker execution time
    cout<<"FaceTracker execution time was: "<<FaceTrackerTimer->elapsed()<<"ms"<<endl;


    if (r.width)
    cvRectangle( &frameToShow,
                 cvPoint( r.x, r.y ),
                 cvPoint( r.x + r.width, r.y + r.height ),
                 CV_RGB( 255, 0, 0 ), 1, 8, 0 );

    if (r2.width)
    cvRectangle( &frameToShow,
                 cvPoint( r2.x, r2.y ),
                 cvPoint( r2.x + r2.width, r2.y + r2.height ),
                 CV_RGB( 255, 0, 0 ), 1, 8, 0 );


    if (r.width){
        double out = 0;
        for (int i = 0; i < 66; i++){
            if (model._shape.at<double>(i,0)<r.x){
                out+=abs(model._shape.at<double>(i,0)-r.x);
                continue;
            }
            if (model._shape.at<double>(i,0)>(r.x+r.width)){
                out+=abs(model._shape.at<double>(i,0)-(r.x+r.width));
                continue;
            }
        }


        if (out>30){
            model.FrameReset();
            return false;
        }
    }

    point points[10];

    points[0].x = model._shape.at<double>(36,0);
    points[0].y = model._shape.at<double>(36+66,0);

    points[1].x = model._shape.at<double>(39,0);
    points[1].y = model._shape.at<double>(39+66,0);

    points[2].x = model._shape.at<double>(42,0);
    points[2].y = model._shape.at<double>(42+66,0);

    points[3].x = model._shape.at<double>(45,0);
    points[3].y = model._shape.at<double>(45+66,0);

    points[4].x = model._shape.at<double>(48,0);
    points[4].y = model._shape.at<double>(48+66,0);

    points[5].x = model._shape.at<double>(54,0);
    points[5].y = model._shape.at<double>(54+66,0);

    points[6].x = model._shape.at<double>(51,0);
    points[6].y = model._shape.at<double>(51+66,0);

    points[7].x = model._shape.at<double>(57,0);
    points[7].y = model._shape.at<double>(57+66,0);

    points[8].x = model._shape.at<double>(0,0);
    points[8].y = model._shape.at<double>(66,0);

    points[9].x = model._shape.at<double>(16,0);
    points[9].y = model._shape.at<double>(16+66,0);


  //  cvCircle(&frameToShow,cvPoint(points[0].x,points[0].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[1].x,points[1].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[2].x,points[2].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[3].x,points[3].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[4].x,points[4].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[5].x,points[5].y),5,cvScalar(255,255,255,0),-1,8,0);
    cvCircle(&frameToShow,cvPoint(points[6].x,points[6].y),5,cvScalar(255,255,255,0),-1,8,0);
    cvCircle(&frameToShow,cvPoint(points[7].x,points[7].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[8].x,points[8].y),5,cvScalar(255,255,255,0),-1,8,0);
  //  cvCircle(&frameToShow,cvPoint(points[9].x,points[9].y),5,cvScalar(255,255,255,0),-1,8,0);
/*

  // 0 1:0.286838 2:0.612854 3:0.919051 4:0.957818 5:0.762663 6:0.766063
    // 1 1:0.173600 2:0.452918 3:0.758034 4:0.809423 5:0.641132 6:0.661205

    nodes[0].index = 1;
    nodes[0].value = distance(points[6],points[7])/distance(points[0],points[3]);

    nodes[1].index = 2;
    nodes[1].value = distance(points[4],points[5])/distance(points[0],points[3]);

    nodes[2].index = 3;
    nodes[2].value = distance(points[4],points[8])/distance(points[0],points[3]);

    nodes[3].index = 4;
    nodes[3].value = distance(points[5],points[9])/distance(points[0],points[3]);

    nodes[4].index = 5;
    nodes[4].value = distance(points[0],points[4])/distance(points[0],points[3]);

    nodes[5].index = 6;
    nodes[5].value = distance(points[3],points[5])/distance(points[0],points[3]);

    nodes[6].index = -1;

*/


    const cv::Mat& pose = model._clm._pglobl;


    /*
                 Y
                  ^
                  |
                  |
                  |
                  |
                  |
                    ---------->  Z
                 /
                /
               /
              /
             < X


      1. sine of turning angle around axis Z
      2. sine of turning angle around axis Y
      3. sine of turning angle around axis X


      */

    double sinx = pose.at<double>(1, 0);
    double siny = pose.at<double>(2, 0);
    double sinz = pose.at<double>(3, 0);

    double cosx = cos(asin(sinx));
    double cosy = cos(asin(siny));
    double cosz = cos(asin(sinz));

    int xx = 1;
    int xy = 0;
    int xz = 0;

    int yx = 0;
    int yy = 1;
    int yz = 0;

    int zx = 0;
    int zy = 0;
    int zz = 1;


    double cx = 0;
    double cy = 0;
    double cz = 0;
    double ez = 0.05;

    double dxx = cosy*(sinz*(xy-cy)) + cosz*(xx-cx)-siny*(xz-cz);
    double dxy = sinx*(cosy*(xz-cz)+siny*(sinz*(xy-cy)+cosz*(xx-cx))) + cosx*(cosz*(xy-cy)-sinz*(xx-cx)) ;
    double dxz = cosx*(cosy*(xz-cz)+siny*(sinz*xy+cosz*xx)) - sinx*(cosz*xy-sinz*xx) ;

    double dyx = cosy*(sinz*(yy-cy)) + cosz*(yx-cx)-siny*(yz-cz);
    double dyy = sinx*(cosy*(yz-cz)+siny*(sinz*(yy-cy)+cosz*(yx-cx))) + cosx*(cosz*(yy-cy)-sinz*(yx-cx)) ;
    double dyz = cosx*(cosy*(yz-cz)+siny*(sinz*yy+cosz*yx)) - sinx*(cosz*yy-sinz*yx) ;

    double dzx = cosy*(sinz*(zy-cy)) + cosz*(zx-cx)-siny*(zz-cz);
    double dzy = -(sinx*(cosy*(zz-cz)+siny*(sinz*(zy-cy)+cosz*(zx-cx))) + cosx*(cosz*(zy-cy)-sinz*(zx-cx))) ;
    double dzz = cosx*(cosy*(zz-cz)+siny*(sinz*zy+cosz*zx)) - sinx*(cosz*zy-sinz*zx) ;

#define HEADPOS
#ifdef HEADPOS

    /*

      Measurements in cm-s

      Equation of the plane is: z = 80
      Equation of the line is given by P(0,0,0) with n(dzx,dzy,dzz)
      The looked slot is given by the intersection of the two.

      */

    // dhc
    double dhc = 80;
    // dvc
    double dvc = (model._shape.at<double>(27+66,0)-240) * 0.1375;
    // gamma
    double gamma = asin(pose.at<double>(1, 0));
    // béta
    double beta = atan(dvc/dhc);
    // alfa
    double alfa = gamma-beta;
    // da
    double da = tan(alfa)*dhc;

    // t
    double t = dhc/dzz;
    // intersection y
    double liney = t*dzy/50; // 50 cm height
    // intersection x
    double linex = t*dzx/40; // 40 cm width

    emit coord(linex,liney);


#endif

#define FORGAT

    // x blue
    // y green
    // z red


#ifdef FORGAT

    cvLine(&frameToShow,cvPoint(500,100),cvPoint(500+50*dxx,100-50*dxy),cvScalar(255,0,0,0),3,8,0);
    cvLine(&frameToShow,cvPoint(500,100),cvPoint(500+50*dyx,100-50*dyy),cvScalar(0,255,0),3,8,0);
    cvLine(&frameToShow,cvPoint(500,100),cvPoint(500+50*dzx,100-50*dzy),cvScalar(0,0,255,0),3,8,0);

#else

    cvLine(&frameToShow,cvPoint(500,100),cvPoint(500+50*xx,100-50*xy),cvScalar(255,0,0,0),3,8,0);
    cvLine(&frameToShow,cvPoint(500,100),cvPoint(500+50*yx,100-50*yy),cvScalar(0,255,0),3,8,0);
    cvLine(&frameToShow,cvPoint(500,100),cvPoint(500+50*zx,100-50*zy),cvScalar(0,0,255,0),3,8,0);

#endif



    point p1,p2;
    p2.x = model._shape.at<double>(27,0);
    p2.y = model._shape.at<double>(27+66,0);


    if (r.width){
        cvCircle(&frameToShow,cvPoint(r.x+r.width/2,r.y+r.height/2),5,cvScalar(255,255,255,0),-1,8,0);

        if (abs((r.x+r.width/2)-(p2.x)) > 50){
            model.FrameReset();
            return false;
        }
        if (abs((r.y+r.height/2)-(p2.y)) > 70){
            model.FrameReset();
            return false;
        }
    }

    for (int i = 0; i<66; i++){
        p1.x = model._shape.at<double>(i,0);
        p1.y = model._shape.at<double>(i+66,0);

        nodes[i].index = i+1;
        nodes[i].value = distance(p1,p2) / distance(points[0],points[3]);
    }

    nodes[66].index = -1;

    double smileResult;
    double angryResult;
    double contemptResult;
    double disgustResult;
    double fearResult;
    double sadnessResult;
    double surpriseResult;

    // Time measuring for SVM regression
    SVMTimer->start();

    int measureInterval = 1;
    for (int i = 0; i< measureInterval; i++){
    smileResult = svm_predict(smileSVM, nodes);
    angryResult = svm_predict(angrySVM, nodes);
    contemptResult = svm_predict(contemptSVM, nodes);
    disgustResult = svm_predict(disgustSVM, nodes);
    fearResult = svm_predict(fearSVM, nodes);
    sadnessResult = svm_predict(sadnessSVM, nodes);
    surpriseResult = svm_predict(surpriseSVM, nodes);
}
    // Reading elapsed time for SVM regression
    double elapsedTime = (double)SVMTimer->elapsed()/measureInterval;
    cout<<"SVM regression time for all emotions was: "<<elapsedTime<<"ms"<<endl;

    std::vector<int>    spectrumVolumes;

    spectrumVolumes.push_back(smileResult*100);
    spectrumVolumes.push_back(angryResult*100);
    spectrumVolumes.push_back(contemptResult*100);
    spectrumVolumes.push_back(disgustResult*100);
    spectrumVolumes.push_back(fearResult*100);
    spectrumVolumes.push_back(sadnessResult*100);
    spectrumVolumes.push_back(surpriseResult*100);

    emit volumes(spectrumVolumes);
    emit smilePercentage(smileResult*100);
    if (smileResult*100>cutoff) emit smileDetected();

    if (handCount > 10){
        emit handSelected(linex,liney);
    }
    if (surpriseResult*100>50){
        emit surpriseSelected(linex,liney);
    }
    // Reading function execution time
    cout<<"processFrame execution time was: "<<processFrameTimer->elapsed()<<"ms"<<endl;


/*
    rotate(plotPoints.begin(), plotPoints.begin()+1, plotPoints.end());
    plotPoints.pop_back();
    plotPoints.push_back((int)(result*100));

    rotate(plotPoints2.begin(), plotPoints2.begin()+1, plotPoints2.end());
    plotPoints2.pop_back();
    plotPoints2.push_back((int)(result2*100));
*/
    //emit plotPointsSignal(plotPoints,plotPoints2);

    return true;
/*
    cvCopy(frame, frameToShow);

    CvRect r = detectFaces(frame);

    if (!r.width){

        emit faceDetected(false);
        emit smilePercentage(0);

    }else{

        fitMutex.lock();
        cout<<"process begin"<<endl;

        emit faceDetected(true);

        if (contrastSize)
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

            for (int i = 0; i<68; i++){
                points[0][i].x = s[2*i];
                points[0][i].y = s[2*i+1];
            }

        }

/*
            double smile = 0;

            smile = calculateSmile(s);

            emit smilePercentage((int)smile);
            if (smile > 30) emit smileDetected();


            //drawAnnotationLines(frameToShow,s);
        }

          cvReleaseImage(&frameToAAM);

          cout<<"process end"<<endl;
          fitMutex.unlock();
          */

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



/*

  memory leak when model releasing

  */
void DetectorThread::loadModelA(QString model){
/*
    cout<<"LoadModelA"<<endl;

    fitMutex.lock();

    try{
        delete guiA;
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

    fitMutex.unlock();*/
}

void DetectorThread::loadModelB(QString model){
/*


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

    fitMutex.unlock();*/
}

void DetectorThread::contrastSizeChanged(int size){
    contrastSize = size;
    newParameters = true;
}

void DetectorThread::smileCutOff(int cutoff){
    this->cutoff = cutoff;
}

void DetectorThread::resetModel(){
    model.FrameReset();
    cout<<"model resetted"<<endl;
}

void DetectorThread::pointsAnnotations(bool drawNumbers){
    this->drawNumbers = drawNumbers;
}







