#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <QTimer>
#include <QProcess>

QT_BEGIN_NAMESPACE

using namespace cv;
using namespace std;

namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    //Mat转QLabel
    QImage MatImageToQt(const Mat &src);
    ~MainWindow();

private slots:
    void readFrame();
    void on_Open_triggered();
    void on_Stop_triggered();
    void on_About_triggered();
    void on_Exit_triggered();
    void on_modeswitch_clicked();
    void readoutput();
    void readerror();
    void readport();
    void readmode();
    void on_Photograph_clicked();
    void on_record_clicked();
    void on_pause_clicked();

private:
    Ui::MainWindow *ui;
    VideoCapture cap;
    Mat src_image;
    QTimer *timer;
    QImage *image;
    QProcess *process;
    QString device; //相机索引
    QString mode; //模式代号
    VideoWriter writer;
};

#endif // MAINWINDOW_H
