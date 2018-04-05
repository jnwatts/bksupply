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

    this->ui->lblVoltage->setMinimumWidth(QFontMetrics(this->ui->lblVoltage->font()).width("00.000") * 1.25);
    this->ui->lblCurrent->setMinimumWidth(this->ui->lblVoltage->minimumWidth());
    this->ui->lblPower->setMinimumWidth(this->ui->lblVoltage->minimumWidth());

    QObject::connect(&bk, &BK1696::voltageChanged, [this]() { this->display(this->ui->lblVoltage, bk.voltage, 3); });
    QObject::connect(&bk, &BK1696::currentChanged, [this]() { this->display(this->ui->lblCurrent, bk.current, 3); });
    QObject::connect(&bk, &BK1696::powerChanged,   [this]() { this->display(this->ui->lblPower,   bk.power,   3); });
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

    this->update();
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
    bk.open(this->ui->port->currentText(), [this]() { this->_timer.start(); this->update(); });
}

void MainWindow::on_close_clicked()
{
    this->_timer.stop();
    bk.close();
    this->update();
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

void MainWindow::display(QLabel *label, double value, int precision)
{
    QString str = QString::number(value, 'f', precision);
    label->setText(str);
}

void MainWindow::update()
{
    this->ui->lblVoltage->setEnabled(bk.isOpen());
    this->ui->lblCurrent->setEnabled(bk.isOpen());
    this->ui->lblPower->setEnabled(bk.isOpen());
    this->ui->on->setEnabled(bk.isOpen());
    this->ui->off->setEnabled(bk.isOpen());
    this->ui->open->setEnabled(!bk.isOpen());
    this->ui->close->setEnabled(bk.isOpen());

}
