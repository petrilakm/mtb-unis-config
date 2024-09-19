#ifndef WINCONFIG_H
#define WINCONFIG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QStandardItemModel>
#include <QDoubleSpinBox>
#include <QAbstractButton>
#include <QCheckBox>
#include <QList>
#include "MtbModules.h"

#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class WinConfig; }
QT_END_NAMESPACE

class WinConfig : public QDialog
{
    Q_OBJECT

public:
    explicit WinConfig(QWidget *parent = nullptr);
    ~WinConfig();

    QStandardItemModel cbInputTypeModel;
    QStandardItemModel cbOutputTypeModel;
    QLabel* lInput[16];
    QLabel* lOutput[16];
    QComboBox* cbInputType[16];
    QComboBox* cbOutputType[16];
    QDoubleSpinBox* dsbOutputSafe[16];
    QDoubleSpinBox* dsbInputDelay[16];
    QLabel* lServo[6];
    QCheckBox* chServoEnable[6];
    QDoubleSpinBox* dsbServoPosA[6];
    QDoubleSpinBox* dsbServoPosB[6];
    QDoubleSpinBox* dsbServoSpeed[6];
    QLabel* lInputType;
    QLabel* lInputDelay;
    QLabel* lOutputType;
    QLabel* lOutputSafe;
    void showConfig(int _moduleAddress, TMtbModuleConfigGeneric *cfg, int _moduleType);

private:
    Ui::WinConfig *ui;
    TMtbModuleConfigGeneric *actcfg;
    int moduleAddress;
    int moduleType;
    void hideAll();
    void showInputType(QList<int> vals);
    void showInputDelay(QList<float> vals);
    void showOutputType(QList<int> vals);
    void showOutputSafe(QList<int> vals);
    void showServos(TMtbModuleConfigUNIS *cfg);

private slots:
    void onCancel(void);
    void onAccept(void);
    void onButtonBox_clicked(QAbstractButton *button);
    void on_pbInputTypeSet_clicked();
    void on_pbInputDelaySet_clicked();
    void on_pbOutputTypeSet_clicked();
    void on_pbOutputSafeSet_clicked();
    void on_chServoEnable();

signals:
    void changeConfig(int _moduleNum, TMtbModuleConfigGeneric *cfg, int _moduleType);
};

#endif // WINCONFIG_H
