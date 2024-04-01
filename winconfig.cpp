#include "winconfig.h"
#include "ui_winconfig.h"

winConfig::winConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::winConfig)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onCancel()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonBox_clicked(QAbstractButton*)));

    QComboBox *cbTmp;
    QLabel *lTmp;
    QDoubleSpinBox *dsbTmp;
    for(int i=0; i<16; i++) {
        // input label
        lTmp = new QLabel(QString::number(i), ui->gbIns);
        lTmp->setGeometry(8, 36+24*i, 15, 22);
        lInput[i] = lTmp;

        // input type
        cbTmp = new QComboBox(ui->gbIns);
        cbTmp->setGeometry(30, 36+24*i, 69,22);
        cbTmp->setModel(&cbInputTypeModel);
        cbInputType[i] = cbTmp;

        // input delay
        dsbTmp = new QDoubleSpinBox(ui->gbIns);
        dsbTmp->setGeometry(100, 36+24*i, 51,22);
        dsbInputDelay[i] = dsbTmp;

        // output label
        lTmp = new QLabel(QString::number(i), ui->gbOuts);
        lTmp->setGeometry(8, 36+24*i, 15, 22);
        lOutput[i] = lTmp;

        // output type
        cbTmp = new QComboBox(ui->gbOuts);
        cbTmp->setGeometry(30, 36+24*i, 69,22);
        cbTmp->setModel(&cbOutputTypeModel);
        cbOutputType[i] = cbTmp;


    }

    ui->cbInputTypeSet->setModel(&cbInputTypeModel);
}

winConfig::~winConfig()
{
    delete ui;
}

// SLOT
void winConfig::showConfig(TMtbModuleConfigGeneric *cfg)
{
    TMtbModuleConfigUNI *cfgu;
    TMtbModuleConfigUNIS *cfgs;
    QStandardItem *si;
    actcfg = cfg;
    for(int i = 0; i < 16; i++) {
        cbInputType[i]->hide();
        dsbInputDelay[i]->hide();
        cbInputType[i]->hide();
    }
    switch (cfg->mtbType) {
    case 0x10: // UNI
        //cfgu = (TMtbModuleConfigUNI *) cfg;

        //menu pro typ vstupu
        cbInputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::InputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniInputTypeGetNameFromIndex(i));
            cbInputTypeModel.appendRow(si);
        }

        // menu pro typ vystupu
        cbOutputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::OutputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(i));
            cbOutputTypeModel.appendRow(si);
        }

        for(int i = 0; i < 16; i++) {
            cbInputType[i]->show();
        }
        // no break !
    case 0x11 ... 0x16: // UNI
        cfgu = (TMtbModuleConfigUNI *) cfg;

        //menu pro typ vstupu
        cbInputTypeModel.clear();

        // menu pro typ vystupu
        cbOutputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::OutputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(i));
            cbOutputTypeModel.appendRow(si);
        }

        // vstup/výstupy
        for(int i = 0; i < 16; i++) {
            dsbInputDelay[i]->setValue(cfgu->inputsDelay[i]);
            dsbInputDelay[i]->show();
            cbInputType[i]->hide();
            cbOutputType[i]->show();
        }
        break;
    case 0x50: // UNIS
        cfgs = (TMtbModuleConfigUNIS *) cfg;

        //menu pro typ vstupu
        cbInputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::InputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniInputTypeGetNameFromIndex(i));
            cbInputTypeModel.appendRow(si);
        }

        // menu pro typ vystupu
        cbOutputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::OutputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(i));
            cbOutputTypeModel.appendRow(si);
        }

        // vstup/výstupy
        for(int i = 0; i < 16; i++) {
            cbInputType[i]->hide();
            dsbInputDelay[i]->show();
            dsbInputDelay[i]->setValue(cfgs->inputsDelay[i]);

            int pom = TMtbModuleTypes::ModuleUniOutputTypeToIndex(cfgs->outputsSafe[i].type);
            cbOutputType[i]->setCurrentIndex(pom);
            cbOutputType[i]->show();
        }
    }
    this->show();
}

void winConfig::onCancel(void)
{
    //this->hide();
}

void winConfig::onAccept(void)
{
    //this->hide();
    TMtbModuleConfigUNI *cfgu;
    TMtbModuleConfigUNIS *cfgs;
    switch (actcfg->mtbType) {
    case 0x10: // UNI
        cfgu = (TMtbModuleConfigUNI *) actcfg;
        break;
    case 0x50: // UNIS
        cfgs = (TMtbModuleConfigUNIS *) actcfg;

        // vstup/výstupy
        for(int i = 0; i < 16; i++) {
            cfgs->inputsDelay[i] = dsbInputDelay[i]->value();
            int pom = TMtbModuleTypes::ModuleUniOutputIndexToType(cbOutputType[i]->currentIndex());
            cfgs->outputsSafe[i].type = pom;
        }
    }
}

void winConfig::onButtonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::ResetRole) {
        cbOutputType[0]->hide();
    }
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole) {
        cbOutputType[1]->hide();
    }
}

