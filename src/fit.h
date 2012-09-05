#ifndef FIT_H
#define FIT_H

#include <DeMoLib_demo.h>
#include <DeMoLib_io.h>

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

#ifdef _EiC
#define WIN32
#endif


//=============================================================================
/** A class for visualising deformable model fitting.*/
class DeMoLib_fit_gui{
public:
  int                _flag; /**< Drawing flag                */
  vnl_vector<double> _pose; /**< Deformable model parameters */

  int  x0_,y0_;               /**< Global access for pose            */
  bool rotate_;               /**< Global access for rotation        */
  bool trans_;                /**< Global access for translation     */
  vcl_vector<vcl_string> fname;
  bool ascii_;
    DeMoLib_demo_pyrd        __model;       /**< Pyramid of deformable models  */
  /**
     Constructor which initialises everyting.
     @param fname Filename of appearance model.
  */

  DeMoLib_fit_gui(vcl_vector<vcl_string>& fnames,
		  vcl_string imFname,int filter,
          bool ascii){

      fname = fnames;
      ascii_ = ascii;
    _flag = 0;
    __cvImage = 0;
    __model.Init(fname,ascii_);
    __model.ZeroAllParameters();
    __model.GetPoseParameters(_pose,0);
    if(this->LoadImage(imFname,filter) < 0)vcl_abort();
    __model.GetPoseParameters(_pose,0);

    if(_pose.size() == 4){ //2D
      _pose(2) = __vxlImage.ni()/2;
      _pose(3) = __vxlImage.nj()/2;
    }
    else if(_pose.size() == 7){ //3D
      _pose(4) = __vxlImage.ni()/2;
      _pose(5) = __vxlImage.nj()/2;
    }
    else{
      vul_printf(vcl_cerr, 
		 "ERROR(%s,%d) : Unsupported pose vector!\n", 
		 __FILE__,__LINE__);vcl_abort();
    }
    __gui  = false;
  }
  /** Destructor that releases everything. */
  ~DeMoLib_fit_gui(){
    if(__gui)cvDestroyWindow("Deformable Model Fitting");
    cvReleaseImage(&__cvImage);
  }
  /** Get current image width.*/
  inline int ImageWidth(){return __vxlImage.ni();}

  /** Get current image height. */
  inline int ImageHeight(){return __vxlImage.nj();}
  /**
     Save image with model drawn on to a file.
     @param imFname Filename where image will be written to.
     @return -1 on failure, 0 otherwise.
  */
  inline int SaveImage(vcl_string imFname){
    return cvSaveImage(imFname.c_str(),__cvImage);
  }
  /** Set tolerance on change of parameters */
  inline void SetParamChangeTol(const double pTol){__fparam._pTol = pTol;}
  
  /** Set tolerance on change of function */
  inline void SetFunctChangeTol(const double fTol){__fparam._fTol = fTol;}
  
  /** Set maximum iterations/pyramid level */
  inline void SetMaxIter(const unsigned int iTol){__fparam._iTol = iTol;}
  
  /** Set iteration decrement throughout pyramid levels */
  inline void SetDecIter(const double iDec){__fparam._iDec = iDec;}

  /** Set fitting flag*/
  inline void SetFitFlag(const unsigned int f){__fparam._flag = f;}

  /**
     Load image to fit from file.
     @param  imFname Filename of image.
     @return -1 on failure, 0 otherwise.
  */
  int LoadImage(vcl_string imFname,int filter){
    if(__cvImage != 0)cvReleaseImage(&__cvImage);
    __cvImage  = cvLoadImage(imFname.c_str());
    if(__cvImage == 0){
      vul_printf(vcl_cerr, 
		 "ERROR(%s,%d) : Failed loading image from %s\n", 
		 __FILE__,__LINE__,imFname.c_str());return -1;
    }
    __drawImage.set_to_memory((vxl_byte*)__cvImage->imageData, 
			      __cvImage->width,__cvImage->height,
			      __cvImage->nChannels,__cvImage->nChannels,
			      __cvImage->nChannels*__cvImage->width,1);
    __cleanImage.deep_copy(__drawImage);
    __vxlImage = vil_load(imFname.c_str());
    //__vxlImage.deep_copy(__drawImage);
    this->Filter(filter);
    return 0;
  }

  void LoadImageMemo(IplImage* frame, int filter){
      __cvImage  = frame;
      if(__cvImage == 0){
          std::cout<<"loadImage hiba"<<std::endl;
      }
      __drawImage.set_to_memory((vxl_byte*)__cvImage->imageData,
                    __cvImage->width,__cvImage->height,
                    __cvImage->nChannels,__cvImage->nChannels,
                    __cvImage->nChannels*__cvImage->width,1);
      __cleanImage.deep_copy(__drawImage);

      cvReleaseImage(&__cvDarkImage);
      __cvDarkImage = cvCreateImage( cvGetSize(frame), frame->depth,frame->nChannels);
      cvSetZero(__cvDarkImage);
      __darkImage.set_to_memory((vxl_byte*)__cvDarkImage->imageData,
                    __cvDarkImage->width,__cvDarkImage->height,
                    __cvDarkImage->nChannels,__cvDarkImage->nChannels,
                    __cvDarkImage->nChannels*__cvDarkImage->width,1);
      __cleanDarkImage.deep_copy(__darkImage);

      //__model.GetPoseParameters(_pose,0);

      __vxlImage.deep_copy(__drawImage);
      this->Filter(filter);
  }

  int LoadImage(IplImage* frame, int filter, int x, int y, double width, double height){
      __model.ZeroAllParameters();
      __model.GetPoseParameters(_pose,0);
      _pose(0) = width;
      _pose(1) = height;
      _pose(2) = x;
      _pose(3) = y;

          __model.SetPoseParameters(_pose,0);
    //if(__cvImage != 0)cvReleaseImage(&__cvImage);
    __cvImage  = frame;
    if(__cvImage == 0){
        std::cout<<"loadImage hiba"<<std::endl;
    }
    __drawImage.set_to_memory((vxl_byte*)__cvImage->imageData,
                  __cvImage->width,__cvImage->height,
                  __cvImage->nChannels,__cvImage->nChannels,
                  __cvImage->nChannels*__cvImage->width,1);
    __cleanImage.deep_copy(__drawImage);

    cvReleaseImage(&__cvDarkImage);
    __cvDarkImage = cvCreateImage( cvGetSize(frame), frame->depth,frame->nChannels);
    cvSetZero(__cvDarkImage);
    __darkImage.set_to_memory((vxl_byte*)__cvDarkImage->imageData,
                  __cvDarkImage->width,__cvDarkImage->height,
                  __cvDarkImage->nChannels,__cvDarkImage->nChannels,
                  __cvDarkImage->nChannels*__cvDarkImage->width,1);
    __cleanDarkImage.deep_copy(__darkImage);

    //__model.GetPoseParameters(_pose,0);

    __vxlImage.deep_copy(__drawImage);
    this->Filter(filter);
    return 0;
  }

  /** Fit to image */
  void Fit(){
    __fparam._im = vil_new_image_resource_of_view(__vxlImage);
    __model.SetPoseParameters(_pose,0);
    __model.TransferParams(0);
    __model.Fit(&__fparam);
    __model.GetPoseParameters(_pose,0);
    //this->Draw();
    return;
  }
  /** Displays model.*/
  inline void GUI(){
              std::cout<<"GUI"<<std::endl;
    cvNamedWindow("Deformable Model Fitting",1);
    __gui = true;  this->Draw(); this->Show();
    rotate_ = false;
    trans_  = false;
  }
  /**
     Show image in window.
     @return -1 of GUI is not on, 0 otherwise.
  */
  inline int Show(){
    if(!__gui)return -1;
    cvShowImage("Deformable Model Fitting",__cvImage);
    return 0;
  }
  /** Draw current model on image*/
  void Draw(){
    __drawImage.deep_copy(__cleanImage);
    __model.SetPoseParameters(_pose,0);
    __model.Draw(__drawImage,_flag);
    __darkImage.deep_copy(__cleanDarkImage);
    __model.SetPoseParameters(_pose,0);
    __model.Draw(__darkImage,_flag);
  }

  int WriteShape(vcl_string fname){
    vnl_vector<double> s;
    __model.GetShape(s,0); return DeMoLib_io::WriteShapeFile(fname,s);
  }

  IplImage* getCvImage(){
    return __cvDarkImage;
  }

private:
  bool                     __gui;         /**< GUI on?                       */
  IplImage*                __cvImage;     /**< OpenCV image                  */
  IplImage*                __cvDarkImage;     /**< OpenCV image                  */
  vil_image_view<vxl_byte> __vxlImage;    /**< VXL image                     */
  vil_image_view<vxl_byte> __drawImage;   /**< VXL image to draw on          */
  vil_image_view<vxl_byte> __cleanImage;  /**< VXL image with no processing  */
  vil_image_view<vxl_byte> __darkImage;  /**< VXL image with no processing  */
  vil_image_view<vxl_byte> __cleanDarkImage;  /**< VXL image with no processing  */
  DeMoLib_demo_FitParam    __fparam;      /**< Fitting parameters            */

  /**
     Filtering operation
     @param filter 0=raw,1=greyscale.
     @return       -1 on failure, 0 otherwise.
  */
  int Filter(int filter){
    switch(filter){
    case 0:break;
    case 1:this->FilterGrey();break;
    default:
      vul_printf(vcl_cerr, 
		 "ERROR(%s,%d) : Unknown filter type %d\n", 
		 __FILE__,__LINE__,filter);
      return -1;
    }
    return 0;
  }
  /** Perform greyscale filtering on image.*/
  void FilterGrey(){
    int i,j,k;
    double v;
    vil_image_view<vxl_byte> dst(__vxlImage.ni(),__vxlImage.nj());
    for(i = 0; i < (int)__vxlImage.ni(); i++){
      for(j = 0; j < (int)__vxlImage.nj(); j++){
	for(k = 0,v = 0.0; k < (int)__vxlImage.nplanes(); k++)
	  v += __vxlImage(i,j,k);
	dst(i,j) = vxl_byte(v/__vxlImage.nplanes());
      }
    }
    __vxlImage = dst;
    return;
  }
};

#endif
