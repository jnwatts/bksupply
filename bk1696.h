#ifndef BK1696_H
#define BK1696_H

#include <QObject>
#include "bkserial.h"

class BK1696 : public BKSerial
{
    Q_OBJECT
public:
    BK1696(QObject *parent = 0);
    ~BK1696(void);

    void outputEnable(bool enable, completed_handler_t complete = nullptr);
    void update(completed_handler_t complete = nullptr);

    bool enabled;
    double voltage;
    double current;
    double power;

signals:
    void enabledChanged(void);
    void voltageChanged(void);
    void currentChanged(void);
    void powerChanged(void);
};

#endif // BK1696_H
