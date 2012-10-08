INCLUDEPATH += /usr/local/include/opencv/
INCLUDEPATH += /usr/include/
INCLUDEPATH += /home/zoltan/DeMoLib_v1_1_1/src/lib/
INCLUDEPATH += /home/zoltan/DeMoLib_v1_1_1/src/gui/
INCLUDEPATH += /home/zoltan/vxl/vcl/
INCLUDEPATH += /home/zoltan/vxl/vxl-1.14.0/core/
INCLUDEPATH += /home/zoltan/vxl/vxl-1.14.0/core/vnl
INCLUDEPATH += /home/zoltan/vxl/bin/vcl/
INCLUDEPATH += /home/zoltan/vxl/bin/core
INCLUDEPATH += /home/zoltan/vxl/vxl-1.14.0/vcl
INCLUDEPATH += /home/zoltan/vxl/vxl-1.14.0/v3p/netlib
INCLUDEPATH += /home/zoltan/vxl/core/
INCLUDEPATH += /home/zoltan/vxl/vxl-1.14.0/core
INCLUDEPATH += /home/zoltan/DeMoLib_v1_1_1/src/libsvm/
INCLUDEPATH += /home/zoltan/vxl/v3p/netlib/
INCLUDEPATH += /home/zoltan/FaceTracker/src/lib
INCLUDEPATH += /home/zoltan/libsvm-3.12
#INCLUDEPATH += /usr/src/linux-headers-3.2.0-30/include/linux/
#INCLUDEPATH += /usr/src/linux-headers-3.2.0-31/include/


LIBS += -L/usr/local/lib/
LIBS += -lopencv_core
LIBS += -lopencv_ml
LIBS += -lopencv_highgui
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_imgproc
LIBS += -lopencv_calib3d
LIBS += -lopencv_video
LIBS += -lopencv_features2d

LIBS += -L/home/zoltan/DeMoLib_v1_1_1/src/lib/
#LIBS += -lDeMoLib
#LIBS += -llibsvm

LIBS += -L/home/zoltan/vxl/lib
LIBS += -lbcal
LIBS += -lbdgl
LIBS += -lbdpg
LIBS += -lbgeo
LIBS += -lbgrl
LIBS += -lbgrl2
LIBS += -lbgrl2_algo
LIBS += -lbil
LIBS += -lbil_algo
LIBS += -lbmsh3d
LIBS += -lbmsh3d_algo
LIBS += -lbmsh3d_pro
LIBS += -lbnl
LIBS += -lbnl_algo
LIBS += -lbprb
#LIBS += -lbpro_batch
LIBS += -lbrdb
LIBS += -lbrip
LIBS += -lbrip_pro
LIBS += -lbsol
LIBS += -lbsta
LIBS += -lbsta_algo
LIBS += -lbsta_vis
LIBS += -lbsvg
LIBS += -lbsvg_pro
LIBS += -lbtol
LIBS += -lbugl
LIBS += -lbvgl_pro
LIBS += -lbvrml
LIBS += -lbxml
LIBS += -lclsfy
LIBS += -lexpatpp
LIBS += -lfhs
LIBS += -lgeotiff
LIBS += -lgevd
LIBS += -lipts
LIBS += -lm23d
LIBS += -lmbl
LIBS += -lmcal
LIBS += -lmfpf
LIBS += -lminizip
LIBS += -lmipa
LIBS += -lmmn
LIBS += -lmpeg2
LIBS += -lmsm
LIBS += -lmvl
LIBS += -lmvl2
LIBS += -lnetlib
LIBS += -losl
LIBS += -louel
LIBS += -louml
LIBS += -lpdf1d
LIBS += -lrgrl
LIBS += -lrply
LIBS += -lrrel
LIBS += -lrsdl
LIBS += -lsdet
LIBS += -lsdet_pro
LIBS += -lshapelib
LIBS += -ltestlib
LIBS += -lv3p_netlib
LIBS += -lvbl
LIBS += -lvbl_example_templates
LIBS += -lvbl_io
LIBS += -lvcl
LIBS += -lvcsl
LIBS += -lvdgl
LIBS += -lvdtop
LIBS += -lvepl
LIBS += -lvepl2
LIBS += -lvgl
LIBS += -lvgl_algo
LIBS += -lvgl_io
LIBS += -lvgl_xio
LIBS += -lvidl
LIBS += -lvidl_pro
LIBS += -lvifa
LIBS += -lvil
LIBS += -lvil1
LIBS += -lvil1_io
LIBS += -lvil3d
LIBS += -lvil3d_algo
LIBS += -lvil3d_io
LIBS += -lvil_algo
LIBS += -lvil_io
LIBS += -lvil_pro
LIBS += -lvimt
LIBS += -lvimt3d
LIBS += -lvimt_algo
LIBS += -lvipl
LIBS += -lvmap
LIBS += -lvnl
LIBS += -lvnl_algo
LIBS += -lvnl_io
LIBS += -lvnl_xio
LIBS += -lvo
LIBS += -lvpdfl
LIBS += -lvpgl
LIBS += -lvpgl_pro
LIBS += -lvpl
LIBS += -lvpyr
LIBS += -lvrml
LIBS += -lvsl
LIBS += -lvsol
LIBS += -lvtol
LIBS += -lvul
LIBS += -lvul_io

LIBS += -L/usr/local/lib

LIBS += -lgeotiff
#LIBS += -ltiff
LIBS += -ljpeg
LIBS += -lpng
LIBS += -lz
LIBS += -lm


LIBS += -L/usr/lib/
#LIBS += -L/lib/x86_64-linux-gnu/
#LIBS += -lusb-0.1
LIBS += -lusb
#LIBS += -ldc1394

OTHER_FILES += \
    EmotionRecognition.pro.user

HEADERS += \
    mainwindow.h \
    detectorthread.h \
    controlthread.h \
    capturethread.h \
    /home/zoltan/DeMoLib_v1_1_1/src/gui/fit.h \
    /home/zoltan/DeMoLib_v1_1_1/src/lib/DeMoLib_demo.h \
    structures.h \
    ../../../FaceTracker/src/lib/Tracker.h \
    ../../../FaceTracker/src/lib/PDM.h \
    ../../../FaceTracker/src/lib/PAW.h \
    ../../../FaceTracker/src/lib/Patch.h \
    ../../../FaceTracker/src/lib/IO.h \
    ../../../FaceTracker/src/lib/FDet.h \
    ../../../FaceTracker/src/lib/FCheck.h \
    ../../../FaceTracker/src/lib/CLM.h \
    ../../../libsvm-3.12/svm.h \
    plot.h \
    emospectrum.h \
    headposeindicator.h \
    automata.h \
    facetrackerfunctions.h
SOURCES += \
    mainwindow.cpp \
    main.cpp \
    detectorthread.cpp \
    controlthread.cpp \
    capturethread.cpp \
    /home/zoltan/vxl/vxl-1.14.0/core/vnl/algo/vnl_cholesky.cxx \
    /home/zoltan/vxl/vxl-1.14.0/core/vnl/algo/Templates/vnl_svd+double-.cxx \
    /home/zoltan/vxl/vxl-1.14.0/core/vnl/algo/Templates/vnl_qr+double-.cxx \
    /home/zoltan/vxl/vxl-1.14.0/core/vnl/Templates/vnl_vector+double-.cxx \
    ../../../FaceTracker/src/lib/Tracker.cc \
    ../../../FaceTracker/src/lib/PDM.cc \
    ../../../FaceTracker/src/lib/PAW.cc \
    ../../../FaceTracker/src/lib/Patch.cc \
    ../../../FaceTracker/src/lib/IO.cc \
    ../../../FaceTracker/src/lib/FDet.cc \
    ../../../FaceTracker/src/lib/FCheck.cc \
    ../../../FaceTracker/src/lib/CLM.cc \
    ../../../libsvm-3.12/svm.cpp \
    plot.cpp \
    emospectrum.cpp \
    headposeindicator.cpp \
    hid_LINUX.c \
    automata.cpp

FORMS += \
    mainwindow.ui
