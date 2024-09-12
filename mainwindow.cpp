#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fileMode = true;
    ml = &cfgfile.modules;
    servo_pos_act = 127;
    moduleSelected = -1;
    servo_pos_last = servo_pos_act;
    timer_update_view = new QTimer(this);
    timer_update_view->setInterval(200);
    QObject::connect(timer_update_view, SIGNAL(timeout()), this, SLOT(timer_tick()));
    timer_update_view->start();

    timer_autoclick = new QTimer(this);
    timer_autoclick->setInterval(100);
    QObject::connect(timer_autoclick, SIGNAL(timeout()), this, SLOT(timer_autoclick_tick()));
    timer_autoclick->stop();

    int i;
    for (i=0; i<16; i++) {
        io_label[i] = new QLabel(ui->gbIo);
        io_label[i]->setText(QString("%1").arg(i));
        io_label[i]->setGeometry(16,46+16*i,20,16);
        io_obutton[i] = new QPushButton(ui->gbIo);
        io_obutton[i]->setText("");
        io_obutton[i]->setGeometry(42,48+16*i,40,14);
        io_ibutton[i] = new QPushButton(ui->gbIo);
        io_ibutton[i]->setText("");
        io_ibutton[i]->setEnabled(false);
        io_ibutton[i]->setGeometry(84,48+16*i,40,14);
        connect(io_obutton[i], SIGNAL(clicked()), this, SLOT(pbOutsClicked()));
    }
    for (i=16; i<28; i++,i++) {
        io_label[i] = new QLabel(ui->gbIo);
        io_label[i]->setText(QString("Servo %1").arg((i >> 1)-7));
        io_label[i]->setGeometry(16,66+16*i,49,16);
        io_obutton[i+0] = new QPushButton(ui->gbIo);
        io_obutton[i+0]->setText("A");
        io_obutton[i+0]->setGeometry(70,58+16*i,40,30);
        io_obutton[i+1] = new QPushButton(ui->gbIo);
        io_obutton[i+1]->setText("B");
        io_obutton[i+1]->setGeometry(110,58+16*i,40,30);
        connect(io_obutton[i+0], SIGNAL(clicked()), this, SLOT(pbOutsClicked()));
        connect(io_obutton[i+1], SIGNAL(clicked()), this, SLOT(pbOutsClicked()));
    }
    initIOstatus();

    socket = new(tcpsocket);
    connect(socket, SIGNAL(connected()), this, SLOT(onSocketConnect()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnect()));
    connect(socket, SIGNAL(getModuleStateOut(QJsonObject)), this, SLOT(on_getModuleStateOut(QJsonObject)));
    connect(socket, SIGNAL(getModuleStateIn(QJsonObject)), this, SLOT(on_getModuleStateIn(QJsonObject)));
    connect(socket, SIGNAL(responseModuleList()), this, SLOT(on_responseModuleList()));
    connect(socket, SIGNAL(responseModuleInfo()), this, SLOT(on_responseModuleInfo()));
    socket->isConnected = false;
    connect(ui->pb_plus,            SIGNAL(pressed()), this, SLOT(pbPlusminusClicked()));
    connect(ui->pb_plusplus,        SIGNAL(pressed()), this, SLOT(pbPlusminusClicked()));
    connect(ui->pb_plusplusplus,    SIGNAL(pressed()), this, SLOT(pbPlusminusClicked()));
    connect(ui->pb_minus,           SIGNAL(pressed()), this, SLOT(pbPlusminusClicked()));
    connect(ui->pb_minusminus,      SIGNAL(pressed()), this, SLOT(pbPlusminusClicked()));
    connect(ui->pb_minusminusminus, SIGNAL(pressed()), this, SLOT(pbPlusminusClicked()));

    connect(&cfgfile, SIGNAL(moduleListChanged()), this, SLOT(mtbModuleListChanged()));

    // module types list
    for(i=0; i< MtbModuleTypes.count; i++) {
        ui->cbModuleType->addItem(MtbModuleTypes.GetNameByIndex(i));
    }

    ui->lvModuleList->setModel(&moduleModel);

    // init config window
    this->winConfig = new class WinConfig(this);
    //winConfig->activateWindow();

    // init list windows
    this->winList = new class WinList(this);
    //winList->activateWindow();

    // ukaže prázdné okno vpravo - schová ovládání desek
    changeLayoutType(-1);
}

MainWindow::~MainWindow()
{
    delete socket;
    delete ui;
}

void MainWindow::refreshModuleList()
{
    moduleSelected = -1;
    // update GUI
    //ui->pbModuleAdd->setEnabled(false);
    ui->pbModuleRemove->setEnabled(false);

    // get text module list
    moduleList.clear();
    if (fileMode) {
        // get items from config file class
        qDebug("refreshModuleList - file");
        for (int i=0; i < cfgfile.modules.count(); i++) {
            TMtbModuleState *ms = &(cfgfile.modules[i]);
            moduleList.append(tr("%1-%2").arg(ms->address).arg(ms->name));
        }
        // show items in list view
        refreshModuleListModel();
    } else {
        qDebug("refreshModuleList - socket");
        // get items from tcpsocket class
        cfgfile.modules.clear();
        for (int i=0; i < socket->modules.count(); i++) {
            TMtbModuleState *ms = &(socket->modules[i]);
            cfgfile.modules.append(*ms);
            if (ms->active) {
                moduleList.append(tr("%1-%2").arg(ms->address).arg(ms->name));
            } else {
                moduleList.append(tr("%1-%2 (disconnected)").arg(ms->address).arg(ms->name));
            }
        }
    }

    refreshModuleListModel();
}

void MainWindow::refreshModuleListModel()
{
    // refresh model
    for(int i = 0; i < moduleModel.rowCount(); i++) {
        delete moduleModel.item(i);
    }
    moduleModel.clear();
    for(int i = 0; i < moduleList.count(); i++) {
        QStandardItem *li;
        li = new QStandardItem(moduleList.at(i));
        li->setEditable(false);
        moduleModel.appendRow(li);
    }
}

void MainWindow::changeLayoutType(int type)
{
    // show default (nothing)
    ui->gbFirmware->hide();
    ui->gbIo->hide();
    ui->gbServo->hide();
    ui->gbAddress->hide();
    ui->pbDaemonReload->hide();
    ui->pbDaemonSave->hide();
    ui->pbLoc->hide();
    ui->pbLoc->setCheckable(true);
    ui->pbLoc->setChecked(false);
    ui->pbModuleList->hide();
    // show list of modules all times
    ui->gbModuleList->show();
    // in file mode don't show anything
    if (fileMode) type = -1;
    switch(type) {
    case 0x10 ... 0x17:
        ui->gbIo->show();
        ui->gbFirmware->show();
        break;
    case 0x50: // UNIS
        ui->gbIo->show();
        ui->gbServo->show();
        ui->gbFirmware->show();
        break;
    default:
        asm("nop;");
    }
    // type selectable only in file mode
    if (fileMode) {
        // u souboru jde měnit vše
        ui->cbModuleType->setEnabled(true);
        // ale nejde restartovat
        ui->pbReboot->hide();
    } else {
        TMtbModuleState *ms;
        // typ modulu jde měnit jen neaktivním modulům.
        if (moduleSelected > -1) {
            ms=&((*ml)[moduleSelected]);
            if (ms->active) {
                ui->cbModuleType->setEnabled(false);
            } else {
                ui->cbModuleType->setEnabled(true);
            }
        }
        // další volby u připojených modulů
        ui->gbAddress->show(); // změna adresy na sběrnici pro modul s tlačítkem
        ui->pbReboot->show();
        ui->pbDaemonReload->show();
        ui->pbDaemonSave->show();
        ui->pbLoc->show();
        ui->pbModuleList->show();
    }
}

void MainWindow::initIOstatus()
{
    int i;
    for (i=0; i<16; i++) {
        inputs[i] = -1;
    }
    for (i=0; i<28; i++) {
        outputs[i] = -1;
    }
}

void MainWindow::timer_tick()
{
    ui->l_pos_act->setText(QString("%1").arg(servo_pos_act));
    if (servo_pos_last != servo_pos_act) {
        sendServoPos();
    }
    servo_pos_last = servo_pos_act;

    // locator button
    if (!fileMode && socket->isConnected && (moduleSelected>-1)) {
        TMtbModuleState *ms;
        ms = &((*ml)[moduleSelected]);

        ui->pbLoc->setEnabled(true);
        ui->pbLoc->setChecked(ms->locator);
    } else {
        ui->pbLoc->setChecked(false);
        ui->pbLoc->setEnabled(false);
    }


    // show states
    int i;
    for (i=0; i<16; i++) {
        if (outputs[i] > 0) {
            io_obutton[i]->setStyleSheet("QPushButton{ background-color: red; }");
        } else if (outputs[i] == 0) {
            io_obutton[i]->setStyleSheet("QPushButton{ background-color: black; }");
        } else {
            io_obutton[i]->setStyleSheet("QPushButton{ background-color: lightgray; }");
        }
        if (inputs[i] > 0) {
            io_ibutton[i]->setStyleSheet("QPushButton{ background-color: green; }");
        } else if (inputs[i] == 0) {
            io_ibutton[i]->setStyleSheet("QPushButton{ background-color: black; }");
        } else {
            io_ibutton[i]->setStyleSheet("QPushButton{ background-color: lightgray; }");
        }
    }
}

void MainWindow::countButtonPos()
{
    bool run_ok = false;
    if (ui->pb_plus->isDown()) {
        servo_pos_act++;
        run_ok = true;
    }
    if (ui->pb_minus->isDown()) {
        servo_pos_act--;
        run_ok = true;
    }
    if (ui->pb_plusplus->isDown()) {
        servo_pos_act+=2;
        run_ok = true;
    }
    if (ui->pb_minusminus->isDown()) {
        servo_pos_act-=2;
        run_ok = true;
    }
    if (ui->pb_plusplusplus->isDown()) {
        servo_pos_act+=5;
        run_ok = true;
    }
    if (ui->pb_minusminusminus->isDown()) {
        servo_pos_act-=5;
        run_ok = true;
    }

    if (!run_ok)
        timer_autoclick->stop();
    else
        timer_autoclick->setInterval(100);

    if (servo_pos_act < SERVO_MIN) servo_pos_act = SERVO_MIN;
    if (servo_pos_act > SERVO_MAX) servo_pos_act = SERVO_MAX;
    ui->l_pos_act->setText(QString("%1").arg(servo_pos_act));
    if (servo_pos_last != servo_pos_act) {
        sendServoPos();
    }
    servo_pos_last = servo_pos_act;
}

void MainWindow::timer_autoclick_tick()
{
    countButtonPos();
}

void MainWindow::pbPlusminusClicked()
{
    countButtonPos();
    timer_autoclick->start(300);
}

void MainWindow::sendServoPos(void)
{
    socket->setServoManual(ui->sbModule->value(), ui->sb_servo->value(), servo_pos_act);
}

void MainWindow::on_pb_set_stop_clicked()
{
    socket->setServoManualEnd(ui->sbModule->value());
}

// connect to MTB-daemon
void MainWindow::on_pb_connect_clicked()
{
    socket->doConnect(ui->le_address->text(), ui->le_port->text().toInt());
}

// disconnect from MTB-daemon
void MainWindow::on_pb_disconnect_clicked()
{
    socket->doDisconnect();
    refreshModuleList();
    changeLayoutType(-1);
    ui->pb_connect->setEnabled(true);
    ui->pbLoadOffline->setEnabled(true);
    ui->pbSaveOffline->setEnabled(true);
    ui->gbAddress->hide(); // změna adresy na sběrnici pro modul s tlačítkem
    ui->pbDaemonReload->hide();
    ui->pbDaemonSave->hide();
    initIOstatus();
}

// load offline file
void MainWindow::on_pbLoadOffline_clicked()
{
    // first disconnect from MTB-daemon
    on_pb_disconnect_clicked();
    //show open file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Vyber soubor mtb-daemon.json"), nullptr, ("json (*.json)"));

    if (fileName != "") {
        //qDebug("open file: %s", fileName.toStdString().c_str());
        cfgfile.loadfromfile(fileName);

        //qDebug("module count: %lld", cfgfile.modules.count());
        setFileMode(true);
        ml = &cfgfile.modules;
        changeLayoutType(-1);
    }
}

void MainWindow::mtbModuleListChanged()
{
    refreshModuleList();
}

void MainWindow::on_pbSaveOffline_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Vyber kam uložit"), nullptr, ("json (*.json)"));
    cfgfile.savetofile(fileName);
}

void MainWindow::pbOutsClicked()
{
    int i;
    for (i=0; i<16; i++) {
        if (((QPushButton *) sender()) == io_obutton[i]) {
            // match normal out
            //qDebug() << (QString("out request ") + QString::number(i));
            int module = ui->sbModule->value();
            int port   = i;
            if (outputs[port] == 0) {
                outputs[port] = 1;
            } else {
                outputs[port] = 0;
            }
            socket->setOutputs(module, port, outputs[port]);
            break;
        }
    }
    for (i=16; i<28; i++) {
        if (((QPushButton *) sender()) == io_obutton[i]) {
            // match servo out
            int servo = (i-16)/2 + 1;
            int state = ((i-16) & 1) ? 1 : 0;
            qDebug() << (QString("servo request ") + QString::number(servo) + QString(" pos ") + ((state) ? QString('B') :QString('A')));

            socket->setServoOuts(ui->sbModule->value(), servo, state);
            // ui->sb_servo->value()
            break;
        }
    }
}

void MainWindow::setFileMode(bool amode)
{
    fileMode = amode;
    if (amode) {
        this->setWindowTitle(winlabel + " - " + tr("souborový režim"));
        ui->pbModuleListRefresh->setEnabled(false);
    } else {
        this->setWindowTitle(winlabel + " - " + tr("USB režim"));
        ui->pbLoadOffline->setEnabled(false);
        ui->pbSaveOffline->setEnabled(false);
    }
}

void MainWindow::onSocketConnect()
{
    ui->pb_connect->setEnabled(false);
    setFileMode(false);

    ml = &socket->modules;
    changeLayoutType(-1);
    qDebug("get module list");
    socket->getModuleList();
}

void MainWindow::onSocketDisconnect()
{

}

void MainWindow::on_getModuleStateOut(QJsonObject json)
{
    //QJsonObject resp = json.object();
    if (json.contains("outputs")) {
        QJsonObject outs = json.value("outputs").toObject();
        // we have some outputs states
        for (const QString &outKey : outs.keys()) {
            QJsonObject out = outs.value(outKey).toObject();
            if (out.contains("value")) {
                outputs[outKey.toInt()] = out.value("value").toInt();
            }
        }
    }
}

void MainWindow::on_getModuleStateIn(QJsonObject json)
{
    //QJsonObject resp = json.object();
    if (json.contains("inputs")) {
        QJsonObject ins = json.value("inputs").toObject();
        if (ins.contains("full")) {
            QJsonArray inarr = ins["full"].toArray();
            //int num = 1;
            int i=0;
            for (const QJsonValueRef & val : inarr) {
                if (QJsonValue(val).toBool()) {
                    inputs[i] = 1;
                } else
                    inputs[i] = 0;
                //num <<= 1;
                i++;
            }
        }
    }
}

void MainWindow::on_responseModuleList()
{
    qDebug("on_responseModuleList");
    refreshModuleList();
}

void MainWindow::on_responseModuleInfo()
{
    //
    if (moduleSelected > -1) {
        TMtbModuleState *ms;
        ms=&((*ml)[moduleSelected]);
        if (ms != nullptr) {
            changeLayoutType(ms->type);
        }
    }
}

void MainWindow::on_pb_servoposset_clicked()
{
    servo_pos_act = ui->sb_servopos->value();
    sendServoPos();
}

void MainWindow::on_pb_servoposset2_clicked()
{
    servo_pos_act = ui->sb_servopos2->value();
    sendServoPos();
}

void MainWindow::on_pbReboot_clicked()
{
    socket->reboot(ui->sbModule->value());
}

void MainWindow::on_pbDaemonReload_clicked()
{
    socket->loadconfig();
}

void MainWindow::on_pbDaemonSave_clicked()
{
    socket->saveconfig();
}

void MainWindow::on_pb_browsefw_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, ("Vyber Firmware"), nullptr, ("HEX (*.hex)"));

    if (fileName != "") {
        QDir dir(QCoreApplication::applicationDirPath());
        fileName = dir.relativeFilePath(fileName);
        ui->le_fw->setText(fileName);
    }
}

void MainWindow::on_pb_fw_upgrade_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "FW upgrade", "Opravdu přehrát FW v desce "+QString::number(ui->sbModule->value())+"?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        socket->upgrade_fw(ui->sbModule->value(), ui->le_fw->text());
    }
}

void MainWindow::on_pbModuleChangeAddress_clicked()
{
    TMtbModuleState *ms = NULL;
    if (moduleSelected > -1) ms=&((*ml)[moduleSelected]); else return; // get pointer to selected module
    int addrNew = ui->sbModule->value();
    int addrOld = ms->address;

    // find duplicate address
    bool dup = false;
    for(int i = 0; i < ml->count(); i++) {
        if (ml->at(i).address == addrNew) dup = true;
    }
    if (dup) {
        QMessageBox::warning(
            this,
            tr("nelze"),
            tr("Nová adresa je stejná s jiným modulem.")
        );
        return;
    }

    if (fileMode) {
        ms->address = addrNew;
        refreshModuleList();
    } else {
        // online mode
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("změna adresy"),
            tr("Opravdu se má změnit adresa modulu %1 na novou adresu %2").arg(addrOld).arg(addrNew),
            QMessageBox::Yes|QMessageBox::No
            );
        if (reply == QMessageBox::Yes) {
            socket->setModuleAddress(addrOld, addrNew);
        }
    }
}

void MainWindow::on_pbModuleChangeName_clicked()
{
    int addr = ui->sbModule->value();
    if (fileMode) {
        cfgfile.setModuleName(addr, ui->leModuleName->text());
    }
}

void MainWindow::on_pbModuleChangeType_clicked()
{
    int addr = ui->sbModule->value();
    int typeindex = ui->cbModuleType->currentIndex();
    int newtype = MtbModuleTypes.IndexToType(typeindex);
    if (fileMode) {
        cfgfile.setModuleType(addr, newtype);
    }
}


void MainWindow::on_pbSettings_clicked()
{
    on_lvModule_settings();
}

void MainWindow::on_lvModule_settings()
{
    TMtbModuleState *ms;
    if (moduleSelected > -1) {
        ms=&((*ml)[moduleSelected]); // get pointer to selected module
        TMtbModuleConfigGeneric *moduleCfg;
        moduleCfg = ms->config;
        this->winConfig->showConfig(moduleCfg, ms->type);
    }
}

// change board by keyboard (Enter)
void MainWindow::on_lvModuleList_activated(const QModelIndex &index)
{
    on_lvModule_changed(index.row());
}

// change board by mouse (clicked)
void MainWindow::on_lvModuleList_clicked(const QModelIndex &index)
{
    on_lvModule_changed(index.row());
}

void MainWindow::on_lvModuleList_doubleClicked()
{
    on_lvModule_settings();
}

// change board
void MainWindow::on_lvModule_changed(const int linenum)
{
    //QStringList split = moduleList.at(linenum).split("-");
    //if (split.count() > 1) {
    TMtbModuleState *ms = NULL;
    ms=&((*ml)[linenum]); // get pointer to selected module
    TMtbModuleState *ms_old = NULL;
    if (moduleSelected > -1) ms_old=&((*ml)[moduleSelected]); // get pointer to selected module
    if (fileMode) {
        changeLayoutType(-1);
    } else {
        changeLayoutType(ms->type);
        if (ms_old) socket->unsubscribeModule(ms_old->address);

        socket->subscribeModule(ms->address);
        socket->getOutputs(ms->address); // implicit module info
    }
    moduleSelected = linenum;
    ui->sbModule->setValue(ms->address);
    ui->leModuleName->setText(ms->name);
    int modtypeindex = MtbModuleTypes.TypeToIndex(ms->type);
    if (modtypeindex >= 0) {
        ui->cbModuleType->setCurrentIndex(modtypeindex);
    } else {
        ui->cbModuleType->setCurrentIndex(-1);
    }
    if (fileMode) {
        //ui->pbModuleAdd->setEnabled(true);
        ui->pbModuleRemove->setEnabled(true);
    } else {
        //ui->pbModuleAdd->setEnabled(false);
        if (ms->active) {
            ui->pbModuleRemove->setEnabled(false);
        } else {
            ui->pbModuleRemove->setEnabled(true);
        }
    }
}

void MainWindow::on_pbModuleRemove_clicked()
{
    if (moduleSelected == -1) {
        return;
    }

    // ukazatel na modul (soubor/sběrnice)
    TMtbModuleState *ms;
    ms = &((*ml)[moduleSelected]);
    if (fileMode) {
        //ms = &(cfgfile.modules[moduleSelected]);
    } else {
        //ms = &(socket->modules[moduleSelected]);
        if (ms->active) {
            QMessageBox::warning(this,tr("nelze"), tr("Modul je aktivní, nelze ho smazat z konfigurace"), QMessageBox::Ok);
            return;
        }
    }

    // potvrzení od uživatele
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr("Smazat"),
                                                  tr("Opravdu smazat modul %1-%2 ?").arg(ms->address).arg(ms->name))) {
        // odstranit ze seznamu modulů

        if (fileMode) {
            cfgfile.modules.remove(moduleSelected);
        } else {
            //socket->modules.removeOne(*ms);
        }
    }
    refreshModuleList();
}

void MainWindow::on_pbModuleAdd_clicked()
{
    int addr = ui->sbModuleAdd->value();

    bool duplicate = false;

    for (int i = 0; i < ml->count(); i++) {
        if (ml->at(i).address == addr) duplicate = true;
    }

    if (duplicate) {
        QMessageBox::warning(this,tr("nelze"), tr("Modul se stejnou adresou již existuje."), QMessageBox::Ok);
        return;
    }

    TMtbModuleState mms;
    mms.address = addr;
    mms.type = -1;
    mms.name = tr("mtb");
    TMtbModuleConfigUNI *confuni = new TMtbModuleConfigUNI;
    mms.config = confuni;

    if (ml->count() < 1) {
        // insert first module
        ml->append(mms);
    } else {
        // find right place for new module
        for (int i = 0; i < ml->count(); i++) {
            if (addr < ml->at(i).address) {
                ml->insert(i, mms);
                refreshModuleList();
                return;
            }
        }
        ml->append(mms);
    }
    refreshModuleList();
}

void MainWindow::on_pbLoc_clicked()
{
    // zapne nebo vypne lokátor
    if (!fileMode && socket && (moduleSelected>-1)) {
        // ukazatel na modul
        TMtbModuleState *ms;
        ms = &((*ml)[moduleSelected]);
        socket->setModuleLocator(ms->address, ui->pbLoc->isChecked());
        socket->getOutputs(ms->address); // implicit module info
    }
}


void MainWindow::on_pbModuleListRefresh_clicked()
{
    if (!fileMode && socket && socket->isConnected) {
        socket->getModuleList();
    }
}


void MainWindow::on_pbModuleList_clicked()
{
    winList->show();
}
