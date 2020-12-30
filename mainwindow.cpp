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

    cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim2/1.png").toStdString());
    cv::Mat ret;
    centerCalibration(mat, ret, centerXDis, centerYDis);

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
//        cv::Mat mat = cv::imread("C:/Users/60455/Desktop/25fr/0.bmp", cv::IMREAD_COLOR);
//        QImage image = GlobalFun::convertMatToQImage(mat);
//        drawMask(image);

        cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim2/%1.png").arg(count).toStdString());
        qreal xDis = 0;
        qreal yDis = 0;
        qreal zDis = 0;
        autoAim(mat, centerXDis, centerYDis, xDis, yDis, zDis);

        QString str = QString("%1 - ").arg(count);
        if ( xDis > 0 ) {
            str += QStringLiteral("向右 ") + QString::number(xDis) + " ";
        } else if ( xDis < 0 ) {
            str += QStringLiteral("向左 ") + QString::number(-xDis) + " ";
        } else {
            // pass
        }

        if ( yDis > 0 ) {
            str += QStringLiteral("向下 ") + QString::number(yDis) + " ";
        } else if ( yDis < 0 ) {
            str += QStringLiteral("向上 ") + QString::number(-yDis) + " ";
        } else {
            // pass
        }

        if ( zDis != 0 ) {
            str += QStringLiteral("Z向移动 ") + QString::number(zDis);
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
    });
    test_timer->start(2000);
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
        cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim2/%1.png").arg(count).toStdString());
        qreal xDis = 0;
        qreal yDis = 0;
        qreal zDis = 0;
        autoAimTest(mat, centerXDis, centerYDis, xDis, yDis, zDis);

        QString str = QString("%1 - ").arg(count);
        if ( xDis > 0 ) {
            str += QStringLiteral("向右 ") + QString::number(xDis) + " ";
        } else if ( xDis < 0 ) {
            str += QStringLiteral("向左 ") + QString::number(-xDis) + " ";
        } else {
            // pass
        }

        if ( yDis > 0 ) {
            str += QStringLiteral("向下 ") + QString::number(yDis) + " ";
        } else if ( yDis < 0 ) {
            str += QStringLiteral("向上 ") + QString::number(-yDis) + " ";
        } else {
            // pass
        }

        if ( zDis != 0 ) {
            str += QStringLiteral("Z向移动 ") + QString::number(zDis);
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
}

//******************************************************************************************************************

void MainWindow::heightCalibration(cv::Mat origin, cv::Mat deformation, int distance, qreal &radius, qreal &scale_mm)
{
    // 转化成灰度图像并进行平滑处理
    cv::Mat gray1;
    cvtColor(origin, gray1, cv::COLOR_BGR2GRAY);
    blur(gray1, gray1, cv::Size(3,3));

    cv::Mat gray2;
    cvtColor(deformation, gray2, cv::COLOR_BGR2GRAY);
    blur(gray2, gray2, cv::Size(3,3));

    // 阈值化
    cv::Mat threshold1, threshold2;
    threshold(gray1, threshold1, 240, 255, cv::THRESH_BINARY);
    threshold(gray2, threshold2, 240, 255, cv::THRESH_BINARY);

    // 找到所有轮廓
    vector<vector<cv::Point> > contours1;
    vector<vector<cv::Point> > contours2;
    vector<cv::Vec4i> hierarchy1;
    vector<cv::Vec4i> hierarchy2;
    findContours(threshold1, contours1, hierarchy1, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    findContours(threshold2, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    if ( contours1.size() > 2 ) {
        // 三角阈值法
        threshold(gray1, threshold1, 0, 255, cv::THRESH_TRIANGLE);

        // 删除二值图像中面积小于设置像素值的对象
        bwareaopen(threshold1, threshold1, 100);

        // 重新寻找轮廓
        contours1.clear();
        hierarchy1.clear();
        findContours(threshold1, contours1, hierarchy1, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }

    if ( contours2.size() > 2 ) {
        // 三角阈值法
        threshold(gray2, threshold2, 0, 255, cv::THRESH_TRIANGLE);

        // 删除二值图像中面积小于设置像素值的对象
        bwareaopen(threshold2, threshold2, 100);

        // 重新寻找轮廓
        contours2.clear();
        hierarchy2.clear();
        findContours(threshold2, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }

    // 多边形逼近轮廓 + 获取矩形和圆形边界框
    vector<vector<cv::Point>> contours_poly1( contours1.size() );
    vector<vector<cv::Point>> contours_poly2( contours2.size() );
    vector<cv::Point2f> center1( contours1.size() );
    vector<cv::Point2f> center2( contours2.size() );
    vector<float> radius1( contours1.size() );
    vector<float> radius2( contours2.size() );

    // 遍历每一个轮廓
    for ( size_t i = 0; i < contours1.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours1[i]), contours_poly1[i], 3, true );  // 多边拟合
        minEnclosingCircle( contours_poly1[i], center1[i], radius1[i] );    // 得到包含二维点集的最小圆
    }
    for ( size_t i = 0; i < contours2.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours2[i]), contours_poly2[i], 3, true );  // 多边拟合
        minEnclosingCircle( contours_poly2[i], center2[i], radius2[i] );    // 得到包含二维点集的最小圆
    }

    // 确定中心点
    cv::Mat ret;
    qreal dx, dy;
    centerCalibration(origin, ret, dx, dy);

    qreal centerX = origin.cols/2 + dx;
    qreal centerY = origin.rows/2 + dy;
    int boundary = 5;

    // 去除中心点
    vector<cv::Point2f> center3;
    vector<float> radius3;

    cv::RNG rng(12345);
    for ( int i = 0; i < (int)center1.size(); ++i )
    {
        if ( abs(center1[i].x - centerX) > boundary || abs(center1[i].y - centerY) > boundary ) {
            cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
            circle( origin, center1[i], (int)radius1[i], color, 2, 8, 0 );

            center3.push_back(center1[i]);
            radius3.push_back(radius1[i]);
            string str = QString::number(radius1[i]).toStdString();
            cv::Point2f org;
            org.x = center1[i].x - radius1[i] - 5;
            org.y = center1[i].y - radius1[i] - 20;
            putText( origin, str, org, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
        }
    }
    for ( int i = 0; i < (int)center2.size(); ++i )
    {
        if ( abs(center2[i].x - centerX) > boundary || abs(center2[i].y - centerY) > boundary ) {
            cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
            circle( deformation, center2[i], (int)radius2[i], color, 2, 8, 0 );

            center3.push_back(center2[i]);
            radius3.push_back(radius2[i]);
            string str = QString::number(radius2[i]).toStdString();
            cv::Point2f org;
            org.x = center2[i].x - radius2[i] - 5;
            org.y = center2[i].y - radius2[i] - 20;
            putText( deformation, str, org, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
        }
    }

    // 计算标定值
    if ( radius3.size() == 2 ) {
        qreal r1 = radius3[0];
        qreal r2 = radius3[1];
        qreal scale = 0;

        if ( r1 < r2 ) {
            radius = r1;
            scale = r2 / r1;
        } else {
            radius = r2;
            scale = r1 / r2;
        }

        scale_mm = distance / scale;
    } else {
        radius = -1;
        scale_mm = -1;
    }
}

void MainWindow::planeCalibration(cv::Mat origin, cv::Mat deformation, int distance, qreal &pix_mm)
{
    // 转化成灰度图像并进行平滑处理
    cv::Mat gray1;
    cvtColor(origin, gray1, cv::COLOR_BGR2GRAY);
    blur(gray1, gray1, cv::Size(3,3));

    cv::Mat gray2;
    cvtColor(deformation, gray2, cv::COLOR_BGR2GRAY);
    blur(gray2, gray2, cv::Size(3,3));

    // 阈值化
    cv::Mat threshold1, threshold2;
    threshold(gray1, threshold1, 240, 255, cv::THRESH_BINARY);
    threshold(gray2, threshold2, 240, 255, cv::THRESH_BINARY);

    // 找到所有轮廓
    vector<vector<cv::Point> > contours1;
    vector<vector<cv::Point> > contours2;
    vector<cv::Vec4i> hierarchy1;
    vector<cv::Vec4i> hierarchy2;
    findContours(threshold1, contours1, hierarchy1, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    findContours(threshold2, contours2, hierarchy2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // 多边形逼近轮廓 + 获取矩形和圆形边界框
    vector<vector<cv::Point>> contours_poly1( contours1.size() );
    vector<vector<cv::Point>> contours_poly2( contours2.size() );
    vector<cv::Point2f> center1( contours1.size() );
    vector<cv::Point2f> center2( contours2.size() );
    vector<float> radius1( contours1.size() );
    vector<float> radius2( contours2.size() );

    // 遍历每一个轮廓
    for ( size_t i = 0; i < contours1.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours1[i]), contours_poly1[i], 3, true );  // 多边拟合
        minEnclosingCircle( contours_poly1[i], center1[i], radius1[i] );    // 得到包含二维点集的最小圆
    }
    for ( size_t i = 0; i < contours2.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours2[i]), contours_poly2[i], 3, true );  // 多边拟合
        minEnclosingCircle( contours_poly2[i], center2[i], radius2[i] );    // 得到包含二维点集的最小圆
    }

    // 确定中心点
    cv::Mat ret;
    qreal dx, dy;
    centerCalibration(origin, ret, dx, dy);

    qreal centerX = origin.cols/2 + dx;
    qreal centerY = origin.rows/2 + dy;
    int boundary = 5;

    // 去除中心点
    vector<cv::Point2f> center3;
    vector<float> radius3;

    for ( size_t i = 0; i < center1.size(); ++i )
    {
        if ( abs(center1[i].x - centerX) > boundary || abs(center1[i].y - centerY) > boundary ) {
            center3.push_back(center1[i]);
            radius3.push_back(radius1[i]);
        }
    }
    for ( size_t i = 0; i < center2.size(); ++i )
    {
        if ( abs(center2[i].x - centerX) > boundary || abs(center2[i].y - centerY) > boundary ) {
            center3.push_back(center2[i]);
            radius3.push_back(radius2[i]);
        }
    }

    cv::RNG rng(12345);
    for ( int i = 0; i < (int)center3.size(); ++i )
    {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
        circle( deformation, center3[i], (int)radius3[i], color, 2, 8, 0 );                                   // 绘制外界圆
    }

    // 计算标定值
    if ( center3.size() == 2 ) {
        dx = abs(center3[1].x - center3[0].x);
        dy = abs(center3[1].y - center3[0].y);
        qreal pix = dx < dy ? dy : dx;
        pix_mm = distance / pix*1.0f;

        cv::Point2f org;
        if ( pix == dy ) {
            org.x = center3[0].x + 20;
            org.y = (center3[0].y + center3[1].y) / 2;
        } else {
            org.x = center3[0].x < center3[1].x ? center3[0].x : center3[1].x;
            org.y = center3[0].y - 20;
        }

        string str = QString::number(pix).toStdString();

        putText( deformation, str, org, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
        line( deformation, center3[0], center3[1], cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    } else {
        pix_mm = -1;
    }
}

void MainWindow::centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy)
{
    // 取图像中心 100x100
    cv::Mat dst = input(cv::Range(input.rows/2 - 50, input.rows/2 + 50), cv::Range(input.cols/2 - 50, input.cols/2 + 50));

    // 转化成灰度图像并进行平滑处理
    cv::Mat src_gray;
    cvtColor(dst, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // 阈值化
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 100, 255, cv::THRESH_BINARY_INV); // 像素值小于100变成0，大于100变成255

    // 边缘检测
    cv::Mat edge;
    Canny(threshold_output, edge, 50, 200, 3);

    // 霍夫线变换
    vector<cv::Vec2f> lines;
    HoughLines(edge, lines, 1, CV_PI/180, 20, 0, 0);

    vector<cv::Vec4i> landscape_lines;
    vector<cv::Vec4i> portrait_lines;

    // 判断是横线还是竖线，并画出所有线条
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 100*(-b));
        pt1.y = cvRound(y0 + 100*(a));
        pt2.x = cvRound(x0 - 100*(-b));
        pt2.y = cvRound(y0 - 100*(a));

        if ( abs(pt2.x - pt1.x) < 10 ) {
            portrait_lines.push_back(cv::Vec4i(pt1.x, pt1.y, pt2.x, pt2.y));
        }
        if ( abs(pt2.y - pt1.y) < 10 ) {
            landscape_lines.push_back(cv::Vec4i(pt1.x, pt1.y, pt2.x, pt2.y));
        }
    }

    // 计算交点
    vector<cv::Point2f> vec;
    for ( size_t i = 0; i < landscape_lines.size() && i < 2; ++i )
    {
        for ( size_t j = 0; j < portrait_lines.size() && j < 2; ++j )
        {
            vec.push_back(getCrossPoint(landscape_lines.at(i), portrait_lines.at(j)));
        }
    }

    // 计算中心点
    cv::Point2f ret(0, 0);
    for ( auto temp : vec )
    {
        ret.x += temp.x;
        ret.y += temp.y;
    }

    // 计算像素差
    dx = ret.x / vec.size() - dst.cols/2;
    dy = ret.y / vec.size() - dst.rows/2;

    output = dst.clone();
    line(output, cv::Point(0, 0), cv::Point(output.cols, output.rows), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    line(output, cv::Point(output.cols, 0), cv::Point(0, output.rows), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    line(output, cv::Point(output.cols/2+dx, output.rows/2+dy),
         cv::Point(output.cols/2, output.rows/2), cv::Scalar(255, 0, 0), 1, cv::LINE_AA);
}

cv::Point2f MainWindow::getCrossPoint(cv::Vec4i lineA, cv::Vec4i lineB)
{
    cv::Point2f pt;
    double X1 = lineA[2] - lineA[0];
    double Y1 = lineA[3] - lineA[1];

    double X2 = lineB[2] - lineB[0];
    double Y2 = lineB[3] - lineB[1];

    double X21 = lineB[0] - lineA[0];
    double Y21 = lineB[1] - lineA[1];

    double D = Y1*X2 - Y2*X1;

    if ( D == 0 ) return cv::Point2f();

    pt.x = (X1*X2*Y21 + Y1*X2*lineA[0] - Y2*X1*lineB[0]) / D;
    pt.y = -(Y1*Y2*X21 + X1*Y2*lineA[1] - X2*Y1*lineB[1]) / D;

    if ((abs(pt.x - lineA[0] - X1 / 2) <= abs(X1 / 2)) &&
            (abs(pt.y - lineA[1] - Y1 / 2) <= abs(Y1 / 2)) &&
            (abs(pt.x - lineB[0] - X2 / 2) <= abs(X2 / 2)) &&
            (abs(pt.y - lineB[1] - Y2 / 2) <= abs(Y2 / 2)))
    {
        return pt;
    }

    return cv::Point2f();
}

void MainWindow::bwareaopen(cv::Mat src, cv::Mat &dst, double min_area)
{
    dst = src.clone();
    std::vector<std::vector<cv::Point> >  contours;
    std::vector<cv::Vec4i>    hierarchy;

    cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point());
    if ( !contours.empty() && !hierarchy.empty() )
    {
        std::vector< std::vector<cv::Point> >::const_iterator itc = contours.begin();

        while ( itc != contours.end() )
        {
            cv::Rect rect = cv::boundingRect( cv::Mat(*itc) );
            double area = contourArea(*itc);

            if ( area < min_area ) {
                for ( int i = rect.y; i < rect.y + rect.height; ++i )
                {
                    uchar *output_data = dst.ptr<uchar>(i);
                    for ( int j = rect.x; j < rect.x + rect.width; ++j )
                    {
                        if ( output_data[j] == 255 ) {
                            output_data[j] = 0;
                        }
                    }
                }
            }
            ++itc;
        }
    }
}

void MainWindow::autoAim(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal &xDis, qreal &yDis, qreal &zMult)
{
    // 转化成灰度图像并进行平滑处理
    cv::Mat src_gray;
    cvtColor(mat, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // 阈值化
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 240, 255, cv::THRESH_BINARY);

    // 找到所有轮廓
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    if ( contours.size() > 2 ) {
        // 三角阈值法
        threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

        // 删除二值图像中面积小于设置像素值的对象
        bwareaopen(threshold_output, threshold_output, 100);

        // 开运算，先腐蚀再膨胀，能够排除小团块物体
//        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11), cv::Point(-1, -1));
//        cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel);

        // 重新寻找轮廓
        contours.clear();
        hierarchy.clear();
        findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }

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

    // 确定中心点
    int centerX = mat.cols/2 + centerXDis;
    int centerY = mat.rows/2 + centerYDis;
    int boundary = 5;
    cv::Point centerPoint;
    qreal x = 0;
    qreal y = 0;

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
        x = centerX - center[0].x;
        y = centerY - center[0].y;

        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
    }
    else
    {
        QVector<cv::Point2f> vec;

        for ( auto &temp : center )
        {
            if ( abs(centerX - temp.x) > boundary*2 || abs(centerY - temp.y) > boundary*2 ) {
               vec.push_back(temp);
            }
        }

        cv::Point2f point(0, 0);
        for ( auto &temp: vec )
        {
            point.x += temp.x;
            point.y += temp.y;
        }
        point.x = point.x / vec.size();
        point.y = point.y / vec.size();
        centerPoint.x = point.x;
        centerPoint.y = point.y;

        x = centerX - point.x;    // 横坐标像素差
        y = centerY - point.y;    // 纵坐标像素差

        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
    }

    //---------------------------------

    // 画多边形轮廓 + 圆形框
    cv::RNG rng(12345);

    for ( int i = 0; i < (int)contours_poly.size(); ++i )
    {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
//        drawContours( mat, contours_poly, i, color, 1, 8, vector<cv::Vec4i>(), 0, cv::Point() );    // 绘制轮廓
        circle( mat, center[i], (int)radius[i], color, 2, 8, 0 );                                   // 绘制外界圆

//        cv::Point org;
//        org.x = center[i].x - 2 * radius[i];
//        org.y = center[i].y;
//        std::string num = QString("%1").arg(i+1).toStdString();
//        putText( mat, num, org, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    }

    if ( xDis != 0 || yDis != 0 )
    {
        line( mat, cv::Point(centerX, centerY), centerPoint, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    }
}

void MainWindow::autoAimTest(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal &xDis, qreal &yDis, qreal &zMult)
{
    cv::destroyAllWindows();

    // 转化成灰度图像并进行平滑处理
    cv::Mat src_gray;
    cvtColor(mat, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // 阈值化
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 240, 255, cv::THRESH_BINARY | cv::THRESH_TRIANGLE);
    cv::imshow("threshold_output1", threshold_output);

    cv::adaptiveThreshold(src_gray, threshold_output, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 21, 20);
    cv::imshow("threshold_output2", threshold_output);

    threshold(threshold_output, threshold_output, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    morphologyEx(threshold_output, threshold_output, cv::MORPH_CLOSE, kernel, cv::Point(-1,-1), 1);
    cv::imshow("threshold_output3", threshold_output);

//    // 找到所有轮廓
//    vector<vector<cv::Point> > contours;
//    vector<cv::Vec4i> hierarchy;
//    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

//    if ( contours.size() > 2 ) {
//        // 三角阈值法
//        threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);
////        cv::imshow("threshold_output2", threshold_output);

//        // 删除二值图像中面积小于设置像素值的对象
//        bwareaopen(threshold_output, threshold_output, 100);
////        cv::imshow("threshold_output3", threshold_output);

//        // 开运算，先腐蚀再膨胀，能够排除小团块物体
////        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
////        cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel);
////        cv::imshow("threshold_output4", threshold_output);

//        // 重新寻找轮廓
//        contours.clear();
//        hierarchy.clear();
//        findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
//    }

//    // 多边形逼近轮廓 + 获取矩形和圆形边界框
//    vector<vector<cv::Point>> contours_poly( contours.size() );
//    vector<cv::Point2f> center( contours.size() );
//    vector<float> radius( contours.size() );

//    // 遍历每一个轮廓
//    for ( size_t i = 0; i < contours.size(); ++i )
//    {
//        approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );    // 多边拟合
//        minEnclosingCircle( contours_poly[i], center[i], radius[i] );       // 得到包含二维点集的最小圆
//    }

//    // 确定中心点
//    int centerX = mat.cols/2 + centerXDis;
//    int centerY = mat.rows/2 + centerYDis;
//    int boundary = 5;
//    cv::Point centerPoint;
//    qreal x = 0;
//    qreal y = 0;

//    // 计算像素差
//    if ( contours_poly.size() == 0 )
//    {
//        xDis = 0;
//        yDis = 0;
//    }
//    else if ( contours_poly.size() == 1 )
//    {
//        centerPoint.x = center[0].x;
//        centerPoint.y = center[0].y;
//        x = centerX - center[0].x;
//        y = centerY - center[0].y;

//        xDis = abs(x) >= boundary ? x : 0;
//        yDis = abs(y) >= boundary ? y : 0;
//    }
//    else
//    {
//        QVector<cv::Point2f> vec;

//        for ( auto &temp : center )
//        {
//            if ( abs(centerX - temp.x) > boundary*2 || abs(centerY - temp.y) > boundary*2 ) {
//               vec.push_back(temp);
//            }
//        }

//        cv::Point2f point(0, 0);
//        for ( auto &temp: vec )
//        {
//            point.x += temp.x;
//            point.y += temp.y;
//        }
//        point.x = point.x / vec.size();
//        point.y = point.y / vec.size();
//        centerPoint.x = point.x;
//        centerPoint.y = point.y;

//        x = centerX - point.x;    // 横坐标像素差
//        y = centerY - point.y;    // 纵坐标像素差

//        xDis = abs(x) >= boundary ? x : 0;
//        yDis = abs(y) >= boundary ? y : 0;
//    }

//    //---------------------------------

//    // 画多边形轮廓 + 圆形框
//    cv::RNG rng(12345);

//    for ( int i = 0; i < (int)contours_poly.size(); ++i )
//    {
//        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
//        circle( mat, center[i], (int)radius[i], color, 2, 8, 0 );                                   // 绘制外界圆
//    }

//    if ( xDis != 0 || yDis != 0 )
//    {
//        line( mat, cv::Point(centerX, centerY), centerPoint, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
//    }
}

void MainWindow::test()
{
//    cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim2/1.png").toStdString());

//    cv::Mat image1 = cv::imread("5.png");
//    cv::Mat image2 = cv::imread("6.png");
//    qreal ret1 = 0;
//    planeCalibration(image1, image2, 5, ret1);
//    qDebug() << "ret1: " << ret1;
//    cv::imshow("image1", image1);
//    cv::imshow("image2", image2);

    cv::Mat image3 = cv::imread("7.png");
    cv::Mat image4 = cv::imread("9.png");
    qreal radius = 0;
    qreal ret2 = 0;
    heightCalibration(image3, image4, 5, radius, ret2);
    qDebug() << "radius: " << radius;
    qDebug() << "ret2: " << ret2;
    cv::imshow("image3", image3);
    cv::imshow("image4", image4);
}
