#include "bk1696.h"

BK1696::BK1696(QObject *parent) : BKSerial(parent)
{
}

BK1696::~BK1696()
{
}

void BK1696::outputEnable(bool enable)
{
    QList<QString> args;
    args << (enable ? "0" : "1");
    this->command("SOUT", args);
}

