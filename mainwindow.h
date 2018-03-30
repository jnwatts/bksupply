#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

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
    void bk1696_changed(void);
    void timeout(void);
    void on_open_clicked();
    void on_close_clicked();

    void on_on_clicked();

    void on_off_clicked();

    void on_clear_clicked();

private:
    Ui::MainWindow *ui;
    QTimer _timer;
};

#endif // MAINWINDOW_H
