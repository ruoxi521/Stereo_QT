#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QDateTime>

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
    connect(ui->Exit,SIGNAL(clicked()),this,SLOT(on_Exit_triggered()));
    connect(ui->modeswitch,SIGNAL(clicked()),this,SLOT(on_modeswitch_clicked()));
    connect(ui->Photograph,SIGNAL(clicked()),this,SLOT(on_Photograph_clicked()));
    connect(ui->lineEdit1,SIGNAL(textChanged(QString)),this,SLOT(readport()));
    connect(ui->lineEdit2,SIGNAL(textChanged(QString)),this,SLOT(readmode()));
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

void MainWindow::on_About_triggered()
{
    QMessageBox::about(this, tr("About Application"),
             tr("The <b>Application</b> example demonstrates how to "
                "write modern GUI applications using Qt, with a menu bar, "
                "toolbars, and a status bar."));
}

void MainWindow::on_Exit_triggered()
{
    exit(0);
}

void MainWindow::on_Photograph_clicked()
{
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-hh hh_mm_ss");
    QString dirName = "Pictures";
    QString homepath = QDir::homePath();
    QImage image = MatImageToQt(src_image);
    if(!QFile::exists(dirName))
    {
       QDir dir;
       if(!dir.mkdir(dirName))
       {
           ui->label_state->setText("创建文件夹 " + dirName + "失败!!!");
//           qDebug() << __FILE__ << __LINE__ << "创建文件夹 " << dirName << "失败!!!";
       }
    }
    QString filePath = QString("%1/%2/%3.png").arg(homepath).arg(dirName).arg(timeStr);
    if(image.save(filePath))
    {
//        qDebug() << __FILE__ << __LINE__ << "保存照片至: " << filePath;
        ui->label_state->setText("保存照片至: " + filePath);
    }else{
        ui->label_state->setText("保存照片失败!!!");
//        qDebug() << __FILE__ << __LINE__ << "保存照片失败!!!";
    }
}

void MainWindow::on_modeswitch_clicked()
{
    process->start("bash");                      //启动终端(Windows下改为cmd)
    process->waitForStarted();                   //等待启动完成
    //    process->write("/home/sujie/Workspace/QT/webcam_Stereo/camera_switch.sh 2 4\n");    //向终端写入命令，注意尾部的“\n”不可省略
    // 切换相机输出模式
    //QString的arg()方法用于填充字符串中的%1,%2...为给定的参数
    QString str1=tr("uvcdynctrl -d /dev/video%1 -S 6:8  '(LE)0x50ff' ").arg(device);
    QString str2=tr("uvcdynctrl -d /dev/video%1 -S 6:15 '(LE)0x00f6' ").arg(device);
    QString str3=tr("uvcdynctrl -d /dev/video%1 -S 6:8  '(LE)0x2500' ").arg(device);
    QString str4=tr("uvcdynctrl -d /dev/video%1 -S 6:8  '(LE)0x5ffe' ").arg(device);
    QString str5=tr("uvcdynctrl -d /dev/video%1 -S 6:15 '(LE)0x0003' ").arg(device);
    QString str6=tr("uvcdynctrl -d /dev/video%1 -S 6:15 '(LE)0x0002' ").arg(device);
    QString str7=tr("uvcdynctrl -d /dev/video%1 -S 6:15 '(LE)0x0012' ").arg(device);
    QString str8=tr("uvcdynctrl -d /dev/video%1 -S 6:15 '(LE)0x0004' ").arg(device);
    QString str9=tr("uvcdynctrl -d /dev/video%1 -S 6:8  '(LE)0x76c3' ").arg(device);
    QString str10=tr("uvcdynctrl -d /dev/video%1 -S 6:10 '(LE)0x0%200'").arg(device).arg(mode);
    QString array[10]={str1,str2,str3,str4,str5,str6,str7,str8,str9,str10};
    char *n;
    for(int i=0;i<10;i++){
        QByteArray m=array[i].toLatin1();
        n=m.data();
        process->write(n);
        process->write("\n");
    }
}

void MainWindow::on_record_clicked()
{
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-hh hh_mm_ss");
    QString dirName = "Videos";
    QString homepath = QDir::homePath();
    double fps = 25.0;  // framerate of the created video stream
    if(!QFile::exists(dirName))
    {
       QDir dir;
       if(!dir.mkdir(dirName))
       {
           ui->label_state->setText("创建文件夹 " + dirName + "失败!!!");
       }
    }
    QString filePath = QString("%1/%2/%3.avi").arg(homepath).arg(dirName).arg(timeStr);
    cap.open(device.toInt());
    writer.open(filePath.toStdString(), VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(640, 480), true);
    // check if we succeeded
    if(!writer.isOpened()) {
            cerr << "Could not open the output video file for write\n";
        }
    while(!src_image.empty())
       {
           // check if we succeeded
           if (!cap.read(src_image)) {
               cerr << "ERROR! blank frame grabbed\n";
               break;
           }
           cap >> src_image;
           //设置保存视频的格式为AVI，编码为MJPG
           writer.write(src_image);
//           namedWindow("VideoPlay", WINDOW_NORMAL);
           imshow("VideoPlay", src_image);
           if (waitKey(5) >= 0)
               break;
           if(!writer.isOpened()) {
               ui->label_state->setText("保存视频至: " + filePath);
               break;
             }
       }
}

void MainWindow::on_pause_clicked()
{
    // 释放资源，清空缓存
//    destroyWindow("VideoPlay");
    destroyAllWindows();
    writer.release();
}

void MainWindow::readport()
{
    this->device=ui->lineEdit1->text();
}

void MainWindow::readmode()
{
    this->mode=ui->lineEdit2->text();
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
