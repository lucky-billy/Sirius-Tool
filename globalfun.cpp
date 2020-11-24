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
