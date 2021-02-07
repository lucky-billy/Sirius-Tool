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

    // ��д�����ļ�ini
    static QString getProperty(const QString &source, QString section, QString key);
    static void setProperty(const QString &source, QString section, QString key, QString value);

    // ��ȡJSON
    static QJsonObject getJsonObj(const QString &source);

    // ��ȡ��ǰʱ��
    static QString getCurrentTime(int type);

    // ��ȡ��ǰ����ִ��·��
    static QString getCurrentPath();

    // ��ȡ��ǰ�߳�ID
    static QString getCurrentThreadID();

    // ˯��
    static void bsleep(int second);

    // �жϸ������Ƿ����
    static bool equals(qreal a, qreal b);

    // �ж��ļ��Ƿ����
    static bool isDirExist(QString &path);

    // �ж��ļ����Ƿ����
    static bool isFileExist(QString &fileName);

    // ɾ���ļ���
    static bool removeFolder(const QString &path);

    // ɾ���ļ�
    static bool removeFile(const QString &fileName);

    // ����csv�ļ�
    static void exportCSV(QString path, QTableWidget *widget);

    // �ж�ͼ���Ƿ����
    static bool judVecAvailable(std::vector<cv::Mat> vec);

    // ��ʾ����
    static void showMessageBox(int type, QString info);

    // ��ȡ����
    static QString getTTF(int id);

    //------------------------------------------------------------------------------

    // �ı�ͼ���в��ֵ����ɫ
    static void changeColorToRed(QImage& image);

    // ����
    static void overExposure(QImage& image);

    // BGR ת GRAY
    static std::vector<cv::Mat> cvtBGR2GRAY(const std::vector<cv::Mat> &vec, cv::Rect rect = cv::Rect(), bool state = false);

    // QImage ת cv::Mat
    static cv::Mat convertQImageToMat(QImage &image);

    // cv::Mat ת QImage
    static QImage convertMatToQImage(const cv::Mat mat);

    // ���㷨�л�ȡ�� mat �����ݴ���ʹ������ڽ�������ʾ��CV_32FC1 ת CV_8UC3
    static cv::Mat dataProcessing(cv::Mat mat, bool state);

    // ����͸����
    static cv::Mat createAlpha(cv::Mat& src);

    // Mat ��ӵ���ͨ�� - alpha ͸����
    static int addAlpha(cv::Mat& src, cv::Mat& dst, cv::Mat& alpha);

    //------------------------------------------------------------------------------

    // �Զ��Խ��㷨
    static void autoAim(cv::Mat src, cv::Mat ori, qreal centerXDis, qreal centerYDis,
                        qreal min_radius, qreal &xDis, qreal &yDis, qreal &zMult);

    // �߶ȱ궨
    static void heightCalibration(cv::Mat src, cv::Mat def, cv::Mat ori, double distance, qreal &radius, qreal &scale_mm);

    // ƽ��궨
    static void planeCalibration(cv::Mat src, cv::Mat def, cv::Mat ori, double distance, qreal &pix_mm);

    // ���ı궨
    static void centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy);

    // �õ�������ά�㼯����СԲ��Բ�ĵİ뾶
    static void getMinCircle(cv::Mat src, cv::Mat ori, cv::Point2f &center, float &radius);

    // ɾ����ֵͼ�������С����������ֵ�Ķ���
    static void bwareaopen(cv::Mat src, cv::Mat &dst, double min_area);

    // ��ȡ��ֱ�ߵĽ���
    static cv::Point2f getCrossPoint(cv::Vec4i lineA, cv::Vec4i lineB);
};

#endif // GLOBALFUN_H
