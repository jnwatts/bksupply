#include <QApplication>
#include <QSerialPortInfo>
#include <QByteArray>
#include <memory>
#include "bkserial.h"

BKSerial::BKSerial(QObject *parent) : QObject(parent)
{
    connect(&this->_serial, SIGNAL(readyRead()), this, SLOT(dataReady()));
    connect(&this->_timeout, SIGNAL(timeout()), this, SLOT(timeout()));
    this->_address = 0;
    this->_serial.setBaudRate(9600);  // baud
    this->_timeout.setInterval(1000); // ms
    this->_timeout.setSingleShot(true);
}

BKSerial::~BKSerial()
{
    if (this->isOpen()) {
        this->_serial.close();
    }
}

void BKSerial::open(QString port)
{
    if (!this->isOpen()) {
        this->_serial.setPortName(port);
        this->_serial.open(QSerialPort::ReadWrite);
        this->startSession();
    }
}

void BKSerial::close(void)
{
    if (this->isOpen()) {
        auto conn = std::make_shared<QMetaObject::Connection>();
        *conn = connect(this, &BKSerial::success, [this, conn]() { this->_serial.close(); QObject::disconnect(*conn); });
        this->endSession();
    }
}

void BKSerial::command(QString command)
{
    QList<QString> args;
    this->command(command, args);
}

void BKSerial::command(QString command, QList<QString> &args)
{
    while (this->_timeout.isActive())
        qApp->processEvents();

    if (!this->isOpen()) {
        emit this->response("ERR: DEVICE NOT OPEN");
        return;
    }

    QByteArray data;
    data.append(command);
    data.append(QString().sprintf("%02d", this->_address));
    foreach (const QString &arg, args) {
        data.append(arg);
    }
    data.append('\r');

    this->_timeout.start();
    this->_serial.write(data);
}

void BKSerial::startSession()
{
    this->command("SESS");
}

void BKSerial::endSession()
{
    this->command("ENDS");
}

QList<QString> BKSerial::ports()
{
    QList<QString> result;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        result << info.portName();
    }

    return result;
}

void BKSerial::dataReady(void)
{
    QByteArray data = this->_serial.readAll();
    this->_response.append(data);
    while (this->_response.size() && this->_response.contains('\r')) {
        int pos = this->_response.indexOf('\r');

        data = this->_response.left(pos);
        if (data == "OK")
            emit this->success();
        else
            emit this->response(data);

        this->_response.remove(0, pos + 1);
        if (this->_response.size() == 0)
            this->_timeout.stop();
    }
}

void BKSerial::timeout()
{
    emit this->response("ERR: TIMEOUT");
}
