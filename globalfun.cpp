#include "globalfun.h"
#include "windows.h"

GlobalFun::GlobalFun()
{

}

QString GlobalFun::getProperty(const QString &source, QString section, QString key)
{
    QString path = section + "/" + key;
    QSettings settings(source, QSettings::IniFormat);
    return settings.value(path, "").toString();
}

void GlobalFun::setProperty(const QString &source, QString section, QString key, QString value)
{
    QString path = section + "/" + key;
    QSettings settings(source, QSettings::IniFormat);
    settings.setValue(path, value);
}

QJsonObject GlobalFun::getJsonObj(const QString &source)
{
    QFile file(source);

    if( !file.open(QFile::ReadOnly) ) {
        QJsonDocument doc = QJsonDocument::fromJson("", nullptr);
        return doc.object();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data, nullptr);
    return doc.object();
}

QString GlobalFun::getCurrentTime(int type)
{
    QDateTime time(QDateTime::currentDateTime());
    switch (type)
    {
    case 1: return time.toString("yyyy-MM-dd-hh_mm_ss");
    case 2: return time.toString("yyyy-MM-dd_hh:mm:ss.zzz");
    case 3: return time.toString("yyyy-MM-dd");
    case 4: return time.toString("yyyy-MM");
    case 5: return time.toString("yyyy-MM-dd hh:mm:ss");
    default: return "";
    }
}

QString GlobalFun::getCurrentPath()
{
    return QDir::currentPath();
}

QString GlobalFun::getCurrentThreadID()
{
    QString str = "";
    str.sprintf("%p", QThread::currentThread());
    return str;
}

void GlobalFun::bsleep(int second)
{
    QTime timer;
    timer.start();
    while ( timer.elapsed() < second )
    {
        QCoreApplication::processEvents();
    }
}

bool GlobalFun::equals(qreal a, qreal b)
{
    return abs(a-b) < pow(2, -52);
}

bool GlobalFun::isDirExist(QString &path)
{
    QDir dir(path);

    if( dir.exists() ) {
        return true;
    } else {
        return dir.mkdir(path);  // ֻ����һ����Ŀ¼�������뱣֤�ϼ�Ŀ¼����
    }
}

bool GlobalFun::isFileExist(QString &fileName)
{
    QFileInfo fileInfo(fileName);
    return fileInfo.isFile();
}

bool GlobalFun::removeFolder(const QString &path)
{
    if ( path.isEmpty() ) {
        return false;
    }

    QDir dir(path);
    if( !dir.exists() ) {
        return true;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);     // ���ù���
    QFileInfoList fileList = dir.entryInfoList();               // ��ȡ���е��ļ���Ϣ
    foreach (QFileInfo file, fileList)                          // �����ļ���Ϣ
    {
        if ( file.isFile() ) {                                  // ���ļ���ɾ��
            file.dir().remove(file.fileName());
        } else {                                                // �ݹ�ɾ��
            removeFolder(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());                      // ɾ���ļ���
}

bool GlobalFun::removeFile(const QString &fileName)
{
    if( QFile::exists(fileName) ) {
        return QFile(fileName).remove();
    } else {
        return true;
    }
}

void GlobalFun::exportCSV(QString path, QTableWidget *widget)
{
    QFile file(path);
    if ( file.open(QFile::WriteOnly | QFile::Truncate) )
    {
        QTextStream data(&file);
        QStringList linelist;
        for (int i = 0; i != widget->columnCount(); ++i) {
            linelist.append(widget->horizontalHeaderItem(i)->text());
        }
        data << linelist.join(",") + "\n";

        for (int i = 0; i != widget->rowCount(); ++i) {
            linelist.clear();
            for (int j = 0; j != widget->columnCount(); ++j) {
                QString str = widget->item(i, j)->text();
                linelist.append(str);
            }
            data << linelist.join(",") + "\n";
        }
        file.close();
    }
}

bool GlobalFun::judVecAvailable(std::vector<cv::Mat> vec)
{
    if ( vec.size() == 0 ) {
        return false;
    }

    for ( auto &temp: vec )
    {
        if ( temp.empty() ) {
            return false;
        }
    }

    return true;
}

void GlobalFun::showMessageBox(int type, QString info)
{
    switch ( type )
    {
    case 1: QMessageBox::question(nullptr, "Question", info, QMessageBox::Ok); break;       // �������������������
    case 2: QMessageBox::information(nullptr, "Information", info, QMessageBox::Ok); break; // ���ڱ����й�������������Ϣ
    case 3: QMessageBox::warning(nullptr, "Warning", info, QMessageBox::Ok); break;         // ���ڱ���ǹؼ�����
    case 4: QMessageBox::critical(nullptr, "Error", info, QMessageBox::Ok); break;          // ���ڱ���ؼ�����
    default: break;
    }
}

QString GlobalFun::getTTF(int id)
{
    QString fileName = "";

    switch (id)
    {
    case 1: fileName = ":/source/ttf/BalooBhaina-Regular.ttf"; break;
    case 2: fileName = ":/source/ttf/BRUX.otf"; break;
    case 3: fileName = ":/source/ttf/Aldrich-Regular.ttf"; break;
    case 4: fileName = ":/source/ttf/Branch-zystoo.otf"; break;
    case 5: fileName = ":/source/ttf/Times_New_Roman.ttf"; break;
    default: fileName = "΢���ź�"; break;
    }

    int fontId = QFontDatabase::addApplicationFont(fileName);
    QString ttf = QFontDatabase::applicationFontFamilies(fontId).at(0);
    return ttf;
}

//------------------------------------------------------------------------------

void GlobalFun::changeColorToRed(QImage& image)
{
    if ( image.format() == QImage::Format_RGB888 ) {
        cv::Mat3b mat = cv::Mat(image.height(), image.width(), CV_8UC3, image.bits());
        std::vector<cv::Mat1b> rgbChannels;
        cv::split(mat, rgbChannels);
        cv::Mat1b red = rgbChannels[0];
        mat.setTo(cv::Vec3b{255, 0, 0}, red >= 254);
    }
}

void GlobalFun::overExposure(QImage& image)
{
//    DWORD start_time = GetTickCount();

    unsigned char *data = image.bits();
    int width = image.width();
    int height = image.height();

    if ( image.format() == QImage::Format_RGB32 )
    {
        for ( int i = 0; i < width; ++i )
        {
            for ( int j = 0; j < height; ++j )
            {
                if ( *data >= 254 && *(data + 1) >= 254 && *(data + 2) >= 254 )
                {
                    *data = 0;
                    *(data + 1) = 0;
                }
                data += 4;
            }
        }
    }
    else if ( image.format() == QImage::Format_RGB888 )
    {
        for ( int i = 0; i < width; ++i )
        {
            for ( int j = 0; j < height; ++j )
            {
                if ( *data >= 254 && *(data + 1) >= 254 && *(data + 2) >= 254 )
                {
                    *(data + 1) = 0;
                    *(data + 2) = 0;
                }
                data += 3;
            }
        }
    }

//    DWORD end_time = GetTickCount();
//    std::cout << "times = " << end_time - start_time << std::endl;
}

std::vector<cv::Mat> GlobalFun::cvtBGR2GRAY(const std::vector<cv::Mat> &vec, cv::Rect rect, bool state)
{
    std::vector<cv::Mat> ret;

    for ( auto &temp : vec )
    {
        if ( temp.empty() ) {
            qDebug() << "Image is empty !";
            ret.push_back(cv::Mat());
        } else {
            cv::Mat gray;
            cvtColor(temp, gray, cv::COLOR_BGR2GRAY);   // ת��Ϊ�Ҷ�ͼ��
            gray.convertTo(gray, CV_32FC1);             // C1��������ת��Ϊ32F
            if ( state ) {
                ret.push_back(gray(rect));
            } else {
                ret.push_back(gray);
            }
        }
    }

    return ret;
}

cv::Mat GlobalFun::convertQImageToMat(QImage &image)
{
    cv::Mat mat;

    switch( image.format() )
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    default: mat = cv::Mat(); break;
    }

    return mat.clone();
}

QImage GlobalFun::convertMatToQImage(const cv::Mat mat)
{
    if( mat.type() == CV_8UC1 )
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for( int i = 0; i < 256; i++ )
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for( int row = 0; row < mat.rows; row++ )
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }

    else if( mat.type() == CV_8UC3 )
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, (int)mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }

    else if( mat.type() == CV_8UC4 )
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, (int)mat.step, QImage::Format_RGB32);
        return image.copy();
    }

    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

cv::Mat GlobalFun::dataProcessing(cv::Mat mat, bool state)
{
    double minV = 0, maxV = 0;

    cv::Mat tmpMask = mat == mat;   // �����Ϊ0�����Ϊ1��nan��nan����ȣ�

    cv::minMaxIdx(mat, &minV, &maxV, nullptr, nullptr, mat == mat); // �ҳ���С�����ֵ
    if (abs(maxV - minV) > 0.00000001) {
        mat = (mat - minV) / (maxV - minV) * 255;   // ��ֵ��0~255֮��
    }

    mat.convertTo(mat, CV_8UC1);
    cv::applyColorMap(mat, mat, cv::COLORMAP_JET);

    if ( state ) {
        mat.setTo(cv::Vec3b(255, 255, 255), ~tmpMask);
    } else {
        mat.setTo(nan(""), ~tmpMask);
    }
    return mat.clone();
}

cv::Mat GlobalFun::createAlpha(cv::Mat& src)
{
    cv::Mat alpha = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
    cv::Mat gray = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);

    cv::cvtColor(src, gray, cv::COLOR_RGB2GRAY);

    for ( int i = 0; i < src.rows; ++i )
    {
        for ( int j = 0; j < src.cols; ++j )
        {
            alpha.at<uchar>(i, j) = 255 - gray.at<uchar>(i, j);
        }
    }

    return alpha;
}

int GlobalFun::addAlpha(cv::Mat& src, cv::Mat& dst, cv::Mat& alpha)
{
    if ( src.channels() == 4 ) {
        return -1;
    } else if ( src.channels() == 1 ) {
        cv::cvtColor(src, src, cv::COLOR_GRAY2RGB);
    }

    dst = cv::Mat(src.rows, src.cols, CV_8UC4);

    std::vector<cv::Mat> srcChannels;
    std::vector<cv::Mat> dstChannels;

    //����ͨ��
    cv::split(src, srcChannels);

    dstChannels.push_back(srcChannels[0]);
    dstChannels.push_back(srcChannels[1]);
    dstChannels.push_back(srcChannels[2]);

    //���͸����ͨ��
    dstChannels.push_back(alpha);

    //�ϲ�ͨ��
    cv::merge(dstChannels, dst);

    return 0;
}

//------------------------------------------------------------------------------

void GlobalFun::autoAim(cv::Mat src, cv::Mat ori, qreal centerXDis, qreal centerYDis,
                        qreal min_radius, qreal &xDis, qreal &yDis, qreal &zMult, float &radius)
{
    // ȷ�����ĵ�
    int centerX = src.cols/2 + centerXDis;
    int centerY = src.rows/2 + centerYDis;
    int boundary = 5;
    cv::Point centerPoint;

    // �õ�������ά�㼯����СԲ��Բ�ĵİ뾶
    cv::Point2f center;
    getMinCircle(src, ori, center, radius);

    if ( radius == 0 ) {
        xDis = 0;
        yDis = 0;
        zMult = 0;
    } else {
        centerPoint.x = center.x;
        centerPoint.y = center.y;

        qreal x = centerX - centerPoint.x;
        qreal y = centerY - centerPoint.y;
        qreal z = radius / min_radius;;
        xDis = abs(x) >= boundary ? x : 0;
        yDis = abs(y) >= boundary ? y : 0;
        zMult = z > 1.5 ? z : 0;
    }

    //---------------------------------

    // ����Բ��
    if ( radius != 0 )
    {
        cv::RNG rng(12345);
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
        circle( src, center, (int)radius, color, 2, 8, 0 );
    }

    if ( xDis != 0 || yDis != 0 )
    {
        line( src, cv::Point(centerX, centerY), centerPoint, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    }
}

void GlobalFun::heightCalibration(cv::Mat src, cv::Mat def, cv::Mat ori, double distance, qreal &radius, qreal &scale_mm)
{
    // �õ�������ά�㼯����СԲ��Բ�ĵİ뾶
    cv::Point2f src_center, def_center;
    float src_radius, def_radius;
    getMinCircle(src, ori, src_center, src_radius);
    getMinCircle(def, ori, def_center, def_radius);

    if ( src_radius == 0 || def_radius == 0 ) {
        radius = -1;
        scale_mm = -1;
    } else {
        // ����궨ֵ
        qreal scale = 0;

        if ( src_radius < def_radius ) {
            radius = src_radius;
            scale = def_radius / src_radius;
        } else {
            radius = def_radius;
            scale = src_radius / def_radius;
        }

        scale_mm = distance / scale;

        cv::RNG rng(12345);
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
        circle( src, src_center, (int)src_radius, color, 2, 8, 0 );
        circle( def, def_center, (int)def_radius, color, 2, 8, 0 );

        string str = QString::number(def_radius).toStdString();
        cv::Point2f org;
        org.x = def_center.x - def_radius - 5;
        org.y = def_center.y - def_radius - 20;
        putText( def, str, org, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
    }
}

void GlobalFun::planeCalibration(cv::Mat src, cv::Mat def, cv::Mat ori, double distance, qreal &pix_mm)
{
    // �õ�������ά�㼯����СԲ��Բ�ĵİ뾶
    cv::Point2f src_center, def_center;
    float src_radius, def_radius;
    getMinCircle(src, ori, src_center, src_radius);
    getMinCircle(def, ori, def_center, def_radius);

    if ( src_radius == 0 || def_radius == 0 ) {
        pix_mm = -1;
    } else {
        qreal dx = abs(def_center.x - src_center.x);
        qreal dy = abs(def_center.y - src_center.y);
        qreal pix = dx < dy ? dy : dx;
        pix_mm = distance / pix;

        cv::Point2f org;
        if ( pix == dy ) {
            org.x = src_center.x + 20;
            org.y = (src_center.y + def_center.y) / 2;
        } else {
            org.x = src_center.x < def_center.x ? src_center.x : def_center.x;
            org.y = src_center.y - 20;
        }

        string str = QString::number(pix).toStdString();

        putText( def, str, org, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
        line( def, src_center, def_center, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );

        cv::RNG rng(12345);
        cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
        circle( def, src_center, (int)src_radius, color, 2, 8, 0 );
        circle( def, def_center, (int)def_radius, color, 2, 8, 0 );
    }
}

void GlobalFun::centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy)
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

void GlobalFun::getMinCircle(cv::Mat src, cv::Mat ori, cv::Point2f &center, float &radius)
{
    // ת���ɻҶ�ͼ�񲢽���ƽ������
    cv::Mat src_gray, ori_gray, gray;
    cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
    cvtColor(ori, ori_gray, cv::COLOR_BGR2GRAY);
    gray = src_gray - ori_gray;
    blur(gray, gray, cv::Size(3,3));

    // ������ֵ��
    cv::Mat threshold_output;
    threshold(gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

    // ����С�����õ�����ͼ��
    bwareaopen(threshold_output, threshold_output, 30);

    // �ҵ���������
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE, cv::Point(0, 0));

    // ���ݳ�ʼ��
    center = cv::Point2f(0, 0);
    radius = 0;

    if ( contours.size() != 0 )
    {
        size_t index = 0;
        size_t maxSize = 0;

        // Ѱ�����ĵ㼯
        for ( size_t i = 0; i < contours.size(); ++i )
        {
            if ( contours[i].size() > maxSize ) {
                maxSize = contours[i].size();
                index = i;
            }
        }

        // �õ�������ά�㼯����СԲ��Բ�ĵİ뾶
        vector<cv::Point> contours_poly = contours[index];
        minEnclosingCircle(contours_poly, center, radius);
    }
}

void GlobalFun::bwareaopen(cv::Mat src, cv::Mat &dst, double min_area)
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

cv::Point2f GlobalFun::getCrossPoint(cv::Vec4i lineA, cv::Vec4i lineB)
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
