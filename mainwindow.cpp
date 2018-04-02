#include <QDebug>
#include <iostream>
#include <QDateTime>
#include <QApplication>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bk1696.h"

static BK1696 bk;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _timer(this)
{
    ui->setupUi(this);

    QObject::connect(&bk, &BK1696::voltageChanged, [this]() { this->ui->lcdVoltage->display(bk.voltage); });
    QObject::connect(&bk, &BK1696::currentChanged, [this]() { this->ui->lcdCurrent->display(bk.current); });
    QObject::connect(&bk, &BK1696::powerChanged,   [this]() { this->ui->lcdPower->display(bk.power); });
    QObject::connect(&this->_timer, &QTimer::timeout, [this]() {
        if (bk.isOpen())
            bk.update();
        else
            this->_timer.stop();
    });

    foreach (const QString &port, BK1696::ports()) {
        this->ui->port->addItem(port);
    }
    this->ui->port->setCurrentIndex(0);

    this->_timer.setInterval(250);
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

void MainWindow::on_open_clicked()
{
    bk.open(this->ui->port->currentText(), [this]() { this->_timer.start(); });
}

void MainWindow::on_close_clicked()
{
    this->_timer.stop();
    bk.close();
}

void MainWindow::on_on_clicked()
{
    this->_timer.stop();
    bk.outputEnable(true, [this]() { this->_timer.start(); });
}

void MainWindow::on_off_clicked()
{
    this->_timer.stop();
    bk.outputEnable(false, [this]() { this->_timer.start(); });
}

void MainWindow::on_clear_clicked()
{
    this->ui->log->clear();
}
