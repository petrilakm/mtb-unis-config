#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fileMode = false;
    servo_pos_act = 127;
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
        //io_ibutton[i]->setEnabled(false);
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

    for(i=0; i< MtbModuleTypes.count; i++) {
        ui->cbModuleType->addItem(MtbModuleTypes.GetNameByIndex(i));
    }
    moduleList.append(tr("1-test"));
    moduleList.append(tr("10-UNI"));
    moduleList.append(tr("11-UNIS"));

    ui->lvModuleList->setModel(&moduleModel);
    QStandardItem *it;
    it = new QStandardItem(tr("test")); it->setEditable(false);
    moduleModel.appendRow(it);
    it = new QStandardItem(tr("10 - UNI")); it->setEditable(false);
    moduleModel.appendRow(it);
    it = new QStandardItem(tr("11 - UNIS")); it->setEditable(false);
    moduleModel.appendRow(it);

    this->winConfig = new class winConfig(this);
    winConfig->activateWindow();
}

MainWindow::~MainWindow()
{
    delete socket;
    delete ui;
}

void MainWindow::refreshModuleList()
{
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
        for (int i=0; i < socket->modules.count(); i++) {
            TMtbModuleState *ms = &(socket->modules[i]);
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
        moduleModel.appendRow(li);
    }
}

void MainWindow::changeLayoutType(int type)
{
    ui->gbFirmware->hide();
    ui->gbIo->hide();
    ui->gbServo->hide();
    ui->gbModuleList->show();
    switch(type) {
    case 0x50: // UNIS
        ui->gbIo->show();
        ui->gbServo->show();
        ui->gbFirmware->show();
        break;
    default:
        ui->gbFirmware->hide();
        asm("nop;");
    }
    if (fileMode) {
        ui->cbModuleType->setEnabled(true);
    } else {
        ui->cbModuleType->setEnabled(false);
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
        qDebug("open file: %s", fileName.toStdString().c_str());
        cfgfile.loadfromfile(fileName);

        qDebug("module count: %lld", cfgfile.modules.count());
        fileMode = true;
        changeLayoutType(-1);
        refreshModuleList();
    }
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
            outputs[port] ^= 1;
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

void MainWindow::onSocketConnect()
{
    ui->pb_connect->setEnabled(false);
    fileMode = false;
    changeLayoutType(-1);
    qDebug("get module list");
    socket->getModuleList();
    //socket->subscribeModule(ui->sbModule->value());
}

void MainWindow::onSocketDisconnect()
{
    ui->pb_connect->setEnabled(true);
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

void MainWindow::on_pb_reload_clicked()
{
    socket->loadconfig();
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

void MainWindow::on_pbSettings_clicked()
{
    this->winConfig->show();
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

// change board
void MainWindow::on_lvModule_changed(const int linenum)
{
    //QStringList split = moduleList.at(linenum).split("-");
    //if (split.count() > 1) {
    TMtbModuleState *ms;
    if (fileMode) {
        ms = &(cfgfile.modules[linenum]);
    } else {
        ms = &(socket->modules[linenum]);
    }
    ui->sbModule->setValue(ms->address);
    ui->leModuleName->setText(ms->name);
    int modtypeindex = MtbModuleTypes.TypeToIndex(ms->type);
    if (modtypeindex >= 0) {
        ui->cbModuleType->setCurrentIndex(modtypeindex);
    } else {
        ui->cbModuleType->setCurrentIndex(-1);
    }
    changeLayoutType(ms->type);
}
