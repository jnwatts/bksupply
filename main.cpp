#include "mainwindow.h"
#include <QApplication>
#include "bkserial.h"

MainWindow *w = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    w = new MainWindow();
    qInstallMessageHandler([] (QtMsgType type, const QMessageLogContext &context, const QString &msg) -> void {
        w->handleMessage(type, context, msg);
    });
    w->show();

    return a.exec();
}
