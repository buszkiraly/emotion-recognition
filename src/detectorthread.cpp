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
#include <algorithm>    // std::rotate

#include <cv.h>
#include <highgui.h>
#include "imgproc.hpp"

#include "facetrackerfunctions.h"

#include <Tracker.h>
#include "svm.h"
#include <algorithm>


CvMat *Q;
CvMat *mx1;
CvMat *my1;
CvMat *mx2;
CvMat *my2;


float DetectorThread::distance(point p1, point p2){
    return sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
}

using namespace std;

extern CvHaarClassifierCascade  *cascade_face, *cascade_hand;
extern char                     *filename_face;
extern char                     *filename_smile;
extern char                     *filename_hand;

CvRect                          *face;

svm_model *smileSVM,*angrySVM, *contemptSVM, *disgustSVM, *fearSVM, *sadnessSVM, *surpriseSVM;
svm_node nodes[133];

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
  FACETRACKER::Tracker model,model_nomem,model_right,model_right_nomem;
  cv::Mat tri;
  cv::Mat kon;

  //initialize camera and display window
  cv::Mat frameMat,gray,im;
  cv::Mat frameMat_right,gray_right,im_right;
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

    extern cv::Mat feature_mtx,scale_mtx,mean_mtx;

    short check_ctr = 0;


DetectorThread::DetectorThread()
{

    Q = (CvMat *)cvLoad("Q.xml",NULL,NULL,NULL);
    mx1 = (CvMat *)cvLoad("mx1.xml",NULL,NULL,NULL);
    my1 = (CvMat *)cvLoad("my1.xml",NULL,NULL,NULL);
    mx2 = (CvMat *)cvLoad("mx2.xml",NULL,NULL,NULL);
    my2 = (CvMat *)cvLoad("my2.xml",NULL,NULL,NULL);

    // Time measurement
    SVMTimer = new QTime();
    HaarFace = new QTime();
    HaarHand = new QTime();
    FaceTrackerTimer = new QTime();
    processFrameTimer = new QTime();
    sendImageTimer = new QTime();
    incomingTimer = new QTime();




    smileSVM = svm_load_model("/home/zoltan/emorec/models/xy_happy.model");
    angrySVM = svm_load_model("/home/zoltan/emorec/models/xy_angry.model");
    contemptSVM = svm_load_model("/home/zoltan/emorec/models/xy_contempt.model");
    disgustSVM = svm_load_model("/home/zoltan/emorec/models/xy_disgusted.model");
    fearSVM = svm_load_model("/home/zoltan/emorec/models/xy_afraid.model");
    sadnessSVM = svm_load_model("/home/zoltan/emorec/models/xy_sad.model");
    surpriseSVM = svm_load_model("/home/zoltan/emorec/models/xy_surprised.model");


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
      model_nomem.Load(ftFile);
      model_right.Load(ftFile);
      model_right_nomem.Load(ftFile);
      tri=FACETRACKER::IO::LoadTri(triFile);
      kon=FACETRACKER::IO::LoadCon(conFile);

      //initialize camera and display window
      fps=0;
      //sss[256];
      t0 = cvGetTickCount();
      fnum=0;

      //loop until quit (i.e user presses ESC)
      failed = true;

      for (int i = 0; i<450; i++){
          plotPoints.push_back(0);
          plotPoints2.push_back(0);
          blinkPoints.push_back(0);
          headPointsRaw.push_back(0);
          headPointsFiltered.push_back(0);
          eyeBrowsRaw.push_back(0);
          eyeBrowsFiltered.push_back(0);
          roundness_vec.push_back(0);
          roundness_vec_filtered.push_back(0);
      }

      storage_hand = NULL;
      storage_face = NULL;

    cutoff = 50;

    frame = NULL;
    frame_right = NULL;
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

    contrastSize = 40;



}

DetectorThread::~DetectorThread(){
    //cvReleaseHaarClassifierCascade( &cascade_face );
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

    storage_face = cvCreateMemStorage( 0 );

    CvSeq *faces=cvHaarDetectObjects(img,cascade_face,storage_face,1.1,3,0,cvSize(30,30));

    int max = 0, maxwidth = 0;

    CvRect *r;

    for(int i = 0 ; i < faces->total ; i++ ) {
        r=(CvRect *)cvGetSeqElem(faces,i);
        if (r->width > maxwidth){
            maxwidth = r->width;
            max = i;
        }
    }

    r=(CvRect *)cvGetSeqElem(faces,max);

    cvReleaseMemStorage(&storage_face);

    if (!faces->total) return cvRect(0,0,0,0);
    else return cvRect(r->x, r->y, r->width, r->height);
}

CvRect DetectorThread::detectHands(IplImage *img){

    storage_hand = cvCreateMemStorage( 0 );

    CvSeq *hands=cvHaarDetectObjects(img,cascade_hand,storage_hand,1.1,3,0,cvSize(5,5));

    int max = 0, maxwidth = 0;

    CvRect *r;

    for(int i = 0 ; i < hands->total ; i++ ) {
        r=(CvRect *)cvGetSeqElem(hands,i);
        if (r->width > maxwidth){
            maxwidth = r->width;
            max = i;
        }
    }

    r=(CvRect *)cvGetSeqElem(hands,max);

    cvReleaseMemStorage(&storage_hand);

    if (!hands->total) return cvRect(0,0,0,0);
    else return cvRect(r->x, r->y, r->width, r->height);

}

void DetectorThread::setBlack(bool state){
    black = state;
    newParameters = true;
}

double DetectorThread::distance(CvPoint pt1, CvPoint pt2){
    return sqrt(pow(pt1.x-pt2.x,2) + pow(pt1.y-pt2.y,2));
}

void DetectorThread::newCapturedImage(IplImage* img, IplImage *img_right){

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

    if (frame_right) cvReleaseImage(&frame_right);

    frame_right = cvCreateImage( cvSize(img_right->width,img_right->height), img_right->depth, img_right->nChannels );
    cvCopy(img_right, frame_right);
    cvReleaseImage(&img_right);



    cv::Mat MATframe = frame;
    cv::Mat MATframe_right = frame_right;
    CvMat _src = MATframe;
    CvMat _src2 = MATframe_right;

    cvRemap(&_src, &_src, mx1, my1);
    cvRemap(&_src2, &_src2, mx2, my2);

    waitForImage = false;

    // Reading function execution time
    cout<<"Incoming frame preprocessing time was: "<<incomingTimer->elapsed()<<"ms"<<endl;
}

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

    //QFuture<CvRect> future;
    check_ctr++;
    if (check_ctr > 20) r = detectFaces(frame);//future = QtConcurrent::run(this,&DetectorThread::detectFaces,frame);


/*
    if (!r.width){
        frameToShow = *frame;
        return true;
    }*/
    // Measuring time for hand detection
    HaarHand->start();


    // It has to find hands on multiple frames in a row to ensure the proper selection
   // CvRect r2 = detectHands(frame);
/*
    if (r2.width){
        handCount++;
    }else{
        handCount = 0;
    }
*/


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
    if(scale == 1)im = frameMat.clone();
    else cv::resize(frameMat,im,cv::Size(scale*frameMat.cols,scale*frameMat.rows));
    //cv::flip(im,im,1);
    cv::cvtColor(im,gray,CV_BGR2GRAY);

    //track this image

    std::vector<int> wSize; if(failed)wSize = wSize2; else wSize = wSize1;
    if(model.Track(gray,wSize,fpd,nIter,clamp,fTol,fcheck) == 0){
      int idx = model._clm.GetViewIdx(); failed = false;
      Draw(im,model._shape,kon,tri,model._clm._visi[idx],drawNumbers);
      for (int i = 0; i<66; i++){
          model._shape2.at<double>(i,0) = model._shape2.at<double>(i,0) /*- model._shape2.at<double>(27,0) */+ 200;
          model._shape2.at<double>(i +66,0) = model._shape2.at<double>(i + 66,0) /*- model._shape2.at<double>(27 + 66,0) */+ 50;
      }
      Draw(im,model._shape2,kon,tri,model._clm._visi[idx],drawNumbers);
    }else{
      if(show){cv::Mat R(im,cvRect(0,0,150,50)); R = cv::Scalar(0,0,255);}
      model.FrameReset(); failed = true;
    }

    //grab image, resize and flip
    frameMat_right = cv::Mat(frame_right,true);
    if(scale == 1)im_right = frameMat_right.clone();
    else cv::resize(frameMat_right,im_right,cv::Size(scale*frameMat_right.cols,scale*frameMat_right.rows));
    //cv::flip(im,im,1);
    cv::cvtColor(im_right,gray_right,CV_BGR2GRAY);

    //track this image

    if(failed)wSize = wSize2; else wSize = wSize1;
    if(model_right.Track(gray_right,wSize,fpd,nIter,clamp,fTol,fcheck) == 0){
      int idx = model_right._clm.GetViewIdx(); failed = false;
      Draw(im_right,model_right._shape,kon,tri,model_right._clm._visi[idx],drawNumbers);
    }else{
      if(show){cv::Mat R(im_right,cvRect(0,0,150,50)); R = cv::Scalar(0,0,255);}
      model_right.FrameReset(); failed = true;
    }

    if (check_ctr > 20){

        check_ctr = 0;

        model_nomem.FrameReset();
        model_right_nomem.FrameReset();

        if(model_nomem.Track(gray,wSize,fpd,nIter,clamp,fTol,fcheck) == 0){
          int idx = model_nomem._clm.GetViewIdx(); failed = false;
          Draw(im,model_nomem._shape,kon,tri,model_nomem._clm._visi[idx],drawNumbers);
        }else{
          if(show){cv::Mat R(im,cvRect(0,0,150,50)); R = cv::Scalar(0,0,255);}
          model_nomem.FrameReset(); failed = true;
        }

        if(model_right_nomem.Track(gray_right,wSize,fpd,nIter,clamp,fTol,fcheck) == 0){
          int idx = model_right_nomem._clm.GetViewIdx(); failed = false;
          Draw(im_right,model_right_nomem._shape,kon,tri,model_right_nomem._clm._visi[idx],drawNumbers);
        }else{
          if(show){cv::Mat R(im_right,cvRect(0,0,150,50)); R = cv::Scalar(0,0,255);}
          model_right_nomem.FrameReset(); failed = true;
        }

        point mem, nomem;
        for (int i = 17; i < 36; i++){
            mem.x = model._shape.at<double>(i,0);
            mem.y = model._shape.at<double>(i+66,0);
            nomem.x = model_nomem._shape.at<double>(i,0);
            nomem.y = model_nomem._shape.at<double>(i+66,0);
            if (distance(mem,nomem) > 10){
                model.FrameReset();
                break;
            }
        }
        for (int i = 17; i < 36; i++){
            mem.x = model_right._shape.at<double>(i,0);
            mem.y = model_right._shape.at<double>(i+66,0);
            nomem.x = model_right_nomem._shape.at<double>(i,0);
            nomem.y = model_right_nomem._shape.at<double>(i+66,0);
            if (distance(mem,nomem) > 10){
                model_right.FrameReset();
                break;
            }
        }

        for (int i = 0; i < 4; i++){
            if ( (abs(model._shape.at<double>(i,0) - model_nomem._shape.at<double>(i,0)) > 10) ||
                (abs(model._shape.at<double>(16-i,0) - model_nomem._shape.at<double>(16-i,0)) > 10) ) {
                model.FrameReset();
                break;
            }
        }
        for (int i = 0; i < 4; i++){
            if ( (abs(model_right._shape.at<double>(i,0) - model_right_nomem._shape.at<double>(i,0)) > 10) ||
                (abs(model_right._shape.at<double>(16-i,0) - model_right_nomem._shape.at<double>(16-i,0)) > 10) ) {
                model_right.FrameReset();
                break;
            }
        }

        if ( r.width ){
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

    }




    frameToShow = im;

    // Reading FaceTracker execution time
    cout<<"FaceTracker execution time was: "<<FaceTrackerTimer->elapsed()<<"ms"<<endl;

/*
    if (r.width){
        cvRectangle( &frameToShow,
                     cvPoint( r.x, r.y ),
                     cvPoint( r.x + r.width, r.y + r.height ),
                     CV_RGB( 255, 0, 0 ), 1, 8, 0 );

        cvSetImageROI(frame, r);
        r2 = detectHands(frame);
        if (r2.width)
            cvRectangle( &frameToShow,
                         cvPoint( r.x + r2.x, r.y + r2.y ),
                         cvPoint( r.x + r2.x + r2.width, r.y + r2.y + r2.height ),
                         CV_RGB( 0, 255, 0 ), 1, 8, 0 );

        cvResetImageROI(frame);



    }
/*
    if (r2.width)
    cvRectangle( &frameToShow,
                 cvPoint( r2.x, r2.y ),
                 cvPoint( r2.x + r2.width, r2.y + r2.height ),
                 CV_RGB( 0, 255, 0 ), 1, 8, 0 );
*/


    /*
     *  Checking the reliability of the model fitting
     */

    // from the rotated back model parameters
    // moving to point#28 starting from 1...
/*
    point center2;
    center2.x = model._shape2.at<double>(27,0);
    center2.y = model._shape2.at<double>(27+66,0);
    for (int i = 0; i < 66; i++){
        model._shape2.at<double>(i,0) -= center2.x;
        model._shape2.at<double>(i+66,0) -= center2.y;
    }

    // check if upper lip point is above the bottom nose point
    if (model._shape2.at<double>(51+66,0) <= model._shape2.at<double>(33+66,0)) model.FrameReset();

    // nem jó a model reset, mert akkor máshogy áll be szarul, jelezni kéne hogy csináljon valamit a felhasználó mert nem ok

*/


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

    //cvCircle(&frameToShow,cvPoint(points[6].x,points[6].y),5,cvScalar(255,255,255,0),-1,8,0);
    //cvCircle(&frameToShow,cvPoint(points[7].x,points[7].y),5,cvScalar(255,255,255,0),-1,8,0);




    /*********          Pupil detection             *************/
/*
      //  cv::Mat subImg = im(cvRect(points[0].x, points[0].y - 20, 20, 20));
        rectangle(frameMat, cvPoint(points[0].x, points[0].y - 10), cvPoint(points[1].x, points[1].y + 10), cvScalar(255,255,255,0),1,8,0);
        cv::Mat subImg = frameMat(cvRect(points[0].x, points[0].y - 10, points[1].x - points[0].x, 20));
        //cv::Mat subImg = im(cv::Range(points[0].x, points[0].y), cv::Range(points[1].x, points[1].y));
        cv::Mat pupil_gray;
        cv::cvtColor(subImg, pupil_gray, CV_BGR2GRAY);
        cv::equalizeHist(pupil_gray,pupil_gray);
        cv::threshold(pupil_gray, pupil_gray, 50, 255, cv::THRESH_BINARY);


        cv::dilate(pupil_gray,pupil_gray,cv::Mat());

        pupil_gray = ~pupil_gray;

        int min_x = points[1].x - points[0].x, min_y = 20, max_x = 0, max_y = 0;

        for (int i = 0; i < 20; i++)
            for (int j = 0; j < points[1].x - points[0].x ; j++){
                if (pupil_gray.at<uchar>(cv::Point(j,i)) == 255){
                    if (j > max_x) max_x = j;
                    if (j < min_x) min_x = j;
                }
            }


        for (int j = 0; j < points[1].x - points[0].x ; j++)
            for (int i = 0; i < 20; i++){
                    if (pupil_gray.at<uchar>(cv::Point(j,i)) == 255){
                        if (i > max_y) max_y = i;
                        if (i < min_y) min_y = i;
                    }
                }


        double width = max_x-min_x;
        double height = max_y - min_y;
        double roundness = height*height/(width*width);

        if (roundness < 0.2) cv::circle(pupil_gray,cv::Point(1,1),1,cv::Scalar(255,255,255));

        std::rotate(roundness_vec.begin(),roundness_vec.begin()+1,roundness_vec.end());
        roundness_vec.at(449) = 50*roundness;

        cout << "ROUNDNESS: " << roundness << endl;

        cv::rectangle(pupil_gray,cv::Point(min_x,min_y),cv::Point(max_x,max_y),cv::Scalar(255,255,255));

        cv::cvtColor(pupil_gray, subImg, CV_GRAY2BGR);
        //frameToShow = subImg;


    /************************************************************/

        /***************       Eye blink detection        ***********/
            point left_upper, left_bottom;
            left_upper.x = model._shape.at<double>(20,0);
            left_upper.y = model._shape.at<double>(20+66,0);
            left_bottom.x = model._shape.at<double>(38,0);
            left_bottom.y = model._shape.at<double>(38+66,0);
            std::rotate(plotPoints.begin(),plotPoints.begin()+1,plotPoints.end());
            plotPoints.pop_back();
            plotPoints.push_back(100*distance(left_upper,left_bottom)/distance(points[0],points[3]));

            double head = model._clm._pglobl.at<double>(1,0) + model._clm._pglobl.at<double>(2,0) + model._clm._pglobl.at<double>(3,0);
            head = head * 100;
            std::rotate(headPointsRaw.begin(),headPointsRaw.begin()+1,headPointsRaw.end());
            headPointsRaw.at(449) = head;

            std::rotate(eyeBrowsRaw.begin(),eyeBrowsRaw.begin()+1,eyeBrowsRaw.end());
            eyeBrowsRaw.at(449) = 100*(model._shape.at<double>(27+66,0)-model._shape.at<double>(19+66,0))/distance(points[0],points[3]);

            int high_pass_kernel[3] = {-3,0,3};


            for (int i = 1; i < 449; i++){
                eyeBrowsFiltered.at(i) = 0;
                for (int j = -1; j< 2; j++)
                    eyeBrowsFiltered.at(i) += 2*(eyeBrowsRaw.at(i+j) * high_pass_kernel[j+1]);
            }
            for (int i = 1; i < 449; i++){
                headPointsFiltered.at(i) = 0;
                for (int j = -1; j< 2; j++)
                    headPointsFiltered.at(i) += 2*(headPointsRaw.at(i+j) * high_pass_kernel[j+1]);
            }
            for (int i = 1; i < 449; i++){
                roundness_vec_filtered.at(i) = 0;
                for (int j = -1; j< 2; j++)
                    roundness_vec_filtered.at(i) += (roundness_vec.at(i+j) * high_pass_kernel[j+1]);
            }

            bool still = true;
            for (int i = 0; i < 5; i++){
                if (abs(headPointsFiltered.at(449-i)) > 25) still = false;
            }
            for (int i = 1; i < 449; i++){
                blinkPoints.at(i) = 0;
                for (int j = -1; j< 2; j++)
                    blinkPoints.at(i) += 2*(plotPoints.at(i+j) * high_pass_kernel[j+1]);
            }

            if ( (blinkPoints.at(448) > 20) && still && (eyeBrowsFiltered.at(448) < 20)) cvCircle(&frameToShow,cvPoint(600,50),5,cvScalar(255,255,255,0),-1,8,0);



            emit plotPointsSignal(eyeBrowsFiltered, blinkPoints);
        /************************************************************/

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

    siny-=0.2;

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
    //double ez = 0.05;

    double dxx = cosy*(sinz*(xy-cy)) + cosz*(xx-cx)-siny*(xz-cz);
    double dxy = sinx*(cosy*(xz-cz)+siny*(sinz*(xy-cy)+cosz*(xx-cx))) + cosx*(cosz*(xy-cy)-sinz*(xx-cx)) ;
    //double dxz = cosx*(cosy*(xz-cz)+siny*(sinz*xy+cosz*xx)) - sinx*(cosz*xy-sinz*xx) ;

    double dyx = cosy*(sinz*(yy-cy)) + cosz*(yx-cx)-siny*(yz-cz);
    double dyy = sinx*(cosy*(yz-cz)+siny*(sinz*(yy-cy)+cosz*(yx-cx))) + cosx*(cosz*(yy-cy)-sinz*(yx-cx)) ;
    //double dyz = cosx*(cosy*(yz-cz)+siny*(sinz*yy+cosz*yx)) - sinx*(cosz*yy-sinz*yx) ;

    double dzx = cosy*(sinz*(zy-cy)) + cosz*(zx-cx)-siny*(zz-cz);
    double dzy = -(sinx*(cosy*(zz-cz)+siny*(sinz*(zy-cy)+cosz*(zx-cx))) + cosx*(cosz*(zy-cy)-sinz*(zx-cx))) ;
    double dzz = cosx*(cosy*(zz-cz)+siny*(sinz*zy+cosz*zx)) - sinx*(cosz*zy-sinz*zx) ;

#define HEADPOS
#ifdef HEADPOS

    /*

      Measurements in cm-s

      Equation of the plane is: z = 80
      Equation of the line is given by P(0,da,0) with n(dzx,dzy,dzz)
      The looked slot is given by the intersection of the two.

      */

    // dhc
    double dhc = 80;
    // dvc
    double dvc = (model._shape.at<double>(27+66,0)-240) * 0.1375;
    // gamma
    //double gamma = asin(pose.at<double>(1, 0));
    // béta
    //double beta = atan(dvc/dhc);
    // alfa
    //double alfa = gamma-beta;
    // da distance from the camera on the Y axis
    //double da = tan(alfa)*dhc + dvc;

    double dxc = (model._shape.at<double>(27,0)-320) * 0.1375;
    // gamma
    //double xgamma = asin(pose.at<double>(2, 0));
    // béta
    //double xbeta = atan(dxc/dhc);
    // alfa
    //double xalfa = xgamma-xbeta;
    // da distance from the camera on the Y axis
    //double xda = tan(xalfa)*dhc + dxc;

    // t
    double t = dhc/dzz;


    // intersection y
    //double liney = t*dzy/50; // 50 cm height
    double liney = (-dvc + t*dzy)/50; // 50 cm height
    // intersection x
    //double linex = t*dzx/40; // 40 cm width
    double linex = (dxc + t*dzx)/40; // 40 cm width

    emit coord(linex,liney);

    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, 8);

    std::ostringstream osstream;
    osstream << dvc;
    std::string string_x = osstream.str();
    cvPutText(&frameToShow, string_x.c_str(), cvPoint(600,400), &font, cvScalar(255, 255, 255, 0));


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

/*
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
    */
/*
    for (int i = 0; i<66; i++){
        p1.x = model._shape.at<double>(i,0);
        p1.y = model._shape.at<double>(i+66,0);

        nodes[i].index = i+1;
        nodes[i].value = distance(p1,p2) / distance(points[0],points[3]);
    }

    nodes[66].index = -1;
    */



    int disparity = abs(model._shape.at<double>(27,0) - model_right._shape.at<double>(27,0));
    cout << "disparity: " << disparity << endl;

    double Z = cvGetReal2D(Q,2,3);
    double W = disparity * cvGetReal2D(Q,3,2) + cvGetReal2D(Q,3,3);

    Z = Z / W;

    CvFont d_font;
    cvInitFont(&d_font, CV_FONT_HERSHEY_SIMPLEX, 0.8, 0.8, 0, 1, 8);

    std::ostringstream d_osstream;
    d_osstream << Z;
    std::string d_string_x = d_osstream.str();
    cvPutText(&frameToShow, d_string_x.c_str(), cvPoint(200,30), &d_font, cvScalar(255, 255, 255, 0));

    // moving to point#28 starting from 1...
    point center;
    center.x = model._shape.at<double>(27,0);
    center.y = model._shape.at<double>(27+66,0);
    for (int i = 0; i < 66; i++){
        model._shape.at<double>(i,0) -= center.x;
        model._shape.at<double>(i+66,0) -= center.y;
    }

    point p36,p45,p48,p54,p51,p57;

    p36.x = model._shape.at<double>(36,0);
    p36.y = model._shape.at<double>(36+66,0);
    p45.x = model._shape.at<double>(45,0);
    p45.y = model._shape.at<double>(45+66,0);
    p48.x = model._shape.at<double>(48,0);
    p48.y = model._shape.at<double>(48+66,0);
    p51.x = model._shape.at<double>(51,0);
    p51.y = model._shape.at<double>(51+66,0);
    p54.x = model._shape.at<double>(54,0);
    p54.y = model._shape.at<double>(54+66,0);
    p57.x = model._shape.at<double>(57,0);
    p57.y = model._shape.at<double>(57+66,0);

    for (int i = 0; i<66; i++){

        nodes[2*i].index = 2*i+1;
        nodes[2*i].value = model._shape.at<double>(i,0) / distance(p36,p45);
        nodes[2*i+1].index = 2*i+2;
        nodes[2*i+1].value = model._shape.at<double>(i+66,0) / distance(p36,p45);
        // scaling

        double scale_low = scale_mtx.at<double>(2*i,0);
        double scale_high = scale_mtx.at<double>(2*i,1);
        double scale_avg = (scale_high + scale_low)/2;
        double scale_mid_point = (scale_high - scale_low)/2;
        nodes[2*i].value = (nodes[2*i].value - scale_avg)/scale_mid_point;
        // scaling

        scale_low = scale_mtx.at<double>(2*i+1,0);
        scale_high = scale_mtx.at<double>(2*i+1,1);
        scale_avg = (scale_high + scale_low)/2;
        scale_mid_point = (scale_high - scale_low)/2;
        nodes[2*i+1].value = (nodes[2*i+1].value - scale_avg)/scale_mid_point;


    }

    for (int i = 0; i < 132; i++){
        cout << " " << nodes[i].value << " ";
    }

     nodes[132].index = -1;

/*
    nodes[0].index = 1;
    nodes[0].value = distance(p36,p48) / distance(p36,p45);
    nodes[0].index = 2;
    nodes[0].value = distance(p45,p54) / distance(p36,p45);
    nodes[0].index = 3;
    nodes[0].value = distance(p48,p54) / distance(p36,p45);
    nodes[0].index = 4;
    nodes[0].value = distance(p51,p57) / distance(p36,p45);
    nodes[5].index = -1;
*/
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
        if (sinx < 0.2 && sinx > -0.2){
            smileResult = svm_predict(smileSVM, nodes);
            angryResult = svm_predict(angrySVM, nodes);
            contemptResult = svm_predict(contemptSVM, nodes);
            disgustResult = svm_predict(disgustSVM, nodes);
            fearResult = svm_predict(fearSVM, nodes);
            sadnessResult = svm_predict(sadnessSVM, nodes);
            surpriseResult = svm_predict(surpriseSVM, nodes);
        }else{
            smileResult = 0;
            angryResult = 0;
            contemptResult = 0;
            disgustResult = 0;
            fearResult = 0;
            sadnessResult = 0;
            surpriseResult = 0;
        }
    }

    cout << "smile: " <<smileResult << endl;
    cout << "surprise: " <<surpriseResult << endl;
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
    spectrumVolumes.push_back((surpriseResult)*100);

    emit volumes(spectrumVolumes);
    emit smilePercentage(smileResult*100);
    if (smileResult*100>cutoff) emit smileDetected();

    //if (handCount > 10){
    if ( (blinkPoints.at(448) > 20) && still /*&& (roundness < 0.2)*/) emit handSelected(linex,liney);
    //}
    if (surpriseResult*100>50){
        emit surpriseSelected(linex,liney);
    }
    // Reading function execution time
    cout<<"processFrame execution time was: "<<processFrameTimer->elapsed()<<"ms"<<endl;

    int x = model._shape.at<double>(0,0) + center.x - 50;
    int y = model._shape.at<double>(19+66,0) - 50 + center.y;
    int width = model._shape.at<double>(16,0) - model._shape.at<double>(0,0) + 100;
    int height = model._shape.at<double>(8+66,0) - model._shape.at<double>(19+66,0) + 100;
    cout << "x: " << x << "y: " << y << "width: " << width << "height: " << height << endl;
    /*cvSetImageROI(&frameToShow,cvRect(x,y,width,height));
    IplImage* img_dst = cvCreateImage( cvSize( width, height ), frameToShow.depth, frameToShow.nChannels );
    cvCopy( &frameToShow, img_dst);
    cvResetImageROI(&frameToShow);
    cvResize(img_dst, &frameToShow, CV_INTER_CUBIC);
    cvReleaseImage(&img_dst);
    //cvRectangle(&frameToShow, cvPoint(x,y),cvPoint(x+width,y+height),cvScalar(255,255,255));
*/
    return true;
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







