QT       += core gui network widgets

INCLUDEPATH += /usr/include/opencv/
INCLUDEPATH += /usr/include/
INCLUDEPATH += /usr/include/qt5/QtWidgets
INCLUDEPATH += /usr/include/qt5
INCLUDEPATH += /home/zoltan/emorec/FaceTracker/src/lib
INCLUDEPATH += /home/zoltan/emorec/libsvm-3.12
INCLUDEPATH += /usr/include/opencv2/imgproc


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

LIBS += -L/usr/local/lib

#LIBS += -lgeotiff
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
    structures.h \
    ../FaceTracker/src/lib/Tracker.h \
    ../FaceTracker/src/lib/PDM.h \
    ../FaceTracker/src/lib/PAW.h \
    ../FaceTracker/src/lib/Patch.h \
    ../FaceTracker/src/lib/IO.h \
    ../FaceTracker/src/lib/FDet.h \
    ../FaceTracker/src/lib/FCheck.h \
    ../FaceTracker/src/lib/CLM.h \
    ../libsvm-3.12/svm.h \
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
    ../FaceTracker/src/lib/Tracker.cc \
    ../FaceTracker/src/lib/PDM.cc \
    ../FaceTracker/src/lib/PAW.cc \
    ../FaceTracker/src/lib/Patch.cc \
    ../FaceTracker/src/lib/IO.cc \
    ../FaceTracker/src/lib/FDet.cc \
    ../FaceTracker/src/lib/FCheck.cc \
    ../FaceTracker/src/lib/CLM.cc \
    ../libsvm-3.12/svm.cpp \
    plot.cpp \
    emospectrum.cpp \
    headposeindicator.cpp \
    hid_LINUX.c \
    automata.cpp

FORMS += \
    mainwindow.ui
