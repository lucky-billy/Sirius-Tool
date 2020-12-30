#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "biccameracontrol.h"
#include "bmvcameracontrol.h"
#include "bbaslercamercontrol.h"
#include "bvrcameracontrol.h"

#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initData();
    void initView();

    void showMask();
    void maskPlus();
    void maskMinus();
    qreal processMTF(QImage &image, QRect &rect);
    void drawMask(QImage &image);

    void heightCalibration(cv::Mat origin, cv::Mat deformation, int distance, qreal &radius, qreal &scale_mm);
    void planeCalibration(cv::Mat origin, cv::Mat deformation, int distance, qreal &pix_mm);
    void centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy);
    cv::Point2f getCrossPoint(cv::Vec4i lineA, cv::Vec4i lineB);

    void bwareaopen(cv::Mat src, cv::Mat &dst, double min_area);
    void autoAim(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal &xDis, qreal &yDis, qreal &zMult);
    void autoAimTest(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal &xDis, qreal &yDis, qreal &zMult);
    void test();

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::MainWindow *ui;
    Camera *m_camera;

    bool mask;
    int width;
    QTimer *test_timer;

    bool is_in_test;
    int count;
    qreal centerXDis;
    qreal centerYDis;
};
#endif // MAINWINDOW_H
