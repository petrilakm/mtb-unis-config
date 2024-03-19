#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QList>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QSocketNotifier>
#include <QModelIndexList>
#include <QStandardItemModel>
#include "tcpsocket.h"
#include "configfile.h"
#include "winconfig.h"
#include "MtbModules.h"
#include <QDebug>

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
    configfile cfgfile;
    bool fileMode;
    void changeLayoutType(int type);

private:
    Ui::MainWindow *ui;
    winConfig *winConfig;
    QLabel *io_label[28];
    QPushButton *io_obutton[28];
    QPushButton *io_ibutton[16];
    int outputs[28];
    int inputs[16];
    TMtbModuleTypes MtbModuleTypes;
    QStandardItemModel moduleModel;
    QList<QString> moduleList;
    int moduleSelected;
    void refreshModuleList(void);
    void refreshModuleListModel(void);

    void initIOstatus(void);
    void sendServoPos(void);
    void countButtonPos(void);
    void on_lvModule_changed(const int linenum);

public slots:
    void timer_tick();
    void timer_autoclick_tick();
private slots:
    void on_pb_set_stop_clicked();
    void on_pb_connect_clicked();
    void on_pb_disconnect_clicked();
    void pbOutsClicked();
    void onSocketConnect();
    void onSocketDisconnect();
    void on_getModuleStateOut(QJsonObject json);
    void on_getModuleStateIn(QJsonObject json);
    void on_responseModuleList();
    void on_responseModuleInfo();
    void on_pb_servoposset_clicked();
    void on_pb_servoposset2_clicked();
    void pbPlusminusClicked();
    void on_pbReboot_clicked();
    void on_pb_reload_clicked();
    void on_pb_browsefw_clicked();
    void on_pb_fw_upgrade_clicked();

    void on_pbSettings_clicked();
    void on_lvModuleList_activated(const QModelIndex &index);
    void on_lvModuleList_clicked(const QModelIndex &index);
    void on_pbLoadOffline_clicked();
    void on_pbSave_clicked();
    void on_pbModuleRemove_clicked();
    void on_pbModuleAdd_clicked();
};
#endif // MAINWINDOW_H
