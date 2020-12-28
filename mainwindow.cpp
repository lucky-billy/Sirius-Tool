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
            str += QStringLiteral("���� ") + QString::number(xDis) + " ";
        } else if ( xDis < 0 ) {
            str += QStringLiteral("���� ") + QString::number(-xDis) + " ";
        } else {
            // pass
        }

        if ( yDis > 0 ) {
            str += QStringLiteral("���� ") + QString::number(yDis) + " ";
        } else if ( yDis < 0 ) {
            str += QStringLiteral("���� ") + QString::number(-yDis) + " ";
        } else {
            // pass
        }

        if ( zDis != 0 ) {
            str += QStringLiteral("Z���ƶ� ") + QString::number(zDis);
        } else {
            // pass
        }

        if ( xDis == 0 && yDis == 0 && zDis == 0 ) {
            str += QStringLiteral("��׼���");
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
    // ��ѧϵͳ���ܵĺ��������кܶ࣬�������е���ɢ�������������жϷ�������ͼ������ѧ���ݺ���(MTF)���ȡ�
    // ���� MTF ���ڹ�ѧϵͳ�;�ͷ�ӹ�������ʹ����Ϊ�㷺��MTF ������ʵ�ķ�ӳ�˳���ϵͳ���﷽��Ϣ���ݵ��񷽵�������

    // MTF�ļ��㷽���кܶ࣬����ʹ������Ŀ��ķ��������Ŀ�귽��������ɢ���������������޼���ɢ�߷�����б��Ե��(Slanted Edge Method)�ȣ�
    // ���е���ɢ����������б��Ե���ǹ�ҵ���ձ�ʹ�õķ�����

    // ��б��Ե������: ���Ȼ�ȡ��б��Ե�ı�Ե��ɢ����(ESF)��Ȼ���󵼵õ���Ӧ������ɢ����(LSF)�������Ҷ�任�õ�MTF��

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
            str += QStringLiteral("���� ") + QString::number(xDis) + " ";
        } else if ( xDis < 0 ) {
            str += QStringLiteral("���� ") + QString::number(-xDis) + " ";
        } else {
            // pass
        }

        if ( yDis > 0 ) {
            str += QStringLiteral("���� ") + QString::number(yDis) + " ";
        } else if ( yDis < 0 ) {
            str += QStringLiteral("���� ") + QString::number(-yDis) + " ";
        } else {
            // pass
        }

        if ( zDis != 0 ) {
            str += QStringLiteral("Z���ƶ� ") + QString::number(zDis);
        } else {
            // pass
        }

        if ( xDis == 0 && yDis == 0 && zDis == 0 ) {
            str += QStringLiteral("��׼���");
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

void MainWindow::centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy)
{
    // ȡͼ������ 100x100
    cv::Mat dst = input(cv::Range(input.rows/2 - 50, input.rows/2 + 50), cv::Range(input.cols/2 - 50, input.cols/2 + 50));

    // ת���ɻҶ�ͼ�񲢽���ƽ������
    cv::Mat src_gray;
    cvtColor(dst, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // ��ֵ��
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 100, 255, cv::THRESH_BINARY_INV); // ����ֵС��100���0������100���255

    // ��Ե���
    cv::Mat edge;
    Canny(threshold_output, edge, 50, 200, 3);

    // �����߱任
    vector<cv::Vec2f> lines;
    HoughLines(edge, lines, 1, CV_PI/180, 20, 0, 0);

    vector<cv::Vec4i> landscape_lines;
    vector<cv::Vec4i> portrait_lines;

    // �ж��Ǻ��߻������ߣ���������������
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

    // ���㽻��
    vector<cv::Point2f> vec;
    for ( size_t i = 0; i < landscape_lines.size() && i < 2; ++i )
    {
        for ( size_t j = 0; j < portrait_lines.size() && j < 2; ++j )
        {
            vec.push_back(getCrossPoint(landscape_lines.at(i), portrait_lines.at(j)));
        }
    }

    // �������ĵ�
    cv::Point2f ret(0, 0);
    for ( auto temp : vec )
    {
        ret.x += temp.x;
        ret.y += temp.y;
    }

    // �������ز�
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

void MainWindow::autoAim(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal &xDis, qreal &yDis, qreal &zDis)
{
    // ת���ɻҶ�ͼ�񲢽���ƽ������
    cv::Mat src_gray;
    cvtColor(mat, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // ��ֵ��
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 240, 255, cv::THRESH_BINARY);

    // �ҵ���������
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    if ( contours.size() > 2 ) {
        // ������ֵ��
        threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

        // ɾ����ֵͼ�������С����������ֵ�Ķ���
        bwareaopen(threshold_output, threshold_output, 100);

        // �����㣬�ȸ�ʴ�����ͣ��ܹ��ų�С�ſ�����
//        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11), cv::Point(-1, -1));
//        cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel);

        // ����Ѱ������
        contours.clear();
        hierarchy.clear();
        findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }

    // ����αƽ����� + ��ȡ���κ�Բ�α߽��
    vector<vector<cv::Point>> contours_poly( contours.size() );
    vector<cv::Point2f> center( contours.size() );
    vector<float> radius( contours.size() );

    // ����ÿһ������
    for ( size_t i = 0; i < contours.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );    // ������
        minEnclosingCircle( contours_poly[i], center[i], radius[i] );       // �õ�������ά�㼯����СԲ
    }

    // ȷ�����ĵ�
    int centerX = mat.cols/2 + centerXDis;
    int centerY = mat.rows/2 + centerYDis;
    int boundary = 5;
    cv::Point centerPoint;
    qreal x = 0;
    qreal y = 0;

    // �������ز�
    if ( contours_poly.size() == 0 )
    {
        xDis = 0;
        yDis = 0;
        zDis = 0;
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

        x = centerX - point.x;    // ���������ز�
        y = centerY - point.y;    // ���������ز�

        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
    }

    //---------------------------------

    // ����������� + Բ�ο�
    cv::RNG rng(12345);

    for ( int i = 0; i < (int)contours_poly.size(); ++i )
    {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
//        drawContours( mat, contours_poly, i, color, 1, 8, vector<cv::Vec4i>(), 0, cv::Point() );    // ��������
        circle( mat, center[i], (int)radius[i], color, 2, 8, 0 );                                   // �������Բ

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

void MainWindow::autoAimTest(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal &xDis, qreal &yDis, qreal &zDis)
{
    cv::destroyAllWindows();

    // ת���ɻҶ�ͼ�񲢽���ƽ������
    cv::Mat src_gray;
    cvtColor(mat, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // ��ֵ��
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 240, 255, cv::THRESH_BINARY);

    // �ҵ���������
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    if ( contours.size() > 2 ) {
        // ������ֵ��
        threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);
//        cv::imshow("threshold_output2", threshold_output);

        // ɾ����ֵͼ�������С����������ֵ�Ķ���
        bwareaopen(threshold_output, threshold_output, 100);
//        cv::imshow("threshold_output3", threshold_output);

        // �����㣬�ȸ�ʴ�����ͣ��ܹ��ų�С�ſ�����
//        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
//        cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel);
//        cv::imshow("threshold_output4", threshold_output);

        // ����Ѱ������
        contours.clear();
        hierarchy.clear();
        findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }

    // ����αƽ����� + ��ȡ���κ�Բ�α߽��
    vector<vector<cv::Point>> contours_poly( contours.size() );
    vector<cv::Point2f> center( contours.size() );
    vector<float> radius( contours.size() );

    // ����ÿһ������
    for ( size_t i = 0; i < contours.size(); ++i )
    {
        approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );    // ������
        minEnclosingCircle( contours_poly[i], center[i], radius[i] );       // �õ�������ά�㼯����СԲ
    }

    // ȷ�����ĵ�
    int centerX = mat.cols/2 + centerXDis;
    int centerY = mat.rows/2 + centerYDis;
    int boundary = 5;
    cv::Point centerPoint;
    qreal x = 0;
    qreal y = 0;

    // �������ز�
    if ( contours_poly.size() == 0 )
    {
        xDis = 0;
        yDis = 0;
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

        x = centerX - point.x;    // ���������ز�
        y = centerY - point.y;    // ���������ز�

        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
    }

    //---------------------------------

    // ����������� + Բ�ο�
    cv::RNG rng(12345);

    for ( int i = 0; i < (int)contours_poly.size(); ++i )
    {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
        circle( mat, center[i], (int)radius[i], color, 2, 8, 0 );                                   // �������Բ
    }

    if ( xDis != 0 || yDis != 0 )
    {
        line( mat, cv::Point(centerX, centerY), centerPoint, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    }
}

void MainWindow::test()
{
//    cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim2/1.png").toStdString());
}
