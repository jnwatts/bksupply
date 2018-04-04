#ifndef BK1696_H
#define BK1696_H

#include <QObject>
#include "bkserial.h"

class BK1696 : public BKSerial
{
    Q_OBJECT
public:
    typedef enum {
        CV_MODE = 0,
        CC_MODE,
    } power_mode_t;

    BK1696(QObject *parent = 0);
    ~BK1696(void);

    void outputEnable(bool enable, completed_handler_t complete = nullptr);
    void update(completed_handler_t complete = nullptr);

    bool enabled;
    double voltage;
    double current;
    double power;
    power_mode_t power_mode;

signals:
    void enabledChanged(void);
    void voltageChanged(void);
    void currentChanged(void);
    void powerChanged(void);
    void powerModeChanged(void);

private:
    void getd(completed_handler_t complete = nullptr);
    void gpal(completed_handler_t complete = nullptr);
};

#endif // BK1696_H
