#ifndef BK1696_H
#define BK1696_H

#include <QObject>
#include "bkserial.h"

class BK1696 : public BKSerial
{
public:
    BK1696(QObject *parent = 0);
    ~BK1696(void);

    void outputEnable(bool enable);
};

#endif // BK1696_H
