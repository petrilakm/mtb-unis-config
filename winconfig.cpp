#include "winconfig.h"
#include "ui_winconfig.h"

winConfig::winConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::winConfig)
{
    ui->setupUi(this);


    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onCancel()));

    QStandardItem *si;

    si = new QStandardItem("Obyč");
    cbInputTypeModel.appendRow(si);
    si = new QStandardItem("IR");
    cbInputTypeModel.appendRow(si);

    QComboBox *cbTmp;
    QLabel *lTmp;
    QDoubleSpinBox *dsbTmp;
    for(int i=0; i<16; i++) {
        // input type
        cbTmp = new QComboBox(ui->gbIns);
        cbTmp->setGeometry(30, 30+24*i, 69,22);
        cbTmp->setModel(&cbInputTypeModel);

        // input delay
        dsbTmp = new QDoubleSpinBox(ui->gbIns);
        dsbTmp->setGeometry(100, 30+24*i, 51,22);

        lTmp = new QLabel(QString::number(i), ui->gbIns);
        lTmp->setGeometry(8, 30+24*i, 15, 22);

        // save to array
        cbInputType[i] = cbTmp;
        dsbInputDelay[i] = dsbTmp;
    }

    ui->cbInputTypeSet->setModel(&cbInputTypeModel);
}

winConfig::~winConfig()
{
    delete ui;
}

void winConfig::onReset(void)
{
    //
}

void winConfig::onCancel(void)
{
    this->hide();
}
