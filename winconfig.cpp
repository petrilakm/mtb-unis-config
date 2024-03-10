#include "winconfig.h"
#include "ui_winconfig.h"

winConfig::winConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::winConfig)
{
    ui->setupUi(this);


    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onCancel()));
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
