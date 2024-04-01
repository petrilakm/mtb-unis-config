#ifndef WINCONFIG_H
#define WINCONFIG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QStandardItemModel>
#include <QDoubleSpinBox>
#include <QAbstractButton>
#include <QList>
#include "MtbModules.h"

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
    QDoubleSpinBox* dsbInputDelay[16];
    void showConfig(TMtbModuleConfigGeneric *cfg);

private:
    Ui::winConfig *ui;
    TMtbModuleConfigGeneric *actcfg;
private slots:
    void onCancel(void);
    void onAccept(void);
    void onButtonBox_clicked(QAbstractButton *button);
};

#endif // WINCONFIG_H
