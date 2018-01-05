#include <iostream>
#include <QDateTime>
#include <QApplication>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bk1696.h"

static BK1696 bk;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&bk, SIGNAL(response(QString)), this, SLOT(response(QString)));
    connect(&bk, SIGNAL(success()), this, SLOT(success()));

    foreach (const QString &port, BK1696::ports()) {
        this->ui->port->addItem(port);
    }
    this->ui->port->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::ostream *out = &std::cout;
    QString prefix;

    prefix = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc()).toString(Qt::ISODate);
    prefix += " ";

    switch (type) {
    case QtDebugMsg:
        prefix += "[DBG] ";
        break;
    case QtWarningMsg:
        prefix += "[WARN] ";
        out = &std::cerr;
        break;
    case QtCriticalMsg:
        prefix += "[CRIT] ";
        out = &std::cerr;
        break;
    case QtFatalMsg:
        prefix += "[FATAL] ";
        out = &std::cerr;
        break;
    case QtInfoMsg:
        prefix += "[INFO] ";
        break;
    }

    if (context.category) {
        QString category = context.category;
        prefix.append(category);
        prefix.append(": ");
    }

    *out << prefix.toStdString() << msg.toStdString() << std::endl;
    this->ui->log->appendPlainText(prefix + msg);
}

void MainWindow::response(QString data)
{
    qInfo(">> %s", qPrintable(data));
}

void MainWindow::success()
{
    qInfo("SUCCESS");
}

void MainWindow::on_open_clicked()
{
    bk.open(this->ui->port->currentText());
}

void MainWindow::on_close_clicked()
{
    bk.close();
}

void MainWindow::on_on_clicked()
{
    bk.outputEnable(true);
}

void MainWindow::on_off_clicked()
{
    bk.outputEnable(false);
}

void MainWindow::on_clear_clicked()
{
    this->ui->log->clear();
}