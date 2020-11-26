#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "biccameracontrol.h"
#include "bmvcameracontrol.h"
#include "bbaslercamercontrol.h"
#include "bvrcameracontrol.h"

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

    void autoAim(cv::Mat mat, qreal &xDis, qreal &yDis);
    void test();

    void showMask();
    void maskPlus();
    void maskMinus();
    qreal processMTF(QImage &image, QRect &rect);
    void drawMask(QImage &image);

private:
    Ui::MainWindow *ui;
    Camera *m_camera;

    bool mask;
    int width;
    QTimer *test_timer;

    bool is_in_test;
    int count;
};
#endif // MAINWINDOW_H
