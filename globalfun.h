#ifndef GLOBALFUN_H
#define GLOBALFUN_H

#include <QTextStream>
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QTableWidget>
#include <QThread>
#include <QMutex>
#include <QScreen>
#include <QTime>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QFontDatabase>

#include <opencv2/opencv.hpp>

#include <iostream>
using namespace std;

class GlobalFun
{
public:
    GlobalFun();

    // 读写配置文件ini
    static QString getProperty(const QString &source, QString section, QString key);
    static void setProperty(const QString &source, QString section, QString key, QString value);

    // 读取JSON
    static QJsonObject getJsonObj(const QString &source);

    // 获取当前时间
    static QString getCurrentTime(int type);

    // 获取当前工程执行路径
    static QString getCurrentPath();

    // 获取当前线程ID
    static QString getCurrentThreadID();

    // 睡眠
    static void bsleep(int second);

    // 判断浮点数是否相等
    static bool equals(qreal a, qreal b);

    // 判断文件是否存在
    static bool isDirExist(QString &path);

    // 判断文件夹是否存在
    static bool isFileExist(QString &fileName);

    // 删除文件夹
    static bool removeFolder(const QString &path);

    // 删除文件
    static bool removeFile(const QString &fileName);

    // 导出csv文件
    static void exportCSV(QString path, QTableWidget *widget);

    // 判断图像是否可用
    static bool judVecAvailable(std::vector<cv::Mat> vec);

    // 提示弹窗
    static void showMessageBox(int type, QString info);

    // 获取字体
    static QString getTTF(int id);

    //------------------------------------------------------------------------------

    // 改变图像中部分点的颜色
    static void changeColorToRed(QImage& image);

    // 过曝
    static void overExposure(QImage& image);

    // BGR 转 GRAY
    static std::vector<cv::Mat> cvtBGR2GRAY(const std::vector<cv::Mat> &vec, cv::Rect rect = cv::Rect(), bool state = false);

    // QImage 转 cv::Mat
    static cv::Mat convertQImageToMat(QImage &image);

    // cv::Mat 转 QImage
    static QImage convertMatToQImage(const cv::Mat mat);

    // 对算法中获取的 mat 做数据处理，使其可以在界面上显示，CV_32FC1 转 CV_8UC3
    static cv::Mat dataProcessing(cv::Mat mat, bool state);

    // 生成透明度
    static cv::Mat createAlpha(cv::Mat& src);

    // Mat 添加第四通道 - alpha 透明度
    static int addAlpha(cv::Mat& src, cv::Mat& dst, cv::Mat& alpha);

    //------------------------------------------------------------------------------

    // 自动对焦算法
    static void autoAim(cv::Mat src, cv::Mat ori, qreal centerXDis, qreal centerYDis,
                        qreal min_radius, qreal &xDis, qreal &yDis, qreal &zMult);

    // 高度标定
    static void heightCalibration(cv::Mat src, cv::Mat def, cv::Mat ori, double distance, qreal &radius, qreal &scale_mm);

    // 平面标定
    static void planeCalibration(cv::Mat src, cv::Mat def, cv::Mat ori, double distance, qreal &pix_mm);

    // 中心标定
    static void centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy);

    // 得到包含二维点集的最小圆的圆心的半径
    static void getMinCircle(cv::Mat src, cv::Mat ori, cv::Point2f &center, float &radius);

    // 删除二值图像中面积小于设置像素值的对象
    static void bwareaopen(cv::Mat src, cv::Mat &dst, double min_area);

    // 获取两直线的交点
    static cv::Point2f getCrossPoint(cv::Vec4i lineA, cv::Vec4i lineB);
};

#endif // GLOBALFUN_H
