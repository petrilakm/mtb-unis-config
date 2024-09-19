#include "winconfig.h"
#include "ui_winconfig.h"

WinConfig::WinConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WinConfig)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(onCancel()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonBox_clicked(QAbstractButton*)));

    QComboBox *cbTmp;
    QLabel *lTmp;
    QDoubleSpinBox *dsbTmp;
    QCheckBox *chTmp;

    lTmp = new QLabel(tr("typ"), ui->gbIns);
    lTmp->setGeometry(30, 12, 60, 22);
    lInputType = lTmp;

    lTmp = new QLabel(tr("zpoždění"), ui->gbIns);
    lTmp->setGeometry(100, 12, 60, 22);
    lInputDelay = lTmp;

    lTmp = new QLabel(tr("typ"), ui->gbOuts);
    lTmp->setGeometry(30, 12, 60, 22);
    lOutputType = lTmp;

    lTmp = new QLabel(tr("stav bezpečný"), ui->gbOuts);
    lTmp->setGeometry(100, 12, 80, 22);
    lOutputSafe = lTmp;

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
        dsbTmp->setSingleStep(0.1);
        dsbTmp->setDecimals(1);
        dsbTmp->setMinimum(0);
        dsbTmp->setMaximum(9);
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

        // output safe
        dsbTmp = new QDoubleSpinBox(ui->gbOuts);
        dsbTmp->setGeometry(100, 36+24*i, 51,22);
        dsbTmp->setSingleStep(1);
        dsbTmp->setDecimals(0);
        dsbTmp->setMinimum(0);
        dsbTmp->setMaximum(255);
        dsbOutputSafe[i] = dsbTmp;
    }

    // nastavení pro serva
    for(int i=0; i<6; i++) {
        // servo label
        lTmp = new QLabel(QString("Servo %1").arg(i+1), ui->gbServos);
        lTmp->setGeometry(8, 12+24*((i*3)+0), 60, 22);
        lServo[i] = lTmp;

        chTmp = new QCheckBox(tr("aktivní"), ui->gbServos);
        chTmp->setGeometry(8+90, 12+24*((i*3)+1), 60, 22);
        connect(chTmp, SIGNAL(toggled(bool)), this, SLOT(on_chServoEnable()));
        chServoEnable[i] = chTmp;

        dsbTmp = new QDoubleSpinBox(ui->gbServos);
        dsbTmp->setGeometry(8, 12+24*((i*3)+1), 61,22);
        dsbTmp->setSingleStep(1);
        dsbTmp->setDecimals(0);
        dsbTmp->setMinimum(0);
        dsbTmp->setMaximum((1 << 16)-1);
        dsbTmp->setToolTip(tr("poloha A"));
        dsbServoPosA[i] = dsbTmp;

        dsbTmp = new QDoubleSpinBox(ui->gbServos);
        dsbTmp->setGeometry(8, 12+24*((i*3)+2), 61,22);
        dsbTmp->setSingleStep(1);
        dsbTmp->setDecimals(0);
        dsbTmp->setMinimum(0);
        dsbTmp->setMaximum((1 << 16)-1);
        dsbTmp->setToolTip(tr("poloha B"));
        dsbServoPosB[i] = dsbTmp;

        dsbTmp = new QDoubleSpinBox(ui->gbServos);
        dsbTmp->setGeometry(8+90, 12+24*((i*3)+2), 51,22);
        dsbTmp->setSingleStep(1);
        dsbTmp->setDecimals(0);
        dsbTmp->setMinimum(0);
        dsbTmp->setMaximum((1 << 8)-1);
        dsbTmp->setToolTip(tr("rychlost"));
        dsbServoSpeed[i] = dsbTmp;
    }
    ui->cbInputTypeSet->setModel(&cbInputTypeModel);
    ui->cbOutputTypeSet->setModel(&cbOutputTypeModel);
}

WinConfig::~WinConfig()
{
    delete ui;
}

// SLOT
void WinConfig::showConfig(int _moduleAddress, TMtbModuleConfigGeneric *cfg, int _moduleType)
{
    TMtbModuleConfigUNI *cfgu;
    TMtbModuleConfigUNIS *cfgs;
    QStandardItem *si;
    int pom;
    float pomf;
    QList<int> lInt;
    QList<float> lFloat;

    actcfg = cfg;
    moduleAddress = _moduleAddress;
    moduleType = _moduleType;

    hideAll();

    //menu pro typ vstupu - prázdné pro většinu desek
    cbInputTypeModel.clear();
    for(int i = 0; i < 16; i++) {
        cbInputType[i]->setCurrentIndex(0);
    }

    switch (moduleType) {
    case 0x10: // UNI with IR
        cfgu = (TMtbModuleConfigUNI *) cfg;

        // menu pro typ vstupu - vyplnit volby pro staré UNI
        cbInputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::InputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniInputTypeGetNameFromIndex(i));
            cbInputTypeModel.appendRow(si);
        }

        // menu pro typ vstupu - zobrazit
        lInt.clear();
        for(int i = 0; i < 16; i++) {
            pom = TMtbModuleTypes::ModuleUniInputTypeToIndex(cfgu->inputsType[i]);
            lInt.append(pom);
        }
        showInputType(lInt);

        // no break !
        [[fallthrough]];
    case 0x11 ... 0x16: // UNI (no IR)
        cfgu = (TMtbModuleConfigUNI *) cfg;

        // menu pro typ výstupu
        cbOutputTypeModel.clear();
        for(int i = 0; i < TMtbModuleTypes::OutputTypeCount; i++) {
            si = new QStandardItem(TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(i));
            cbOutputTypeModel.appendRow(si);
        }

        // vstup, zpoždění
        lFloat.clear();
        for(int i = 0; i < 16; i++) {
            pomf = (cfgu->inputsDelay[i] / 10.0);
            lFloat.append(pomf);
        }
        showInputDelay(lFloat);

        // výstup, typ
        lInt.clear();
        for(int i = 0; i < 16; i++) {
            pom = (cfgu->outputsSafe[i].type);
            lInt.append(pom);
        }
        showOutputType(lInt);

        // výstup, bezpečny stav
        lInt.clear();
        for(int i = 0; i < 16; i++) {
            pom = (cfgu->outputsSafe[i].value);
            lInt.append(pom);
        }
        showOutputSafe(lInt);
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
            //qDebug("type %d", i);
            si = new QStandardItem(TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(i));
            cbOutputTypeModel.appendRow(si);
        }

        // vstup, zpoždění
        lFloat.clear();
        for(int i = 0; i < 16; i++) {
            pomf = (cfgs->inputsDelay[i] / 10.0);
            lFloat.append(pomf);
        }
        showInputDelay(lFloat);

        // výstup, typ
        lInt.clear();
        for(int i = 0; i < 16; i++) {
            pom = (cfgs->outputsSafe[i].type);
            lInt.append(pom);
        }
        showOutputType(lInt);

        // výstup, bezpečny stav
        lInt.clear();
        for(int i = 0; i < 16; i++) {
            pom = (cfgs->outputsSafe[i].value);
            lInt.append(pom);
        }
        showOutputSafe(lInt);

        showServos(cfgs);
    }
    this->show();
}

void WinConfig::onCancel(void)
{
    //this->hide();
}

void WinConfig::onAccept(void)
{
    //this->hide();
    TMtbModuleConfigUNI *cfgu;
    TMtbModuleConfigUNIS *cfgs;
    int pom;
    switch (moduleType) {
    case 0x10: // UNI
        cfgu = (TMtbModuleConfigUNI *) actcfg;
        for(int i = 0; i < 16; i++) {
            cfgu->inputsType[i] = cbInputType[i]->currentIndex();
        }
        // no break !
        [[fallthrough]];
    case 0x11 ... 0x16: // UNI
        cfgu = (TMtbModuleConfigUNI *) actcfg;
        // vstup/výstupy
        for(int i = 0; i < 16; i++) {
            cfgu->inputsDelay[i] = (dsbInputDelay[i]->value() * 10.0);
            pom = TMtbModuleTypes::ModuleUniOutputIndexToType(cbOutputType[i]->currentIndex());
            cfgu->outputsSafe[i].type = pom;
            cfgu->outputsSafe[i].value = dsbOutputSafe[i]->value();
        }
        emit changeConfig(moduleAddress, cfgu, moduleType); // propaguje změny dále
        break;
    case 0x50: // UNIS
        cfgs = (TMtbModuleConfigUNIS *) actcfg;

        // vstup/výstupy
        for(int i = 0; i < 16; i++) {
            cfgs->inputsDelay[i] = (dsbInputDelay[i]->value() * 10.0);
            pom = TMtbModuleTypes::ModuleUniOutputIndexToType(cbOutputType[i]->currentIndex());
            cfgs->outputsSafe[i].type = pom;
            cfgs->outputsSafe[i].value = dsbOutputSafe[i]->value();
        }
        for(int i = 0; i < 6; i++) {
            cfgs->servoSpeed[i] = (dsbServoSpeed[i]->value());
            cfgs->servoPosition[i].posA = (dsbServoPosA[i]->value());
            cfgs->servoPosition[i].posB = (dsbServoPosB[i]->value());
            cfgs->servoEnabledMask = (cfgs->servoEnabledMask & ~(1<<i)) | ((chServoEnable[i]->isChecked()) ? (1 << i) : 0);

        }
        emit changeConfig(moduleAddress, cfgs, moduleType); // propaguje změny dále
    }
}

void WinConfig::onButtonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::ResetRole) {
        showConfig(moduleAddress, actcfg, moduleType);
    }
}

void WinConfig::on_pbInputTypeSet_clicked()
{
    int set = ui->cbInputTypeSet->currentIndex();
    for(int i = 0; i < 16; i++) {
        cbInputType[i]->setCurrentIndex(set);
    }
}

void WinConfig::on_pbInputDelaySet_clicked()
{
    float set = ui->dsbInputDelaySet->value();
    for(int i = 0; i < 16; i++) {
        dsbInputDelay[i]->setValue(set);
    }
}

void WinConfig::on_pbOutputTypeSet_clicked()
{
    int set = ui->cbOutputTypeSet->currentIndex();
    for(int i = 0; i < 16; i++) {
        cbOutputType[i]->setCurrentIndex(set);
    }
}

void WinConfig::on_pbOutputSafeSet_clicked()
{
    int set = ui->dsbOutputSafeSet->value();
    for(int i = 0; i < 16; i++) {
        dsbOutputSafe[i]->setValue(set);
    }
}

void WinConfig::hideAll()
{
    for(int i = 0; i < 16; i++) {
        cbInputType[i]->hide();
        dsbInputDelay[i]->hide();
        cbOutputType[i]->hide();
        dsbOutputSafe[i]->hide();
    }
    ui->cbInputTypeSet->hide();
    ui->pbInputTypeSet->hide();
    ui->dsbInputDelaySet->hide();
    ui->pbInputDelaySet->hide();
    ui->cbOutputTypeSet->hide();
    ui->pbOutputTypeSet->hide();
    ui->dsbOutputSafeSet->hide();
    ui->pbOutputSafeSet->hide();

    for(int i = 0; i < 6; i++) {
        chServoEnable[i]->hide();
        dsbServoPosA[i]->hide();
        dsbServoPosB[i]->hide();
        dsbServoSpeed[i]->hide();
        lServo[i]->hide();
    }
}

void WinConfig::showInputType(QList<int> vals)
{
    for(int i = 0; i < vals.count(); i++) {
        cbInputType[i]->setCurrentIndex(vals.at(i));
        cbInputType[i]->show();
    }
    ui->cbInputTypeSet->show();
    ui->pbInputTypeSet->show();
}

void WinConfig::showInputDelay(QList<float> vals)
{
    for(int i = 0; i < vals.count(); i++) {
        dsbInputDelay[i]->setValue(vals.at(i));
        dsbInputDelay[i]->show();
    }
    ui->dsbInputDelaySet->show();
    ui->pbInputDelaySet->show();
}

void WinConfig::showOutputType(QList<int> vals)
{
    for(int i = 0; i < vals.count(); i++) {
        cbOutputType[i]->setCurrentIndex(vals.at(i));
        cbOutputType[i]->show();
    }
    ui->cbOutputTypeSet->show();
    ui->pbOutputTypeSet->show();
}

void WinConfig::showOutputSafe(QList<int> vals)
{
    for(int i = 0; i < vals.count(); i++) {
        dsbOutputSafe[i]->setValue(vals.at(i));
        dsbOutputSafe[i]->show();
    }
    ui->dsbOutputSafeSet->show();
    ui->pbOutputSafeSet->show();
}

void WinConfig::showServos(TMtbModuleConfigUNIS *cfg)
{
    if (!cfg) return;
    for(int i = 0; i < 6; i++) {
        bool enabled = ((cfg->servoEnabledMask >> i) & 1);
        chServoEnable[i]->setChecked(enabled);
        dsbServoPosA[i]->setValue(cfg->servoPosition[i].posA);
        dsbServoPosB[i]->setValue(cfg->servoPosition[i].posB);
        dsbServoSpeed[i]->setValue(cfg->servoSpeed[i]);

        chServoEnable[i]->show();
        if (enabled) {
            dsbServoPosA[i]->show();
            dsbServoPosB[i]->show();
            dsbServoSpeed[i]->show();
        }

        lServo[i]->show();
    }

}

void WinConfig::on_chServoEnable()
{
    bool enabled = false;
    for(int i = 0; i < 6; i++) {
        if(sender() == chServoEnable[i]) {
            enabled = chServoEnable[i]->isChecked();
            if (enabled) {
                dsbServoPosA[i]->show();
                dsbServoPosB[i]->show();
                dsbServoSpeed[i]->show();
            } else {
                dsbServoPosA[i]->hide();
                dsbServoPosB[i]->hide();
                dsbServoSpeed[i]->hide();
            }
            break;
        }
    }
}
