#ifndef BKSERIAL_H
#define BKSERIAL_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QVector>
#include <functional>
#include <QMap>

class BKSerial : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void(QStringList)> response_handler_t;
    typedef std::function<void(void)> completed_handler_t;
    typedef std::function<QString(QString)> transform_t;

    typedef struct {
        int first;
        int last;
        transform_t transform;
    } response_part_t;

    typedef struct {
        int size;
        QVector<response_part_t> parts;
    } response_t;

    typedef struct {
        QString command;
        response_t *response;
        response_handler_t response_handler;
    } request_t;

    explicit BKSerial(QObject *parent = 0);
    ~BKSerial(void);

    void open(QString port, completed_handler_t complete = nullptr);
    void close(completed_handler_t complete);
    bool isOpen(void) { return this->_serial.isOpen(); }

    void setAddress(int address) { this->_address = address; }
    int address(void) { return this->_address; }

    void command(QString command, response_handler_t response_handler = nullptr);
    void command(QString command, QList<QString> &args, response_handler_t response_handler = nullptr);

    static QList<QString> ports();

protected:
    void registerResponse(QString command, const response_t &response);
    void failure(request_t &request, QString reason);
    void parseResponse(request_t &request, QString data);

signals:
    void openChanged(void);

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
    QMap<QString, response_t> _responses;
    int _timeout_count;
};

#endif // BKSERIAL_H
