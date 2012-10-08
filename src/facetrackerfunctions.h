#ifndef FACETRACKERFUNCTIONS_H
#define FACETRACKERFUNCTIONS_H

#include <CLM.h>
#include <FDet.h>
#include <FCheck.h>

void Draw(cv::Mat &image,cv::Mat &shape,cv::Mat &kon,cv::Mat &tri,cv::Mat &visi, bool drawNumbers)
{
  int i,n = shape.rows/2; cv::Point p1,p2; cv::Scalar c;

  //draw triangulation
  c = CV_RGB(0,0,0);
  for(i = 0; i < tri.rows; i++){
    if(visi.at<int>(tri.at<int>(i,0),0) == 0 ||
       visi.at<int>(tri.at<int>(i,1),0) == 0 ||
       visi.at<int>(tri.at<int>(i,2),0) == 0)continue;
    p1 = cv::Point(shape.at<double>(tri.at<int>(i,0),0),
           shape.at<double>(tri.at<int>(i,0)+n,0));
    p2 = cv::Point(shape.at<double>(tri.at<int>(i,1),0),
           shape.at<double>(tri.at<int>(i,1)+n,0));
    cv::line(image,p1,p2,c);
    p1 = cv::Point(shape.at<double>(tri.at<int>(i,0),0),
           shape.at<double>(tri.at<int>(i,0)+n,0));
    p2 = cv::Point(shape.at<double>(tri.at<int>(i,2),0),
           shape.at<double>(tri.at<int>(i,2)+n,0));
    cv::line(image,p1,p2,c);
    p1 = cv::Point(shape.at<double>(tri.at<int>(i,2),0),
           shape.at<double>(tri.at<int>(i,2)+n,0));
    p2 = cv::Point(shape.at<double>(tri.at<int>(i,1),0),
           shape.at<double>(tri.at<int>(i,1)+n,0));
    cv::line(image,p1,p2,c);
  }
  //draw connections
  c = CV_RGB(0,0,255);
  for(i = 0; i < kon.cols; i++){
    if(visi.at<int>(kon.at<int>(0,i),0) == 0 ||
       visi.at<int>(kon.at<int>(1,i),0) == 0)continue;
    p1 = cv::Point(shape.at<double>(kon.at<int>(0,i),0),
           shape.at<double>(kon.at<int>(0,i)+n,0));
    p2 = cv::Point(shape.at<double>(kon.at<int>(1,i),0),
           shape.at<double>(kon.at<int>(1,i)+n,0));
    cv::line(image,p1,p2,c,1);
  }
  //draw points

  for(i = 0; i < n; i++){
    if(visi.at<int>(i,0) == 0)continue;
    p1 = cv::Point(shape.at<double>(i,0),shape.at<double>(i+n,0));
    c = CV_RGB(255,0,0); cv::circle(image,p1,2,c);

    if (drawNumbers){
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, 8);

        std::ostringstream osstream;
        osstream << i;
        std::string string_x = osstream.str();
        putText(image, string_x, p1, 0, 0.3,cvScalar(255, 255, 255, 0));
    }
  }return;
}
//=============================================================================
int parse_cmd(int argc, const char** argv,
          char* ftFile,char* conFile,char* triFile,
          bool &fcheck,double &scale,int &fpd)
{
  int i; fcheck = false; scale = 1; fpd = -1;
  for(i = 1; i < argc; i++){
    if((std::strcmp(argv[i],"-?") == 0) ||
       (std::strcmp(argv[i],"--help") == 0)){
      std::cout << "track_face:- Written by Jason Saragih 2010" << std::endl
       << "Performs automatic face tracking" << std::endl << std::endl
       << "#" << std::endl
       << "# usage: ./face_tracker [options]" << std::endl
       << "#" << std::endl << std::endl
       << "Arguments:" << std::endl
       << "-m <string> -> Tracker model (default: ../model/face2.tracker)"
       << std::endl
       << "-c <string> -> Connectivity (default: ../model/face.con)"
       << std::endl
       << "-t <string> -> Triangulation (default: ../model/face.tri)"
       << std::endl
       << "-s <double> -> Image scaling (default: 1)" << std::endl
       << "-d <int>    -> Frames/detections (default: -1)" << std::endl
       << "--check     -> Check for failure" << std::endl;
      return -1;
    }
  }
  for(i = 1; i < argc; i++){
    if(std::strcmp(argv[i],"--check") == 0){fcheck = true; break;}
  }
  if(i >= argc)fcheck = false;
  for(i = 1; i < argc; i++){
    if(std::strcmp(argv[i],"-s") == 0){
      if(argc > i+1)scale = std::atof(argv[i+1]); else scale = 1;
      break;
    }
  }
  if(i >= argc)scale = 1;
  for(i = 1; i < argc; i++){
    if(std::strcmp(argv[i],"-d") == 0){
      if(argc > i+1)fpd = std::atoi(argv[i+1]); else fpd = -1;
      break;
    }
  }
  if(i >= argc)fpd = -1;
  for(i = 1; i < argc; i++){
    if(std::strcmp(argv[i],"-m") == 0){
      if(argc > i+1)std::strcpy(ftFile,argv[i+1]);
      else strcpy(ftFile,"/home/zoltan/FaceTracker/model/face2.tracker");
      break;
    }
  }
  if(i >= argc)std::strcpy(ftFile,"/home/zoltan/FaceTracker/model/face2.tracker");
  for(i = 1; i < argc; i++){
    if(std::strcmp(argv[i],"-c") == 0){
      if(argc > i+1)std::strcpy(conFile,argv[i+1]);
      else strcpy(conFile,"../model/face.con");
      break;
    }
  }
  if(i >= argc)std::strcpy(conFile,"/home/zoltan/FaceTracker/model/face.con");
  for(i = 1; i < argc; i++){
    if(std::strcmp(argv[i],"-t") == 0){
      if(argc > i+1)std::strcpy(triFile,argv[i+1]);
      else strcpy(triFile,"../model/face.tri");
      break;
    }
  }
  if(i >= argc)std::strcpy(triFile,"/home/zoltan/FaceTracker/model/face.tri");
  return 0;
}


#endif // FACETRACKERFUNCTIONS_H
