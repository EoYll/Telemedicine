#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Telemedicine.h"
#include"opencv2/opencv.hpp"
#include<qsqldatabase.h>
#include<qsqltablemodel.h>
#include<qtimer.h>
#include<qtimezone.h>
#include<qfiledialog.h>
#include<qmessagebox.h>
#include<qbuffer.h>
#include<qsqlerror.h>
#include <QPluginLoader>
#include <QDebug>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>  // 正确使用 Qt 的并发头文件
#include <QElapsedTimer>
using namespace cv;
using namespace std;
class Telemedicine : public QMainWindow
{
    Q_OBJECT

public:
    Telemedicine(QWidget *parent = nullptr);
    ~Telemedicine();
    //初始化主体
    void initMainWindow();
    void ctImgRead();//读取ct
    void ctImgProc();//处理ct
    void ctImgSave();//保存ct
    void ctImgShow();//显示ct                              
    void ctImgHoughCircles();
    void onTableSelectChange(int row);
    void showUserPhoto(); //加载显示患者图片
private slots:
    void on_startPushButton_clicked();
    void on_basicTableView_clicked(const QModelIndex& index);
    void on_tabWidget_tabBarClicked(int index);
    void onTimeOut();

private:
    Ui::TelemedicineClass ui;
    Mat myCtImg;
    Mat myCtGrayImg;
    QImage myCtQImage;
    QSqlTableModel* model;
    QSqlTableModel* model_d;
    QTimer* myTimer;
    // 在类声明中定义（确保生命周期）
    QTimer time1;
    QTimer time2;
    int flag = 0;  // 改为成员变量
};
static bool createMySqlConn()
{
    QSqlDatabase sqldb = QSqlDatabase::addDatabase("QMYSQL");
    if (!sqldb.isValid()) {
        // 诊断驱动加载失败的具体原因
        qDebug() << "QMYSQL driver not loaded. Error:" << sqldb.lastError().text();

        // 检查插件是否存在
        QPluginLoader loader("qsqlmysql");
        if (!loader.load()) {
            qDebug() << "Plugin load error:" << loader.errorString();
            QMessageBox::critical(nullptr, "Plugin Error",
                "Failed to load MySQL driver:\n" + loader.errorString());
        }
        else {
            qDebug() << "Driver loaded successfully but still not valid";
        }
        return false;
    }
    sqldb.setHostName("localhost");
    sqldb.setDatabaseName("patient");
    sqldb.setUserName("root");
    sqldb.setPassword("123456");
    if (!sqldb.open()) {
        qDebug() << QSqlDatabase::drivers(); // 检查可用驱动列表
        QMessageBox::critical(nullptr, QObject::tr("后台数据库连接失败"), QString("无法创建连接！请检查排除故障后重启程序."), QMessageBox::Cancel);
        return false;
    }
    QMessageBox::information(0, QObject::tr("后台数据库已启动，正在运行……"), "数据库连接成功！即将启动应用程序");
    sqldb.close();
    return true;
}