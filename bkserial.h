#ifndef BKSERIAL_H
#define BKSERIAL_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>

class BKSerial : public QObject
{
    Q_OBJECT
public:
    explicit BKSerial(QObject *parent = 0);
    ~BKSerial(void);

    void open(QString port);
    void close(void);
    bool isOpen(void) { return this->_serial.isOpen(); }

    void setAddress(int address) { this->_address = address; }
    int address(void) { return this->_address; }

    void command(QString command);
    void command(QString command, QList<QString> &args);

    void startSession();
    void endSession(void);

    static QList<QString> ports();

signals:
    void response(QString data);
    void success(void);

public slots:

private slots:
    void dataReady(void);
    void timeout(void);

private:
    int _address;
    QSerialPort _serial;
    QByteArray _response;
    QTimer _timeout;
};

#endif // BKSERIAL_H
