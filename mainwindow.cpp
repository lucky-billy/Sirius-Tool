#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globalfun.h"
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    is_in_test = true;

    if ( !is_in_test ) {
        initData();
        initView();
    } else {
        count = 1;
        test();
    }
}

MainWindow::~MainWindow()
{
    if ( !is_in_test ) {
        m_camera->stop();
        m_camera->destroyCamera();
        if ( test_timer->isActive() ) { test_timer->stop(); }
    }

    delete ui;
}

void MainWindow::initData()
{
    // init
    mask = false;

    // config
    QString config = "config.ini";
    QFileInfo fileInfo(config);
    int camera_type = 0;

    if ( !fileInfo.isFile() ) {
        QSettings settings(config, QSettings::IniFormat);
        settings.setValue("camera/type", 1);
        settings.setValue("camera/interval", 100);
        settings.setValue("common/width", 300);
        camera_type = 1;
        width = 300;
    } else {
        QSettings settings(config, QSettings::IniFormat);
        camera_type = settings.value("camera/type").toInt();
        width = settings.value("common/width").toInt();
    }

    // camera
    switch ( camera_type )
    {
    case 1: m_camera = new BBaslerCamerControl(Camera::CameraType::Basler); break;
    case 2: m_camera = new BICCameraControl(Camera::CameraType::IC_Imaging); break;
    case 3: m_camera = new BMVCameraControl(Camera::CameraType::MV); break;
    case 4: m_camera = new BVRCameraControl(Camera::CameraType::Virtual); break;
    default: m_camera = new BBaslerCamerControl(Camera::CameraType::Basler); break;
    }

    connect(m_camera, &Camera::updateImage, this, [&](QImage image){ drawMask(image); });
    connect(m_camera, &Camera::error, this, [&](QString message){ GlobalFun::showMessageBox(4, message); }, Qt::QueuedConnection);

    m_camera->initCamera();
    m_camera->openCamera();
//    m_camera->startWork();

    //---------------------------------

    // test
    test_timer = new QTimer(this);
    count = 1;

    connect(test_timer, &QTimer::timeout, this, [&](){
        int module = 2;

        if ( module == 1 ) {
            // MTF
            cv::Mat mat = cv::imread("C:/Users/60455/Desktop/25fr/0.bmp", cv::IMREAD_COLOR);
            QImage image = GlobalFun::convertMatToQImage(mat);
            drawMask(image);
        }

        if ( module == 2 ) {
            // 自动对焦
            cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim2/%1.png").arg(count).toStdString());
            qreal xDis = 0;
            qreal yDis = 0;
            qreal zDis = 0;
            qreal min_radius = 10;

            cv::Mat ret;
            qreal centerXDis = 0;
            qreal centerYDis = 0;
            GlobalFun::centerCalibration(mat, ret, centerXDis, centerYDis);
            GlobalFun::autoAim(mat, centerXDis, centerYDis, min_radius, xDis, yDis, zDis);

            QString str = QString("%1 - ").arg(count);
            if ( xDis > 0 ) {
                str += QStringLiteral("需向右 ") + QString::number(xDis);
            } else if ( xDis < 0 ) {
                str += QStringLiteral("需向左 ") + QString::number(-xDis);
            } else {
                // pass
            }

            if ( yDis > 0 ) {
                str += QStringLiteral(" 需向下 ") + QString::number(yDis);
            } else if ( yDis < 0 ) {
                str += QStringLiteral(" 需向上 ") + QString::number(-yDis);
            } else {
                // pass
            }

            if ( zDis != 0 ) {
                str += QStringLiteral(" 需缩小 ") + QString::number(zDis) + QStringLiteral(" 倍");
            } else {
                // pass
            }

            if ( xDis == 0 && yDis == 0 && zDis == 0 ) {
                str += QStringLiteral("对准完成");
            }

            QImage image = GlobalFun::convertMatToQImage(mat);
            QImage pic = image.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio);
            QPainter painter(&pic);
            QFont font;
            font.setBold(true);
            font.setPixelSize(32);
            painter.setFont(font);
            QPen pen(Qt::red);
            painter.setPen(pen);
            painter.drawText(10, 40, str);

            int wl = this->geometry().width();
            int hl = this->geometry().height() - 40;
            int wp = pic.width();
            int hp = pic.height();

            ui->label->setGeometry( (wl - wp) / 2.0, (hl - hp) / 2.0, pic.width(), pic.height() );
            ui->label->setPixmap(QPixmap::fromImage(pic));

            count++;
            if ( count == 71 ) { count = 1; }
        }

        if ( module == 3 ) {
            // 光暗自动调节
            cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/overExposure/%1.bmp").arg(count).toStdString());

            cv::Mat gray;
            cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
            blur(gray, gray, cv::Size(3,3));

            cv::Mat mean, std;
            meanStdDev(gray, mean, std);

            double m, s;
            m = mean.at<double>(0,0);
            s = std.at<double>(0,0);

            QString str = QString("%1 - ").arg(count);
            str += "mean: " + QString::number(m) + ", std: " + QString::number(s);

            double minVal = 0;
            double maxVal = 0;
            cv::minMaxIdx(gray, &minVal, &maxVal, nullptr, nullptr);
            str += ", maxVal: " + QString::number(maxVal);

            if ( maxVal < 180 ) {
                if ( maxVal > 140 ) {
                    str += QStringLiteral(" - 偏暗");
                } else if ( maxVal > 100 ) {
                    str += QStringLiteral(" - 过暗");
                } else {
                    str += QStringLiteral(" - 非常暗");
                }
            } else if ( maxVal > 254 ) {
                if ( s < 80 ) {
                    str += QStringLiteral(" - 偏亮");
                } else if ( s < 100 ) {
                    str += QStringLiteral(" - 过亮");
                } else {
                    str += QStringLiteral(" - 非常亮");
                }
            } else {
                str += QStringLiteral(" - 正合适");
            }
            qDebug() << str;

            QImage image = GlobalFun::convertMatToQImage(mat);
            QImage pic = image.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio);
            QPainter painter(&pic);
            QFont font;
            font.setBold(true);
            font.setPixelSize(24);
            painter.setFont(font);
            QPen pen(Qt::red);
            painter.setPen(pen);
            painter.drawText(10, 40, str);

            int wl = this->geometry().width();
            int hl = this->geometry().height() - 40;
            int wp = pic.width();
            int hp = pic.height();

            ui->label->setGeometry( (wl - wp) / 2.0, (hl - hp) / 2.0, pic.width(), pic.height() );
            ui->label->setPixmap(QPixmap::fromImage(pic));

            count++;
            if ( count == 18 ) { count = 1; }
        }

    });
    test_timer->start(3000);
}

void MainWindow::initView()
{
    setWindowTitle("Sirius-Tool");
    setMaximumSize(1024, 768);
    setMinimumSize(1024, 768);

    ui->action_plus->setEnabled(false);
    ui->action_minus->setEnabled(false);
    ui->action_calculate->setEnabled(false);

    connect(ui->action_draw, &QAction::triggered, this, [&](){ showMask(); });
    connect(ui->action_plus, &QAction::triggered, this, [&](){ maskPlus(); });
    connect(ui->action_minus, &QAction::triggered, this, [&](){ maskMinus(); });
}

void MainWindow::showMask()
{
    ui->action_plus->setEnabled(!ui->action_plus->isEnabled());
    ui->action_minus->setEnabled(!ui->action_minus->isEnabled());
    ui->action_calculate->setEnabled(!ui->action_calculate->isEnabled());
    mask = !mask;
}

void MainWindow::maskPlus()
{
    int w = ui->label->width();
    int h = ui->label->height();

    if ( width >= w*0.98 || width >= h*0.98 ) {
        GlobalFun::showMessageBox(3, "The mask size has reached the maximum !");
        return;
    }

    width += 20;
}

void MainWindow::maskMinus()
{
    if ( width <= 100 ) {
        GlobalFun::showMessageBox(3, "The mask size has reached the minimum !");
        return;
    }

    width -= 20;
}

qreal MainWindow::processMTF(QImage &image, QRect &rect)
{
    // 光学系统性能的衡量方法有很多，常见的有点扩散函数法、瑞利判断法、点列图法、光学传递函数(MTF)法等。
    // 其中 MTF 法在光学系统和镜头加工制造中使用最为广泛。MTF 曲线真实的反映了成像系统将物方信息传递到像方的能力。

    // MTF的计算方法有很多，比如使用条形目标的方法、随机目标方法、点扩散函数法、带宽受限激光散斑法、倾斜边缘法(Slanted Edge Method)等，
    // 其中点扩散函数法和倾斜边缘法是工业界普遍使用的方法。

    // 倾斜边缘法流程: 首先获取倾斜边缘的边缘扩散函数(ESF)，然后求导得到对应的线扩散函数(LSF)，最后傅里叶变换得到MTF。

    cv::Mat mat = GlobalFun::convertQImageToMat(image);
    cv::Mat gray;
    if ( mat.type() == CV_8UC3 ) {
        cv::cvtColor(mat, gray, cv::COLOR_RGB2GRAY);
    } else if ( mat.type() == CV_8UC4 ) {
        cv::cvtColor(mat, gray, cv::COLOR_RGBA2GRAY);
    }

    cv::Mat ret = gray(cv::Rect(rect.x(), rect.y(), rect.width(), rect.height()));
    cv::Mat ySlice = ret.col(ret.cols / 2).clone();

    qreal max = 0;
    qreal min = 255;
    uchar *data = ySlice.data;
    int count = 1;
    vector<qreal> vec;

    for ( int i = 0; i < ySlice.rows; ++i )
    {
        if ( *data > max ) { max = *data; }
        if ( *data < min ) { min = *data; }
        data++;
        count++;

        if ( count == 100 || i == ySlice.rows - 1 ) {
            qreal mtf = (max - min) / 255.0f;
            if ( mtf != 0 ) { vec.push_back(mtf); }

            max = 0;
            min = 255;
            count = 1;
        }
    }

    qreal sum = 0;
    for ( auto &temp : vec )
    {
        sum += temp;
    }

    qreal average_mtf = sum / vec.size();
    return average_mtf;
}

void MainWindow::drawMask(QImage &image)
{
    QImage pic = image.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio);

    if ( mask ) {
        // draw mask
        QPainter painter(&pic);
        QPen pen(Qt::blue);
        pen.setWidth(3);
        painter.setPen(pen);

        int x = ( pic.width() - width )/2;
        int y = ( pic.height() - width )/2;
        painter.drawRect(x, y, width, width);

        // cal mtf
        QRect rect(x, y, width, width);
        qreal mtf = processMTF(pic, rect);

        // draw text
        QFont font;
        font.setBold(true);
        font.setPixelSize(32);
        painter.setFont(font);
        pen.setColor(Qt::red);
        painter.setPen(pen);

        painter.drawText(10, 40, "MTF: " + QString::number(mtf));
    }

    int wl = this->geometry().width();
    int hl = this->geometry().height() - 40;
    int wp = pic.width();
    int hp = pic.height();

    ui->label->setGeometry( (wl - wp) / 2.0, (hl - hp) / 2.0, pic.width(), pic.height() );
    ui->label->setPixmap(QPixmap::fromImage(pic));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ( event->key() == Qt::Key_Enter ) {
        cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim7/%1.png").arg(count).toStdString());
        qreal xDis = 0;
        qreal yDis = 0;
        qreal zDis = 0;
        qreal min_radius = 10;

        cv::Mat ret;
        qreal centerXDis = 0;
        qreal centerYDis = 0;
        GlobalFun::centerCalibration(mat, ret, centerXDis, centerYDis);
        autoAim(mat, centerXDis, centerYDis, min_radius, xDis, yDis, zDis);

        QString str = QString("%1 - ").arg(count);
        if ( xDis > 0 ) {
            str += QStringLiteral("需向右 ") + QString::number(xDis);
        } else if ( xDis < 0 ) {
            str += QStringLiteral("需向左 ") + QString::number(-xDis);
        } else {
            // pass
        }

        if ( yDis > 0 ) {
            str += QStringLiteral(" 需向下 ") + QString::number(yDis);
        } else if ( yDis < 0 ) {
            str += QStringLiteral(" 需向上 ") + QString::number(-yDis);
        } else {
            // pass
        }

        if ( zDis != 0 ) {
            str += QStringLiteral(" 需缩小 ") + QString::number(zDis) + QStringLiteral(" 倍");
        } else {
            // pass
        }

        if ( xDis == 0 && yDis == 0 && zDis == 0 ) {
            str += QStringLiteral("对准完成");
        }

        QImage image = GlobalFun::convertMatToQImage(mat);
        QImage pic = image.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio);
        QPainter painter(&pic);
        QFont font;
        font.setBold(true);
        font.setPixelSize(32);
        painter.setFont(font);
        QPen pen(Qt::red);
        painter.setPen(pen);
        painter.drawText(10, 40, str);

        int wl = this->geometry().width();
        int hl = this->geometry().height() - 40;
        int wp = pic.width();
        int hp = pic.height();

        ui->label->setGeometry( (wl - wp) / 2.0, (hl - hp) / 2.0, pic.width(), pic.height() );
        ui->label->setPixmap(QPixmap::fromImage(pic));

        count++;
        if ( count == 109 ) { count = 1; }
    }
}

//******************************************************************************************************************

void MainWindow::test()
{
    int module = 2;

    if ( module == 1 ) {
        // 测试平面标定和高度标定
        cv::Mat mat1 = cv::imread(QString("../Sirius-Tool/test/1.png").toStdString());
        cv::Mat mat2 = cv::imread(QString("../Sirius-Tool/test/2.png").toStdString());
        cv::Mat mat3 = cv::imread(QString("../Sirius-Tool/test/3.png").toStdString());
        cv::Mat mat4 = cv::imread(QString("../Sirius-Tool/test/4.png").toStdString());

        qreal pix_mm = 0;
        GlobalFun::planeCalibration(mat1, mat2, 0.1, pix_mm);
        cv::imshow("mat2", mat2);
        qDebug() << "pix_mm: " << pix_mm;

        qreal radius;
        qreal scale_mm;
        GlobalFun::heightCalibration(mat3, mat4, 0.025, radius, scale_mm);
        cv::imshow("mat4", mat4);
        qDebug() << "radius: " << radius;
        qDebug() << "scale_mm: " << scale_mm;
    }
    else if ( module == 2 ) {
        // 透视变换
        cv::Mat src = cv::imread(QString("../Sirius-Tool/test/src.png").toStdString());
        cv::imshow("src", src);
        int width = src.cols;
        int height = src.rows;

        vector<cv::Point2f> src_coners(4);
        src_coners[0] = cv::Point2f(0, 0);
        src_coners[1] = cv::Point2f(100, 0);
        src_coners[2] = cv::Point2f(0, height);
        src_coners[3] = cv::Point2f(100, height);

        vector<cv::Point2f> dst_coners(4);
        dst_coners[0] = cv::Point2f(0, 0);
        dst_coners[1] = cv::Point2f(width, 0);
        dst_coners[2] = cv::Point2f(0, height);
        dst_coners[3] = cv::Point2f(width, height);

        cv::Mat warpMatrix = getPerspectiveTransform(src_coners, dst_coners);
        cv::Mat dst;
        warpPerspective(src, dst, warpMatrix, dst.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT);
        cv::imshow("dst", dst);
    }
}

void MainWindow::autoAim(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal min_radius, qreal &xDis, qreal &yDis, qreal &zMult)
{
    cv::destroyAllWindows();

    // 确定中心点
    int centerX = mat.cols/2 + centerXDis;
    int centerY = mat.rows/2 + centerYDis;
    int boundary = 8;
    cv::Point centerPoint;

    //---------------------------------

    // 转化成灰度图像并进行平滑处理
    cv::Mat src_gray;
    cvtColor(mat, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // 三角阈值法
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

    // 删除二值图像中面积小于设置像素值的对象
    GlobalFun::bwareaopen(threshold_output, threshold_output, 10);

    // 膨胀
    cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
    cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_DILATE, kernel1);

    // 找到所有轮廓
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    //---------------------------------

    // 多边形逼近轮廓 + 获取矩形和圆形边界框
    vector<vector<cv::Point>> contours_poly( contours.size() );
    vector<cv::Point2f> center( contours.size() );
    vector<float> radius( contours.size() );

    // 遍历每一个轮廓
    for ( size_t i = 0; i < contours.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );    // 多边拟合
        minEnclosingCircle( contours_poly[i], center[i], radius[i] );       // 得到包含二维点集的最小圆
    }

    // 计算像素差
    if ( contours_poly.size() == 0 )
    {
        xDis = 0;
        yDis = 0;
        zMult = 0;
    }
    else if ( contours_poly.size() == 1 )
    {
        centerPoint.x = center[0].x;
        centerPoint.y = center[0].y;

        qreal x = centerX - centerPoint.x;
        qreal y = centerY - centerPoint.y;
        qreal z = radius[0] / min_radius;;
        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
        zMult = z > 1.5 ? z : 0;
    }
    else
    {
        qreal my_radius = 0;

        for ( size_t i = 0; i < radius.size(); ++i )
        {
            if ( radius[i] > my_radius && (abs(centerX - center[i].x) >= boundary || abs(centerY - center[i].y) >= boundary) ) {
                centerPoint.x = center[i].x;
                centerPoint.y = center[i].y;
                my_radius = radius[i];
            }
        }

        qreal x = centerX - centerPoint.x;  // 横坐标像素差
        qreal y = centerY - centerPoint.y;  // 纵坐标像素差
        qreal z = my_radius / min_radius;   // 放大比例
        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
        zMult = z > 1.5 ? z : 0;
    }

    //---------------------------------

    // 画多边形轮廓 + 圆形框
    cv::RNG rng(12345);
    qreal my_radius = 0;
    cv::Point my_center;

    for ( int i = 0; i < (int)contours_poly.size(); ++i )
    {
        if ( radius[i] > my_radius && (abs(centerX - center[i].x) >= boundary || abs(centerY - center[i].y) >= boundary) ) {
            my_radius = radius[i];
            my_center = center[i];
        }
    }
    cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
    circle( mat, my_center, (int)my_radius, color, 2, 8, 0 );

    if ( xDis != 0 || yDis != 0 )
    {
        line( mat, cv::Point(centerX, centerY), centerPoint, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    }
}
