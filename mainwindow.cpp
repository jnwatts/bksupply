#include <QSettings>
#include <QFileDialog>
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
    _settings("SROZ", "bksupply"),
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

    this->loadSettings();

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
    bk.open(this->ui->port->currentText(), [this]() { this->_timer.start(); this->update(); });
}

void MainWindow::on_close_clicked()
{
    this->_timer.stop();
    bk.close([this]() { this->update(); });
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
    this->ui->port->setEnabled(!bk.isOpen());
    this->ui->addPort->setEnabled(!bk.isOpen());
    this->ui->delPort->setEnabled(!bk.isOpen() && this->isSelectedPortCustom());
}

void MainWindow::updatePorts(void)
{
    this->ui->port->clear();
    for (int i = 0; i < this->_customPorts.size(); ++i) {
        this->ui->port->addItem(this->_customPorts.at(i), QVariantList({"CUSTOM", i}));
    }
    for (int i = 0; i < BK1696::ports().size(); ++i) {
        this->ui->port->addItem(BK1696::ports().at(i), QVariantList({"BK", i}));
    }
}

void MainWindow::addCustomPort(QString port)
{
    this->_customPorts << port;
    this->saveSettings();
    this->updatePorts();
}

void MainWindow::removeCustomPort(int index)
{
    this->_customPorts.remove(index);
    this->saveSettings();
    this->updatePorts();
}

bool MainWindow::isSelectedPortCustom()
{
    if (this->ui->port->currentIndex() < 0)
        return false;
    QVariantList selectedItem = this->ui->port->currentData().toList();
    QString type = selectedItem.at(0).toString();
    return (type == "CUSTOM");
}

void MainWindow::saveSettings(void)
{
    QSettings &settings = this->settings();
    settings.beginWriteArray("custom_ports");
    for (int i = 0; i < this->_customPorts.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("path", this->_customPorts.at(i));
    }
    settings.endArray();
    settings.setValue("selected_port", this->ui->port->currentIndex());
}

void MainWindow::loadSettings(void)
{
    QSettings &settings = this->settings();
    int size;

    size = settings.beginReadArray("custom_ports");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString port = settings.value("path", "").toString();
        if (port.size() > 0)
            this->_customPorts << port;
    }
    settings.endArray();
    this->updatePorts();

    int selected_port = settings.value("selected_port", 0).toInt();
    if (selected_port >= this->ui->port->count() || selected_port < 0)
        selected_port = 0;
    this->ui->port->setCurrentIndex(selected_port);

    this->update();
}

void MainWindow::on_addPort_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setResolveSymlinks(false);
    if (dialog.exec())
        this->addCustomPort(dialog.selectedFiles().at(0));
}

void MainWindow::on_delPort_clicked()
{
    if (this->ui->port->currentIndex() < 0)
        return;
    QVariantList selectedItem = this->ui->port->currentData().toList();
    QString type = selectedItem.at(0).toString();
    int index = selectedItem.at(1).toInt();

    if (type == "CUSTOM") {
        this->removeCustomPort(index);
    }
}

void MainWindow::on_port_currentIndexChanged(int index)
{
    if (index < 0)
        return;

    this->update();
}
