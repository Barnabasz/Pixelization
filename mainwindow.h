#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    //Prepare image
    void on_btnAddImage_clicked();

    //Apply chosen functions to image
    void on_btnConvert_clicked();

    //Save image
    void on_btnSave_clicked();

    //Display help dialog
    void on_btnHelp_clicked();

    //Check for functions to apply
    void on_checkBoxColorReduction_stateChanged(int arg1);
    void on_checkBoxPixelize_stateChanged(int arg1);
    void on_checkBoxEdges_stateChanged(int arg1);

    //Filling pallete of retro colors
    void setColors();

    //Repaint image in retro colors (QColor arrColors[32])
    void paintRetro(QImage&);

    //Pixelize image
    void paintPixelization(QImage&);

    //Pixelize single step
    void paintPixelizationStep(QImage&);
    //Find color of new pixel
    QColor pixelizationColor(QImage&, int x, int y);
    void toSize(QImage&, int it);
    //Detect edges and create mask of edges (char ** contourMask)
    void detectEdges(QImage&);

    //Paint edges on image
    void paintEdges(QImage&);



    void taskPixelization(QImage&, int k);
    void taskDEdges(QImage&, int k);
    void taskPEdges(QImage&, int k);
    void taskRetro(QImage& img, int k );



private:
    Ui::MainWindow *ui;
    QImage loadedImage;
    QImage * modifiedImage = nullptr;

    int SrcWidth, SrcHeight, TgtWidth, TgtHeight;
    int numberOfThreads;
    //Check which functions to apply
    bool isImageSelected = true;
    bool isColorReductionChecked;
    bool isPixelizeChecked;
    bool isEdgesChecked;

    QImage Resoult;

    //Array for 16 basic colors.
    QColor arrColors[32];

    //Mask of edges
    char** contourMask;
};

#endif // MAINWINDOW_H
