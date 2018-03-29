#include <QDebug>
#include <iostream>
#include <QDateTime>
#include <QApplication>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bk1696.h"

static BK1696 bk;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _timer(this)
{
    ui->setupUi(this);

    connect(&bk, SIGNAL(response(QString)), this, SLOT(response(QString)));
    connect(&bk, SIGNAL(success()), this, SLOT(success()));
    connect(&this->_timer, SIGNAL(timeout()), this, SLOT(timeout()));

    foreach (const QString &port, BK1696::ports()) {
        this->ui->port->addItem(port);
    }
    this->ui->port->setCurrentIndex(0);

    _timer.setInterval(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::ostream *out = &std::cout;
    QString prefix;

    prefix = QDateTime::currentDateTime().toOffsetFromUtc(QDateTime::currentDateTime().offsetFromUtc()).toString(Qt::ISODate);
    prefix += " ";

    switch (type) {
    case QtDebugMsg:
        prefix += "[DBG] ";
        break;
    case QtWarningMsg:
        prefix += "[WARN] ";
        out = &std::cerr;
        break;
    case QtCriticalMsg:
        prefix += "[CRIT] ";
        out = &std::cerr;
        break;
    case QtFatalMsg:
        prefix += "[FATAL] ";
        out = &std::cerr;
        break;
    case QtInfoMsg:
        prefix += "[INFO] ";
        break;
    }

    if (context.category) {
        QString category = context.category;
        prefix.append(category);
        prefix.append(": ");
    }

    *out << prefix.toStdString() << msg.toStdString() << std::endl;
    this->ui->log->appendPlainText(prefix + msg);
}



void MainWindow::response(QString data)
{
//    qInfo(">> %s", qPrintable(data));

    auto parseGPAL = [this](QString gpal_data) {
        QStringList parts;

        struct offset_t {
            int first;
            int last;
            bool binary_decode;
        };

        static const auto binary_decode = [](QString data) -> QString {
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

        static const QList<offset_t> offsets = {
            {1,8, true},
            {9,9, false},
            {10,17, true},
            {18,18, false},
            {19,26, true},
            {27,27, false},
            {28,31, true},
            {32,35, true},
            {36,36, false},
            {37,37, false},
            {38,38, false},
            {39,39, false},
            {40,45, true},
            {46,46, false},
            {47,47, false},
            {48,48, false},
            {49,54, true},
            {55,55, false},
            {56,56, false},
            {57,57, false},
            {58,59, true},
            {60,60, false},
            {61,61, false},
            {62,62, false},
            {63,63, false},
            {64,64, false},
            {65,65, false},
            {66,66, false},
            {67,67, false},
            {68,68, false},
        };

        for (const offset_t &o : offsets) {
            QString p = gpal_data.mid(o.first - 1, o.last - o.first + 1);
            if (o.binary_decode)
                p = binary_decode(p);
            parts << p;
        }


        enum {
            READING_VOLTAGE = 0,
            READING_CURRENT = 2,
            READING_POWER = 4,
            SETTING_VOLTAGE = 12,
            SETTING_CURRENT = 16,
            OUTPUT_ON = 27,
            OUTPUT_OFF = 28,
        };

        this->ui->lcdVolt->display(parts.at(READING_VOLTAGE).toDouble());
        this->ui->lcdAmp->display(parts.at(READING_CURRENT).toDouble());
        this->ui->lcdWatt->display(parts.at(READING_POWER).toDouble());
    };

    parseGPAL(data);
}

void MainWindow::success()
{
//    qInfo("SUCCESS");
    if (bk.isOpen() && !this->_timer.isActive())
        this->_timer.start();
}

void MainWindow::timeout()
{
    if (bk.isOpen())
        bk.command("GPAL");
}

void MainWindow::on_open_clicked()
{
    bk.open(this->ui->port->currentText());
}

void MainWindow::on_close_clicked()
{
    bk.close();
}

void MainWindow::on_on_clicked()
{
    this->_timer.stop();
    bk.outputEnable(true);
}

void MainWindow::on_off_clicked()
{
    this->_timer.stop();
    bk.outputEnable(false);
}

void MainWindow::on_clear_clicked()
{
    this->ui->log->clear();
}
