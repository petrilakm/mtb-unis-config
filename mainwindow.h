#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QSocketNotifier>
#include "tcpsocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define SERVO_MIN (0)
#define SERVO_MAX (255)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    int servo_pos_act;
    int servo_pos_last;
    QTimer *timer_update_view;
    QTimer *timer_autoclick;
    tcpsocket *socket;

private:
    Ui::MainWindow *ui;
    QLabel *io_label[28];
    QPushButton *io_obutton[28];
    QPushButton *io_ibutton[16];
    int outputs[28];
    int inputs[16];
    void initIOstatus(void);
    void sendServoPos(void);
    void countButtonPos(void);

public slots:
    void timer_tick();
    void timer_autoclick_tick();
private slots:
    void on_pb_set_stop_clicked();
    void on_pb_connect_clicked();
    void on_pb_disconnect_clicked();
    void on_pb_outs_clicked();
    void onSocketConnect();
    void onSocketDisconnect();
    void on_getModuleStateOut(QJsonObject json);
    void on_getModuleStateIn(QJsonObject json);
    void on_pb_servoposset_clicked();
    void on_pb_servoposset2_clicked();
    void on_plusminus_clicked();

};
#endif // MAINWINDOW_H
