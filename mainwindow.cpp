#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include "math.h"
#include <cmath>
#include <QImage>
#include <QMessageBox>
#include <thread>


using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setColors();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnAddImage_clicked()
{

    QString filename = QFileDialog::getOpenFileName(this, tr("Choose"), "", tr("Images (*.png)"));

    if ( QString::compare(filename, QString()) != 0 )
    {
        QImage image;
        bool valid = image.load(filename);

        if (valid)
        {
            isImageSelected = true;
            loadedImage = image;
            ui->lblImage->setPixmap(QPixmap::fromImage(image));
        }
        else {
                cout << "Error, there is no image!" << endl;
        }
    }

}


void MainWindow::on_btnConvert_clicked()
{
    if (isImageSelected && !loadedImage.isNull()) {
        QImage img = loadedImage.copy();

        if ( isEdgesChecked == true ) detectEdges(img);

        if ( isColorReductionChecked == true ) paintRetro(img);

        if(ui -> pixelizeFirst->isChecked()){
            if ( isPixelizeChecked == true ) paintPixelization(img);
            if ( isEdgesChecked == true ) paintEdges(img);
        }
        if(ui -> edgesFirst->isChecked()){
            if ( isEdgesChecked == true ) paintEdges(img);
            if ( isPixelizeChecked == true ) paintPixelization(img);
        }
        ui->lblConverted->setPixmap(QPixmap::fromImage(img));

        modifiedImage = new QImage();
        *modifiedImage = img.copy();

    }
}

void MainWindow::paintPixelization(QImage& Source){
    int it = ui->pixelizeSlider->value();
    for(int i = 0; i < it; i++)
        paintPixelizationStep(Source);
    toSize(Source, it);
}

void MainWindow::paintPixelizationStep(QImage& Source){
    numberOfThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
    std::thread *tt = new std::thread[static_cast<unsigned>(numberOfThreads)];
    SrcWidth = Source.width();
    SrcHeight = Source.height();
    TgtWidth = SrcWidth / 2 +1;
    TgtHeight = SrcHeight / 2 +1;
    Resoult =  QImage(TgtWidth, TgtHeight, QImage::Format::Format_RGBA8888_Premultiplied );
    for (int i = 0; i < static_cast<int>(numberOfThreads); ++i){
        tt[i] = std::thread(&MainWindow::taskPixelization, this, std::ref(Source), i);
        //task(std::ref(Source), i);
    }
    taskPixelization(std::ref(Source), numberOfThreads);

        for (int i = 0; i < numberOfThreads; ++i)
        {
            tt[i].join();
        }

    Source = Resoult;

}

void MainWindow::taskPixelization(QImage& Source, int k){
    int part = TgtWidth / numberOfThreads;
    int stop;
    if(k < numberOfThreads){
        stop = (k+1)*part;
    }
    else{
        stop = TgtWidth;
    }
    for (int x = k*part; x < stop; x++) {
        int x2 = 2 * x;
        for (int y = 0; y < TgtHeight; y++) {
            int y2 = 2 * y;
            QColor color;
            color = pixelizationColor(std::ref(Source), x2, y2);
            Resoult.setPixelColor(x, y, color);
        }
    }
}

QColor MainWindow::pixelizationColor(QImage& Source, int x, int y){
    int r = 255, g = 255, b = 255, a = 255;
    if(x < Source.width()-1 && y < Source.height()-1 )
         a = Source.pixelColor(x, y).alpha();
    if(a != 0){
        if(x < Source.width()-1 && y < Source.height()-1 ){
            r = (Source.pixelColor(x, y).red() + Source.pixelColor(x, y+1).red()+Source.pixelColor(x+1, y).red() + Source.pixelColor(x+1, y+1).red())/4;
            g = (Source.pixelColor(x, y).green() + Source.pixelColor(x, y+1).green()+Source.pixelColor(x+1, y).green() + Source.pixelColor(x+1, y+1).green())/4;
            b = (Source.pixelColor(x, y).blue() + Source.pixelColor(x, y+1).blue()+Source.pixelColor(x+1, y).blue() + Source.pixelColor(x+1, y+1).blue())/4;
        }
        else if(y < Source.height()-1 ){
            r = (Source.pixelColor(x-1, y).red() + Source.pixelColor(x-1, y+1).red())/2;
            g = (Source.pixelColor(x-1, y).green() + Source.pixelColor(x-1, y+1).green())/2;
            b = (Source.pixelColor(x-1, y).blue() + Source.pixelColor(x-1, y+1).blue())/2;
        }
        else if(x < Source.width()-1 ){
            r = (Source.pixelColor(x, y-1).red() + Source.pixelColor(x+1, y-1).red())/2;
            g = (Source.pixelColor(x, y-1).green() + Source.pixelColor(x+1, y-1).green())/2;
            b = (Source.pixelColor(x, y-1).blue() + Source.pixelColor(x+1, y-1).blue())/2;
        }
        return QColor(r, g, b, 255);
    }
    else {
        return QColor(255,255,255,0);
    }
}

void MainWindow::toSize(QImage& Source, int it){
    it = static_cast<int>(pow(2, it));
    int SrcWidth = Source.width();
    int SrcHeight = Source.height();
    int TgtWidth = loadedImage.width(), TgtHeight = loadedImage.height();
    QImage Resoult(TgtWidth, TgtHeight, QImage::Format::Format_RGBA8888_Premultiplied);
    for(int x = 0; x < SrcWidth; x++){
        for(int y = 0; y < SrcHeight; y++){
            for(int x1 = 0; x1 < it; x1++){
                for(int y1 = 0; y1 < it; y1++){
                    if(x*it+x1 < TgtWidth && y*it+y1 < TgtHeight){
                    Resoult.setPixelColor(x*it+x1, y*it+y1, Source.pixelColor(x,y));
                    }
               }
            }
        }
    }
    Source = Resoult;
}

void MainWindow::paintRetro(QImage & img) {
    numberOfThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
    std::thread *tt = new std::thread[static_cast<unsigned>(numberOfThreads)];

    for (int i = 0; i < static_cast<int>(numberOfThreads); ++i){
        tt[i] = std::thread(&MainWindow::taskRetro, this, std::ref(img), i);
        //task(std::ref(Source), i);
    }
    taskRetro(std::ref(img), numberOfThreads);

    for (int i = 0; i < numberOfThreads; ++i)
    {
        tt[i].join();
    }
}

void MainWindow::taskRetro(QImage& img, int k ){
    int colorIndex, maxDelta = 765, tmpRed = 255, tmpBlue = 255, tmpGreen = 255;
    int part = img.width() / numberOfThreads;
    int stop;
    if(k < numberOfThreads){
        stop = (k+1)*part;
    }
    else{
        stop = img.width();
    }
    for (int x = k*part; x < stop; x++) {
        for (int y = 0; y < img.height(); y++) {
            colorIndex = 0;
            maxDelta = 765;
            tmpRed = 255;
            tmpBlue = 255;
            tmpGreen = 255;

            for (int i = 0; i < ui->retroSlider->value(); i++) {
                tmpRed = abs(img.pixelColor(x,y).red() - arrColors[i].red());
                tmpGreen = abs(img.pixelColor(x,y).green() - arrColors[i].green());
                tmpBlue = abs(img.pixelColor(x,y).blue() - arrColors[i].blue());
                if ( tmpRed + tmpBlue + tmpGreen < maxDelta) {
                        maxDelta = abs(tmpRed + tmpBlue + tmpGreen);
                        colorIndex = i;
                }
            }

            QColor newColor = arrColors[colorIndex];
            newColor.setAlpha(img.pixelColor(x,y).alpha());
            img.setPixelColor(x,y,newColor);
        }
    }
}




void MainWindow::paintEdges(QImage& img) {
    numberOfThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
    std::thread *tt = new std::thread[static_cast<unsigned>(numberOfThreads)];
    for (int i = 0; i < static_cast<int>(numberOfThreads); ++i){
        tt[i] = std::thread(&MainWindow::taskPEdges, this, std::ref(img), i);
        //task(std::ref(Source), i);
    }
    taskPEdges(std::ref(img), numberOfThreads);

    for (int i = 0; i < numberOfThreads; ++i)
    {
        tt[i].join();
    }
}

void MainWindow::taskPEdges(QImage& img, int k ){
    int thicness = ui->thicknessSlider->value();
    int part = img.width() / numberOfThreads;
    int stop;
    if(k < numberOfThreads){
        stop = (k+1)*part;
        if(stop > img.width())
            stop = img.width();
    }
    else{
        stop = img.width();
    }
    for (int i = k*part; i < stop; i++) {
        for (int j = 0; j < img.height(); j++) {
            if ( thicness == 1 ) {
                if ( contourMask[i][j] == 'b') {
                    img.setPixelColor(i,j,arrColors[0]);
                }
            }

            if ( thicness == 2 ) {
                if ( contourMask[i][j] == 'b') {
                    img.setPixelColor(i,j,arrColors[0]);
                    if(i<img.width())
                        img.setPixelColor(i+1,j,arrColors[0]);
                    if(i>0)
                        img.setPixelColor(i-1,j,arrColors[0]);
                    if(j<img.width())
                        img.setPixelColor(i,j+1,arrColors[0]);
                    if(j>0)
                        img.setPixelColor(i,j-1,arrColors[0]);
                }
            }
            if ( thicness == 3 ) {
                if ( contourMask[i][j] == 'b') {
                    img.setPixelColor(i,j,arrColors[0]);
                    if(i<img.width())
                        img.setPixelColor(i+1,j,arrColors[0]);
                    if(i>0)
                        img.setPixelColor(i-1,j,arrColors[0]);
                    if(j<img.width())
                        img.setPixelColor(i,j+1,arrColors[0]);
                    if(j>0)
                        img.setPixelColor(i,j-1,arrColors[0]);
                    if(i<img.width()+1)
                        img.setPixelColor(i+2,j,arrColors[0]);
                    if(i>1)
                        img.setPixelColor(i-2,j,arrColors[0]);
                    if(j<img.width()+1)
                        img.setPixelColor(i,j+2,arrColors[0]);
                    if(j>1)
                        img.setPixelColor(i,j-2,arrColors[0]);
                }
            }
        }
    }
}


//detect edges
void MainWindow::detectEdges(QImage& img) {
    numberOfThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
    std::thread *tt = new std::thread[static_cast<unsigned>(numberOfThreads)];
    contourMask = new char*[img.width()];

    for(int i = 0; i < img.width(); ++i)
        contourMask[i] = new char[img.height()];


    for (int i = 0; i < static_cast<int>(numberOfThreads); ++i){
        tt[i] = std::thread(&MainWindow::taskDEdges, this, std::ref(img), i);
        //task(std::ref(Source), i);
    }
    //taskDEdges(std::ref(img), numberOfThreads);

    for (int i = 0; i < numberOfThreads; ++i)
    {
        tt[i].join();
    }
}

void MainWindow::taskDEdges(QImage& img, int k ){
    QColor backgroundColor = img.pixelColor(1,1);
    int part = img.width() / numberOfThreads;
    int stop;
    if(k < numberOfThreads){
        stop = (k+1)*part;
    }
    else{
        stop = img.width();
    }
    for (int i = k*part; i < stop; i++) {
        for (int j = 0; j < img.height(); j++) {
            if (img.pixelColor(i,j) != backgroundColor && ( (j>0 && img.pixelColor(i,j-1) == backgroundColor) || ( j<img.height() && img.pixelColor(i,j+1) == backgroundColor) || ( i>0 && img.pixelColor(i-1,j) == backgroundColor) || (i<img.width() && img.pixelColor(i+1,j) == backgroundColor ))) {
                contourMask[i][j] = 'b';
            }
            else{
                contourMask[i][j] = 'n';
            }
        }
    }
}

void MainWindow::setColors() {

        //Sixteen Predefined Colors in HTML
        arrColors[0] = QColor(0,0,0); //Black
        arrColors[1] = QColor(0,255,255); //Aqua
        arrColors[2] = QColor(0,0,255); //Blue
        arrColors[3] = QColor(255,0,255); //Fuchsia (pink)
        arrColors[4] = QColor(204,204,204); //Gray
        arrColors[5] = QColor(0,204,0); //Green
        arrColors[6] = QColor(0,255,0); //Lime
        arrColors[7] = QColor(204,0,0); //Maroon
        arrColors[8] = QColor(0,0,204); //Navy
        arrColors[9] = QColor(204,204,0); //Olive
        arrColors[10] = QColor(204,0,204); //Purple
        arrColors[11] = QColor(255,0,0); //Red
        arrColors[12] = QColor(180,180,180); //Silver
        arrColors[13] = QColor(0,204,204); //Teal
        arrColors[14] = QColor(204,204,0); //Yellow
        arrColors[15] = QColor(255,255,255); //White

        //darker colors of these above
        arrColors[16] = QColor(100,100,100);
        arrColors[17] = QColor(0,120,120);
        arrColors[18] = QColor(0,0,120);
        arrColors[19] = QColor(120,0,120);
        arrColors[20] = QColor(70,70,70);
        arrColors[21] = QColor(0,120,0);
        arrColors[22] = QColor(0,155,0);
        arrColors[23] = QColor(120,0,0);
        arrColors[24] = QColor(0,0,120);
        arrColors[25] = QColor(120,120,0);
        arrColors[26] = QColor(120,0,120);
        arrColors[27] = QColor(120,0,0);
        arrColors[28] = QColor(30,30,30);
        arrColors[29] = QColor(0,70,70);
        arrColors[30] = QColor(70,70,0);
        arrColors[31] = QColor(120,120,120);

}

void MainWindow::on_checkBoxColorReduction_stateChanged(int checkState)
{
    if (checkState == Qt::Unchecked) isColorReductionChecked = false;
        else
            isColorReductionChecked = true;
}

void MainWindow::on_checkBoxPixelize_stateChanged(int checkState)
{
    if (checkState == Qt::Unchecked) isPixelizeChecked = false;
        else
            isPixelizeChecked = true;
}

void MainWindow::on_checkBoxEdges_stateChanged(int checkState)
{
    if (checkState == Qt::Unchecked) isEdgesChecked = false;
        else
            isEdgesChecked = true;
}

void MainWindow::on_btnSave_clicked()
{
    if (modifiedImage != nullptr ) {
    QString imagePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("JPEG (*.jpg *.jpeg);;PNG (*.png)" )  );
    modifiedImage->save(imagePath);
}
}

void MainWindow::on_btnHelp_clicked()
{
    QMessageBox::information(this,tr("HELP"),tr("Tutaj bedzie opis apki ^^"));
}
