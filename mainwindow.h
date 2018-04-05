#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSettings>
#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private slots:
    void on_open_clicked();
    void on_close_clicked();
    void on_on_clicked();
    void on_off_clicked();
    void on_clear_clicked();
    void on_addPort_clicked();
    void on_delPort_clicked();
    void on_port_currentIndexChanged(int index);

private:
    void display(QLabel *label, double value, int precision);
    void update(void);
    void updatePorts(void);
    void addCustomPort(QString port);
    void removeCustomPort(int index);
    bool isSelectedPortCustom(void);
    void saveSettings(void);
    void loadSettings(void);
    QSettings &settings(void) { return this->_settings; }

    Ui::MainWindow *ui;
    QSettings _settings;
    QTimer _timer;
    QVector<QString> _customPorts;
};

#endif // MAINWINDOW_H
