#include <QApplication>
#include <QSerialPortInfo>
#include <QByteArray>
#include <memory>
#include "bkserial.h"

const int MAX_TIMEOUTS = 3;

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

void BKSerial::open(QString port, completed_handler_t complete)
{
    if (!this->isOpen()) {
        this->_timeout_count = 0;
        this->_serial.setPortName(port);
        this->_serial.open(QSerialPort::ReadWrite);
        this->command("SESS", [this, complete](QStringList data) {
            Q_UNUSED(data);
            emit this->openChanged();
            if (complete)
                complete();
        });
    }
}

void BKSerial::close(completed_handler_t complete)
{
    if (this->isOpen()) {
        this->command("ENDS", [this, complete](QStringList data) {
            Q_UNUSED(data);

            this->_serial.close();

            emit this->openChanged();

            if (complete)
                complete();
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
    request_t request = {command, nullptr, response_handler};
    auto iter = this->_responses.find(command);
    if (iter != this->_responses.end())
        request.response = &(iter.value());

    if (!this->isOpen()) {
        this->failure(request, "ERR: DEVICE NOT OPEN");
        return;
    }

    QByteArray data;
    data.append('\r');
    data.append(command);
    data.append(QString().sprintf("%02d", this->_address));
    foreach (const QString &arg, args) {
        data.append(arg);
    }
    data.append('\r');

    if (this->_requests.size() == 0)
        this->_timeout.start();

    this->_requests << request;
    this->_serial.write(data);
}

QList<QString> BKSerial::ports()
{
    QList<QString> result;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        result << info.portName();
    }

    return result;
}

void BKSerial::registerResponse(QString command, const response_t &response)
{
    this->_responses[command] = response;
}

void BKSerial::failure(request_t &request, QString reason)
{
    request.response_handler({reason});
}

void BKSerial::parseResponse(request_t &request, QString data)
{
    Q_ASSERT(request.response != nullptr);
    response_t &response = *request.response;

    QStringList parts;

    if (response.size == data.size()) {
        for (auto &part : response.parts) {
            QString part_str = data.mid(part.first - 1, (part.last ? part.last : part.first) - part.first + 1);
            if (part.transform)
                part_str = part.transform(part_str);
            parts << part_str;
        }
    } else {
        parts << data;
    }

    request.response_handler(parts);
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

            if (request.response_handler) {
                if (data == "OK" || request.response == nullptr)
                    request.response_handler({data});
                else
                    this->parseResponse(request, data);
            }
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
    this->_timeout_count++;
    if (this->_timeout_count > MAX_TIMEOUTS) {
        qWarning("Too many timeouts, closing port");
        this->_serial.close();
        emit this->openChanged();

        for (request_t &request : this->_requests) {
            this->failure(request, "ERR: TIMEOUT");
        }
        this->_requests.clear();
    } else {
        if (this->_requests.size() > 0) {
            request_t &request = this->_requests.first();
            this->failure(request, "ERR: TIMEOUT");
            this->_requests.pop_front();
        }
        if (this->_requests.size() > 0)
            this->_timeout.start();
    }
}
