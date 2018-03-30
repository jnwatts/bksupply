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
        this->command("SESS");
    }
}

void BKSerial::close(void)
{
    if (this->isOpen()) {
        this->command("ENDS", [this](QString data) {
            Q_UNUSED(data);
            this->_serial.close();
        });
    }
}

void BKSerial::command(QString command, response_handler_t response_handler)
{
    QList<QString> args;
    this->command(command, args, response_handler);
}

void BKSerial::command(QString command, QList<QString> &args, response_handler_t response_handler)
{
    if (!this->isOpen()) {
        if (response_handler)
            response_handler("ERR: DEVICE NOT OPEN");
        return;
    }

    QByteArray data;
    data.append(command);
    data.append(QString().sprintf("%02d", this->_address));
    foreach (const QString &arg, args) {
        data.append(arg);
    }
    data.append('\r');

    request_t request = {response_handler};
    this->_requests << request;
    this->_serial.write(data);
    this->_timeout.start();
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
    this->_timeout.stop();
    this->_response.append(data);
    while (this->_response.size() && this->_response.contains('\r')) {
        int pos = this->_response.indexOf('\r');
        if (this->_requests.size() > 0) {
            request_t &request = this->_requests.first();

            data = this->_response.left(pos);
            if (request.response_handler)
                request.response_handler(data);
            if (data == "OK")
                this->_requests.removeFirst();
        }
        this->_response.remove(0, pos + 1);

    }
    if (this->_requests.size() > 0)
        this->_timeout.start();
}

void BKSerial::timeout()
{
    for (request_t &request : this->_requests) {
        if (request.response_handler)
            request.response_handler("ERR: TIMEOUT");
    }

    this->_requests.clear();
}
