#include "bk1696.h"

typedef std::function<QString(QString)> transform_t;

static QString binaryDecode(QString data);

typedef struct {
    int first;
    int last;
    transform_t transform;
} gpal_offset_t;

enum {
    READING_VOLTAGE = 0,
    READING_CURRENT = 2,
    READING_POWER = 4,
    SETTING_VOLTAGE = 12,
    SETTING_CURRENT = 16,
    OUTPUT_ON = 27,
    OUTPUT_OFF = 28,
};

static const QList<gpal_offset_t> gpal_offsets = {
    {1,  8,  &binaryDecode},
    {9,  0,  nullptr},
    {10, 17, &binaryDecode},
    {18, 0,  nullptr},
    {19, 26, &binaryDecode},
    {27, 0,  nullptr},
    {28, 31, &binaryDecode},
    {32, 35, &binaryDecode},
    {36, 0,  nullptr},
    {37, 0,  nullptr},
    {38, 0,  nullptr},
    {39, 0,  nullptr},
    {40, 45, &binaryDecode},
    {46, 0,  nullptr},
    {47, 0,  nullptr},
    {48, 0,  nullptr},
    {49, 54, &binaryDecode},
    {55, 0,  nullptr},
    {56, 0,  nullptr},
    {57, 0,  nullptr},
    {58, 59, &binaryDecode},
    {60, 0,  nullptr},
    {61, 0,  nullptr},
    {62, 0,  nullptr},
    {63, 0,  nullptr},
    {64, 0,  nullptr},
    {65, 0,  nullptr},
    {66, 0,  nullptr},
    {67, 0,  nullptr},
    {68, 0,  nullptr},
};
const int gpal_length = 68;

BK1696::BK1696(QObject *parent) : BKSerial(parent),
    enabled(false),
    voltage(0.0),
    current(0.0),
    power(0.0)
{
}

BK1696::~BK1696()
{
}

void BK1696::outputEnable(bool enable)
{
    QList<QString> args;
    args << (enable ? "0" : "1");
    this->command("SOUT", args, [this, enable](QString data) {
        if (data == "OK") {
            this->enabled = enable;
            emit this->enabledChanged();
        }
    });
}

void BK1696::update(void)
{
    this->command("GPAL", [this](QString data) {
        QStringList parts;

        if (data.size() != gpal_length)
            return;

        for (const gpal_offset_t &o : gpal_offsets) {
            QString p = data.mid(o.first - 1, (o.last ? o.last : o.first) - o.first + 1);
            if (o.transform)
                p = o.transform(p);
            parts << p;
        }

        this->voltage = parts.at(READING_VOLTAGE).toDouble();
        this->current = parts.at(READING_CURRENT).toDouble();
        this->power = parts.at(READING_POWER).toDouble();

        emit this->voltageChanged();
        emit this->currentChanged();
        emit this->powerChanged();
    });
}

static QString binaryDecode(QString data)
{
    QString p = "";
    uint32_t v = 0;
    for (QChar c : data) {
        v <<= 4;
        v |= (c.toLatin1() - '0');
    }
    for (int i = 0; i < 4; i++) {
        uint32_t t = (v >> (3-i)*8) & 0xff;
        switch (t & 0x7F) {
        case 0b0000000: p += ' '; break;
        case 0b0111111: p += '0'; break;
        case 0b0000110: p += '1'; break;
        case 0b1011011: p += '2'; break;
        case 0b1001111: p += '3'; break;
        case 0b1100110: p += '4'; break;
        case 0b1101101: p += '5'; break;
        case 0b1111101: p += '6'; break;
        case 0b0000111: p += '7'; break;
        case 0b1111111: p += '8'; break;
        case 0b1101111: p += '9'; break;
        default: p += '\''; p += QString::number(t&0x7f, 16); p+='\''; break;
        }
        if (t & 0x80)
            p += ".";
    }
    return p;
};
