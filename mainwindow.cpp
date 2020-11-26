#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globalfun.h"
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    is_in_test = false;

    if ( !is_in_test ) {
        initData();
        initView();
    } else {
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

        cv::Mat mat = cv::imread(QString("../Sirius-Tool/test/aim/%1.png").arg(count).toStdString());
        qreal xDis = 0;
        qreal yDis = 0;
        autoAim(mat, xDis, yDis);

        QString str = "";
        if ( xDis > 0 && yDis > 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(xDis) + QStringLiteral(", ���� �ƶ� ") + QString::number(yDis);
        } else if ( xDis > 0 && yDis < 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(xDis) + QStringLiteral(", ���� �ƶ� ") + QString::number(-yDis);
        } else if ( xDis > 0 && yDis == 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(xDis);
        } else if ( xDis < 0 && yDis > 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(-xDis) + QStringLiteral(", ���� �ƶ� ") + QString::number(yDis);
        } else if ( xDis < 0 && yDis < 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(-xDis) + QStringLiteral(", ���� �ƶ� ") + QString::number(-yDis);
        } else if ( xDis < 0 && yDis == 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(-xDis);
        } else if ( xDis == 0 && yDis > 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(yDis);
        } else if ( xDis == 0 && yDis < 0 ) {
            str = QStringLiteral("��Ҫ ���� �ƶ� ") + QString::number(-yDis);
        } else if ( xDis == 0 && yDis == 0 ) {
            str = QStringLiteral("��׼���");
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
        if ( count == 10 ) { count = 1; }
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

//******************************************************************************************************************

void MainWindow::autoAim(cv::Mat mat, qreal &xDis, qreal &yDis)
{
    // ת���ɻҶ�ͼ�񲢽���ƽ������
    cv::Mat src_gray;
    cvtColor(mat, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, src_gray, cv::Size(3,3));

    // ��ֵ��
    cv::Mat threshold_output;
    threshold(src_gray, threshold_output, 250, 255, cv::THRESH_BINARY);

    // �ҵ���������
    vector<vector<cv::Point> > contours;
    vector<cv::Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    if ( contours.size() == 2 ) {
        // ����αƽ����� + ��ȡ���κ�Բ�α߽��
        vector<vector<cv::Point>> contours_poly( contours.size() );
        vector<cv::Rect> boundRect( contours.size() );
        vector<cv::Point2f> center( contours.size() );
        vector<float> radius( contours.size() );

        // ����ÿһ������
        for ( size_t i = 0; i < contours.size(); ++i )
        {
            approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
            boundRect[i] = boundingRect( cv::Mat(contours_poly[i]) );
            minEnclosingCircle( contours_poly[i], center[i], radius[i] );   // �õ�������ά�㼯����СԲ
        }

        int centerX = mat.cols/2;
        int centerY = mat.rows/2;
        cv::Point c;
        cv::Point p;
        for ( auto &temp : center )
        {
            if ( abs(centerX - temp.x) <= 10 && abs(centerY - temp.y) <= 10 ) {
                c = temp;
            } else {
                p = temp;
            }
        }

        qreal x = c.x - p.x;    // ���������ز�
        qreal y = c.y - p.y;    // ���������ز�

        xDis = abs(x) <= 10 ? 0 : x;
        yDis = abs(y) <= 10 ? 0 : y;
    } else {
        xDis = 0;
        yDis = 0;
    }

    //---------------------------------

    // ����������� + ��Χ�ľ��ο� + Բ�ο�
//    cv::RNG rng(12345);
//    cv::Mat drawing = cv::Mat::zeros( threshold_output.size(), CV_8UC3 );

//    for ( int i = 0; i < (int)contours.size(); ++i )
//    {
//        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//        drawContours( drawing, contours_poly, i, color, 1, 8, vector<cv::Vec4i>(), 0, cv::Point() );    // ��������
//        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );                     // ������Ӿ���
//        circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );                                   // �������Բ
//    }

//    imshow( "Contours", drawing );
}

void MainWindow::test()
{
    // pass
}
