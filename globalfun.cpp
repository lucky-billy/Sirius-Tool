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
        return dir.mkdir(path);  // 只创建一级子目录，即必须保证上级目录存在
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

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);     // 设置过滤
    QFileInfoList fileList = dir.entryInfoList();               // 获取所有的文件信息
    foreach (QFileInfo file, fileList)                          // 遍历文件信息
    {
        if ( file.isFile() ) {                                  // 是文件，删除
            file.dir().remove(file.fileName());
        } else {                                                // 递归删除
            removeFolder(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());                      // 删除文件夹
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
    case 1: QMessageBox::question(nullptr, "Question", info, QMessageBox::Ok); break;       // 在正常操作中提出问题
    case 2: QMessageBox::information(nullptr, "Information", info, QMessageBox::Ok); break; // 用于报告有关正常操作的信息
    case 3: QMessageBox::warning(nullptr, "Warning", info, QMessageBox::Ok); break;         // 用于报告非关键错误
    case 4: QMessageBox::critical(nullptr, "Error", info, QMessageBox::Ok); break;          // 用于报告关键错误
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
    default: fileName = "微软雅黑"; break;
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
            cvtColor(temp, gray, cv::COLOR_BGR2GRAY);   // 转换为灰度图像
            gray.convertTo(gray, CV_32FC1);             // C1数据类型转换为32F
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

    cv::Mat tmpMask = mat == mat;   // 不相等为0，相等为1（nan与nan不相等）

    cv::minMaxIdx(mat, &minV, &maxV, nullptr, nullptr, mat == mat); // 找出最小、最大值
    if (abs(maxV - minV) > 0.00000001) {
        mat = (mat - minV) / (maxV - minV) * 255;   // 数值在0~255之间
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

    //分离通道
    cv::split(src, srcChannels);

    dstChannels.push_back(srcChannels[0]);
    dstChannels.push_back(srcChannels[1]);
    dstChannels.push_back(srcChannels[2]);

    //添加透明度通道
    dstChannels.push_back(alpha);

    //合并通道
    cv::merge(dstChannels, dst);

    return 0;
}

//------------------------------------------------------------------------------

void GlobalFun::autoAim(cv::Mat mat, qreal centerXDis, qreal centerYDis, qreal min_radius, qreal &xDis, qreal &yDis, qreal &zMult)
{
    // 确定中心点
    int centerX = mat.cols/2 + centerXDis;
    int centerY = mat.rows/2 + centerYDis;
    int boundary = 10;
    cv::Point centerPoint;

    //---------------------------------

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

    if ( contours.size() > 2 )
    {
        // 三角阈值法
        threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

        // 删除二值图像中面积小于设置像素值的对象
        bwareaopen(threshold_output, threshold_output, 100);

        // 闭运算
        cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9), cv::Point(-1, -1));
        cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_CLOSE, kernel1);

        // 开运算
        cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
        cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel2);

        // 重新寻找轮廓
        contours.clear();
        hierarchy.clear();
        findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }
    else {
        // 检测阈值化后的结果
        if ( contours.size() == 2 ) {
            // 三角阈值法
            threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

            // 闭运算
            cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9), cv::Point(-1, -1));
            cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_CLOSE, kernel1);

            // 开运算
            cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5), cv::Point(-1, -1));
            cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel2);

            // 临时存储轮廓
            vector<vector<cv::Point> > temp_contours;
            vector<cv::Vec4i> temp_tehierarchy;

            // 寻找轮廓
            findContours(threshold_output, temp_contours, temp_tehierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

            if ( contours.size() > temp_contours.size() ) {
                // 两个图形变成一个图形
                size_t c_ts = 0;
                size_t t_ts = 0;

                for ( size_t i = 0; i < contours.size(); ++i )
                {
                    c_ts += contours[i].size();
                }
                for ( size_t i = 0; i < temp_contours.size(); ++i )
                {
                    t_ts += temp_contours[i].size();
                }

                if ( c_ts * 3 < t_ts ) {
                    // 三角阈值法
                    threshold(src_gray, threshold_output, 0, 255, cv::THRESH_TRIANGLE);

                    // 闭运算
                    cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15), cv::Point(-1, -1));
                    cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_CLOSE, kernel1);

                    // 开运算
                    cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15), cv::Point(-1, -1));
                    cv::morphologyEx(threshold_output, threshold_output, cv::MORPH_OPEN, kernel2);

                    // 重新寻找轮廓
                    contours.clear();
                    hierarchy.clear();
                    findContours(threshold_output, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
                }
            }
        }
    }

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

//    // 画多边形轮廓 + 圆形框
//    cv::RNG rng(12345);
//    qreal my_radius = 0;
//    cv::Point my_center;

//    for ( int i = 0; i < (int)contours_poly.size(); ++i )
//    {
//        if ( radius[i] > my_radius && (abs(centerX - center[i].x) >= boundary || abs(centerY - center[i].y) >= boundary) ) {
//            my_radius = radius[i];
//            my_center = center[i];
//        }
//    }
//    cv::Scalar color = cv::Scalar( rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255) );
//    circle( mat, my_center, (int)my_radius, color, 2, 8, 0 );

//    if ( xDis != 0 || yDis != 0 )
//    {
//        line( mat, cv::Point(centerX, centerY), centerPoint, cv::Scalar(0, 0, 255), 1, cv::LINE_AA );
//    }
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

void GlobalFun::heightCalibration(cv::Mat origin, cv::Mat deformation, double distance, qreal &radius, qreal &scale_mm)
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
    int boundary = 10;

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

void GlobalFun::planeCalibration(cv::Mat origin, cv::Mat deformation, double distance, qreal &pix_mm)
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
    int boundary = 10;

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

void GlobalFun::centerCalibration(cv::Mat input, cv::Mat &output, qreal &dx, qreal &dy)
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
