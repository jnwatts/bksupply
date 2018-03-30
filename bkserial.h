#ifndef BKSERIAL_H
#define BKSERIAL_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <functional>

class BKSerial : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void(QString)> response_handler_t;

    typedef struct {
        response_handler_t response_handler;
    } request_t;

    explicit BKSerial(QObject *parent = 0);
    ~BKSerial(void);

    void open(QString port);
    void close(void);
    bool isOpen(void) { return this->_serial.isOpen(); }

    void setAddress(int address) { this->_address = address; }
    int address(void) { return this->_address; }

    void command(QString command, response_handler_t response_handler = nullptr);
    void command(QString command, QList<QString> &args, response_handler_t response_handler = nullptr);

    static QList<QString> ports();

public slots:

private slots:
    void dataReady(void);
    void timeout(void);

private:
    int _address;
    QSerialPort _serial;
    QByteArray _response;
    QTimer _timeout;
    QList<request_t> _requests;
};

#endif // BKSERIAL_H
