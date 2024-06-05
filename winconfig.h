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

#include <QtLogging>

namespace Ui {
class winConfig;
}

class winConfig : public QDialog
{
    Q_OBJECT

public:
    explicit winConfig(QWidget *parent = nullptr);
    ~winConfig();

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
    void showConfig(TMtbModuleConfigGeneric *cfg, int _moduleType);

private:
    Ui::winConfig *ui;
    TMtbModuleConfigGeneric *actcfg;
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
};

#endif // WINCONFIG_H
