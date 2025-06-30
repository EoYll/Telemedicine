#include "Telemedicine.h"
#include <QtWidgets/QApplication>
#include<qprocess.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (!createMySqlConn()) {
        QProcess process;
        process.start("net", { "start", "mysql80" }); // 启动已注册的服务
        process.waitForFinished(5000); // 等待命令执行
    }
    Telemedicine w;
    w.show();
    return a.exec();
}
