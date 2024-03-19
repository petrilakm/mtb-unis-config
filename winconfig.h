#ifndef WINCONFIG_H
#define WINCONFIG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QStandardItemModel>
#include <QDoubleSpinBox>
#include <QList>

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
    QLabel* lInput[16];
    QLabel* lOutput[16];
    QComboBox* cbInputType[16];
    QDoubleSpinBox* dsbInputDelay[16];

private:
    Ui::winConfig *ui;
private slots:
    void onCancel(void);
    void onReset(void);
};

#endif // WINCONFIG_H
