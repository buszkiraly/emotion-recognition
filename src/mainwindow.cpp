/*

    A QFileDialogoknál kell beállítani, hogy melyik mappát nyissa meg, illetve a MainWindow konstruktorában adhatsz meg egy másik képet, amit betölt a Labelek helyére


*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "iostream"
#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include <QLabel>
#include <QTime>
#include <QPlainTextEdit>
#include <QTime>
#include <QFileDialog>
#include <QMessageBox>
#include "structures.h"
#include "QProcess"

using namespace std;

extern DetectorThread   *det;
extern CaptureThread    *cap;
extern ControlThread    *con;

// Time measurement
QTime *functionTimer;
extern QTime *signalTime;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Time measurement
    functionTimer = new QTime();


    ui->pushButton_11->setVisible(false);
    ui->blackA->setVisible(false);
    ui->recordA->setVisible(false);
    ui->contrastLabel->setVisible(false);
    ui->contrastSizeA->setVisible(false);

    ui->progressBar->setValue(0);

    QObject::connect( this, SIGNAL( points(std::vector<int>,std::vector<int>) ) ,
        ui->plotLabel, SLOT( points(std::vector<int>,std::vector<int>) ));

    QObject::connect( this, SIGNAL( drawPlot()) ,
        ui->plotLabel, SLOT( paintEvent(QPaintEvent*)) );

    QObject::connect( this, SIGNAL( emoVolumes(std::vector<int>)) ,
        ui->spectrumLabel, SLOT( volumes(std::vector<int>)) );

    QObject::connect( this, SIGNAL( outGoingHeadPoseCoords(double,double)) ,
        ui->headPoseLabel, SLOT( coord(double,double)) );

    std::vector<int> myVolumes;
    myVolumes.push_back(10);
    myVolumes.push_back(20);
    myVolumes.push_back(30);
    myVolumes.push_back(40);

    emit emoVolumes(myVolumes);

    det_timer = new QTime();
    cap_timer = new QTime();

    det_framenumber = 0;
    cap_framenumber = 0;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clicked_A(){
    append("a");
}

void MainWindow::clicked_B(){
    append("b");
}

void MainWindow::clicked_C(){
    append("c");
}

void MainWindow::clicked_1(){
    append("1");
}

void MainWindow::clicked_2(){
    append("2");
}

void MainWindow::clicked_3(){
    append("3");
}

void MainWindow::clicked_4(){
    append("4");
}

void MainWindow::clicked_5(){
    append("5");
}

void MainWindow::clicked_6(){
    append("6");
}

void MainWindow::clicked_Kesz(){
    emit readyToSmile(ui->chocolate->toPlainText());
}

void MainWindow::blackToggle(bool state){

    emit blackToggled(state);
}

void MainWindow::clickedWriteShape(){
    emit writeShape();
}

void MainWindow::clickedCamera(){
    emit cameraSource();
}

void MainWindow::clickedVideo(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "/home/zoltan",
                                                     tr("Videos (*.avi *.ogv *.mpeg)"));
    if (fileName.isEmpty()) return;

    emit videoSource(fileName);
}

void MainWindow::clickedPicture(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "/home/zoltan",
                                                     tr("Images (*.jpg *.bmp *.gif)"));
    if (fileName.isEmpty()) return;

    emit pictureSource(fileName);
}

void MainWindow::append(const char* toAppend){

    QString *qAppend = new QString( toAppend );

    if ( (toAppend[0] == 'a') || (toAppend[0] == 'b') || (toAppend[0] == 'c') ){
        ui->chocolate->setPlainText(*qAppend);
    }
    else{
        if (ui->chocolate->toPlainText().size() == 1){
            ui->chocolate->setPlainText(ui->chocolate->toPlainText() + *qAppend);
        }
    }

}

void MainWindow::setProgressBar(int value){
    ui->progressBar->setValue(value);
}


void MainWindow::setDetectedImage(IplImage *img){

    // Reading signal transition time
    cout<<"Detected image singal transition time was: "<<signalTime->elapsed()<<"ms"<<endl;

    if (!img) return;

    // Starting time measurement
    functionTimer->start();

    int width = ui->labelDet->geometry().width();
    int height = ui->labelDet->geometry().height();
    IplImage *frame = cvCreateImage( cvSize(width,height ), img->depth, img->nChannels );

    cvResize(img, frame);
    cvCvtColor(frame,frame,CV_BGR2RGB);

    QImage* image = new QImage((unsigned char *)frame->imageData,frame->width,frame->height,QImage::Format_RGB888);

    det_framenumber++;

    if (det_framenumber == 5){
        det_elapsed = det_timer->elapsed();
        if (det_elapsed) {
            det_fps = 5000/det_elapsed;
            det_fps_str = det_fps_str.setNum(det_fps);
        }else{
            det_fps_str = "-";
        }
        det_framenumber = 0;
        det_timer->start();
    }

    ui->label_det_fps->setText("FPS: " + det_fps_str);

    ui->labelDet->setPixmap(QPixmap::fromImage(*image));
    ui->labelDet->show();

    delete image;
    cvReleaseImage(&frame);
    cvReleaseImage(&img);

    // Reading execution time
    cout<<"setDetectedImage execution time was: "<<functionTimer->elapsed()<<"ms"<<endl;
}

void MainWindow::setCapturedImage(IplImage*  img){

    if (!img) return;

    // Starting time measurement
    functionTimer->start();

    int width = ui->labelCap->geometry().width();
    int height = ui->labelCap->geometry().height();
    IplImage *frame = cvCreateImage( cvSize(width,height ), img->depth, img->nChannels );

    cvResize(img, frame);
    cvCvtColor(frame,frame,CV_BGR2RGB);

    QImage* image = new QImage((unsigned char *)frame->imageData,frame->width,frame->height,QImage::Format_RGB888);

    cap_framenumber++;

    if (cap_framenumber == 5){
        cap_elapsed = cap_timer->elapsed();
        if (cap_elapsed) {
            cap_fps = 5000/cap_elapsed;
            cap_fps_str = cap_fps_str.setNum(cap_fps);
        }else{
            cap_fps_str = "-";
        }
        cap_framenumber = 0;
        cap_timer->start();
    }

    ui->label_cap_fps->setText("FPS: " + cap_fps_str);

    ui->labelCap->setPixmap(QPixmap::fromImage(*image));
    ui->labelCap->show();

    delete image;
    cvReleaseImage(&frame);
    cvReleaseImage(&img);

    // Reading execution time
    cout<<"setCapturedImage execution time was: "<<functionTimer->elapsed()<<"ms"<<endl;
}

void MainWindow::smilePercentage(int perc){
    ui->smileProgress->setValue(perc);
}

void MainWindow::clickedStop(){
    if (!strcmp(ui->stopButton->text().toStdString().c_str(),"Állj")){
            ui->stopButton->setText("Lejátszás");
            emit stopCapture(true);
    }else{
        ui->stopButton->setText("Állj");
        emit stopCapture(false);
    }
}

void MainWindow::frameCount(double n,double m){
    QString progress, p2;

    progress = progress.setNum(n);
    progress.append("/");
    p2 = p2.setNum(m);
    progress.append(p2);
    if (!ui->frameCounter->isSliderDown()) ui->frameCounter->setValue(ui->frameCounter->maximum()*n/m);
    ui->videoProgress->setText(progress);
}

void MainWindow::framePos(){
    emit seek(ui->frameCounter->value());
}

void MainWindow::error(char * errorText){
    QMessageBox::warning(this, tr("Emotion Recognition"),
                                    tr(errorText),
                                    QMessageBox::Ok );
}

void MainWindow::disableFrameCounter(){
    ui->frameCounter->setVisible(false);
}

void MainWindow::enableFrameCounter(){
    ui->frameCounter->setVisible(true);
}

void MainWindow::on_rewindButton_clicked()
{
    emit rewind();
}

void MainWindow::on_forwardButton_clicked()
{
    emit forward();
}

void MainWindow::on_contrastSizeA_valueChanged(int arg1)
{
    emit contrastSizeChanged(arg1);
}

void MainWindow::incomingPoints(vector<int> plotPoints,vector<int> plotPoints2){

    emit points(plotPoints,plotPoints2);

}

void MainWindow::incomingVolumes(vector<int> volumes){
    emit emoVolumes(volumes);
}

void MainWindow::incomingHeadPoseCoords(double x, double y)
{
    emit outGoingHeadPoseCoords(x,y);
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    emit smileCutOff(value);
}

void MainWindow::on_pushButton_15_clicked()
{
    emit resetModel();
}

void MainWindow::on_checkBox_clicked()
{

}

void MainWindow::on_checkBox_toggled(bool checked)
{
    emit pointsAnnotations(checked);
}

void MainWindow::lookedAtSlot(QString slot){
    if (slot == NULL) {
        ui->lookedChocolateLabel->setText("Out of range");
        return;
    }
    ui->lookedChocolateLabel->setText(slot);
}

void MainWindow::selectedSlot(QString slot){
    if (slot == NULL) {
        ui->lookedChocolateLabel->setText("Out of range");
        return;
    }
    ui->selectedChocolateLabel->setText(slot);
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    emit selectionAlgorithm(index);
}

void MainWindow::waitingForSmile(bool waiting){
    if (waiting) ui->label_8->setText("Igen");
    else ui->label_8->setText("Nem");
}

void MainWindow::capDevs(std::vector<std::string> devices){
    for (std::vector<std::string>::iterator it = devices.begin(); it<devices.end(); ++it){
        std::string str = *it;
        ui->comboBox_2->addItem(str.c_str());
        cout<<str<<endl;
    }
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    emit selectedDevice(index-1);
}

