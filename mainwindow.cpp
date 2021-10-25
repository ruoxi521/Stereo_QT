#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "mythread.h"
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    image = new QImage();
    process = new QProcess(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(readFrame()));
    connect(ui->Open,SIGNAL(clicked()),this,SLOT(on_Open_triggered()));
    connect(ui->Stop,SIGNAL(clicked()),this,SLOT(on_Stop_triggered()));
    connect(ui->modeswitch,SIGNAL(clicked()),this,SLOT(on_modeswitch_clicked()));
    connect(ui->lineEdit,SIGNAL(textChanged(QString)),this,SLOT(readport()));
    connect(process , &QProcess::readyReadStandardOutput, this , &MainWindow::readoutput);
    connect(process , &QProcess::readyReadStandardError, this , &MainWindow::readerror);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Open_triggered()
{
    //How to convert QString to int?
    cap.open(device.toInt());       //视频捕捉设备 id ---笔记本电脑的用0表示
    timer->start(33);
}

void MainWindow::on_Stop_triggered()
{
    // 停止读取数据。
    timer->stop();
    cap.release();
    ui->cameraView->clear();
}

void MainWindow::on_modeswitch_clicked()
{
    process->start("bash");                      //启动终端(Windows下改为cmd)
    process->waitForStarted();                   //等待启动完成
//    QString homePath = QDir::currentPath();;
//    QFile file(homePath + "/camera_switch.sh");
//    QFile file1(":/new/prefix1/camera_switch.sh\n");
//    qDebug()<<file1 << endl;
//    process->write(file.readAll());    //读文件,默认只识别UTF-8,向终端写入命令，注意尾部的“\n”不可省略
    process->write("/home/sujie/Workspace/QT/webcam_Stereo/camera_switch.sh 2 4\n");    //向终端写入命令，注意尾部的“\n”不可省略
}

void MainWindow::readport()
{
    this->device=ui->lineEdit->text();
    QString str=QString("/home/sujie/Workspace/QT/webcam_Stereo/camera_switch.sh %1\n").arg(device);
    char *n;
    QByteArray m=str.toLatin1();
    n=m.data();
    process->write(n);
}
void MainWindow::readoutput()
{
    ui->textEdit->append(process->readAllStandardOutput().data());   //将输出信息读取到编辑框
}
void MainWindow::readerror()
{
    QMessageBox::information(0, "Error", process->readAllStandardError().data());   //弹出信息框提示错误信息
}

void MainWindow::readFrame()
{
    cap.read(src_image);
    QImage imag = MatImageToQt(src_image);
    ui->cameraView->setScaledContents(true);
    ui->cameraView->setPixmap(QPixmap::fromImage(imag));
}

//Mat转成QImage
QImage MainWindow::MatImageToQt(const Mat &src)
{
    //CV_8UC1 8位无符号的单通道---灰度图片
    if(src.type() == CV_8UC1)
    {
        //使用给定的大小和格式构造图像
        //QImage(int width, int height, Format format)
        QImage qImage(src.cols,src.rows,QImage::Format_Indexed8);
        //扩展颜色表的颜色数目
        qImage.setColorCount(256);

        //在给定的索引设置颜色
        for(int i = 0; i < 256; i ++)
        {
            //得到一个黑白图
            qImage.setColor(i,qRgb(i,i,i));
        }
        //复制输入图像,data数据段的首地址
        uchar *pSrc = src.data;
        //
        for(int row = 0; row < src.rows; row ++)
        {
            //遍历像素指针
            uchar *pDest = qImage.scanLine(row);
            //从源src所指的内存地址的起始位置开始拷贝n个
            //字节到目标dest所指的内存地址的起始位置中
            memcpy(pDest,pSrc,src.cols);
            //图像层像素地址
            pSrc += src.step;
        }
        return qImage;
    }
    //为3通道的彩色图片
    else if(src.type() == CV_8UC3)
    {
        //得到图像的的首地址
        const uchar *pSrc = (const uchar*)src.data;
        //以src构造图片
        QImage qImage(pSrc,src.cols,src.rows,src.step,QImage::Format_RGB888);
        //在不改变实际图像数据的条件下，交换红蓝通道
        return qImage.rgbSwapped();
    }
    //四通道图片，带Alpha通道的RGB彩色图像
    else if(src.type() == CV_8UC4)
    {
        const uchar *pSrc = (const uchar*)src.data;
        QImage qImage(pSrc, src.cols, src.rows, src.step, QImage::Format_ARGB32);
        //返回图像的子区域作为一个新图像
        return qImage.copy();
    }
    else
    {
        return QImage();
    }
}
