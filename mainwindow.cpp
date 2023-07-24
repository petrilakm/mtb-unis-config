#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
        io_label[i] = new QLabel(ui->gb_io);
        io_label[i]->setText(QString("%1").arg(i));
        io_label[i]->setGeometry(16,46+16*i,20,16);
        io_obutton[i] = new QPushButton(ui->gb_io);
        io_obutton[i]->setText("");
        io_obutton[i]->setGeometry(42,48+16*i,40,14);
        io_ibutton[i] = new QPushButton(ui->gb_io);
        io_ibutton[i]->setText("");
        //io_ibutton[i]->setEnabled(false);
        io_ibutton[i]->setGeometry(84,48+16*i,40,14);
        connect(io_obutton[i], SIGNAL(clicked()), this, SLOT(on_pb_outs_clicked()));
    }
    for (i=16; i<28; i++,i++) {
        io_label[i] = new QLabel(ui->gb_io);
        io_label[i]->setText(QString("Servo %1").arg((i >> 1)-7));
        io_label[i]->setGeometry(16,66+16*i,49,16);
        io_obutton[i+0] = new QPushButton(ui->gb_io);
        io_obutton[i+0]->setText("A");
        io_obutton[i+0]->setGeometry(70,58+16*i,40,30);
        io_obutton[i+1] = new QPushButton(ui->gb_io);
        io_obutton[i+1]->setText("B");
        io_obutton[i+1]->setGeometry(110,58+16*i,40,30);
        connect(io_obutton[i+0], SIGNAL(clicked()), this, SLOT(on_pb_outs_clicked()));
        connect(io_obutton[i+1], SIGNAL(clicked()), this, SLOT(on_pb_outs_clicked()));
    }
    initIOstatus();

    socket = new(tcpsocket);
    connect(socket, SIGNAL(connected()), this, SLOT(onSocketConnect()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnect()));
    connect(socket, SIGNAL(getModuleStateOut(QJsonObject)), this, SLOT(on_getModuleStateOut(QJsonObject)));
    connect(socket, SIGNAL(getModuleStateIn(QJsonObject)), this, SLOT(on_getModuleStateIn(QJsonObject)));
    socket->isConnected = false;
    connect(ui->pb_plus,            SIGNAL(pressed()), this, SLOT(on_plusminus_clicked()));
    connect(ui->pb_plusplus,        SIGNAL(pressed()), this, SLOT(on_plusminus_clicked()));
    connect(ui->pb_plusplusplus,    SIGNAL(pressed()), this, SLOT(on_plusminus_clicked()));
    connect(ui->pb_minus,           SIGNAL(pressed()), this, SLOT(on_plusminus_clicked()));
    connect(ui->pb_minusminus,      SIGNAL(pressed()), this, SLOT(on_plusminus_clicked()));
    connect(ui->pb_minusminusminus, SIGNAL(pressed()), this, SLOT(on_plusminus_clicked()));
}

MainWindow::~MainWindow()
{
    delete socket;
    delete ui;
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
            io_obutton[i]->setStyleSheet("QPushButton{ background-color: red }");
        } else if (outputs[i] == 0) {
            io_obutton[i]->setStyleSheet("QPushButton{ background-color: gray }");
        } else {
            io_obutton[i]->setStyleSheet("QPushButton{ background-color: lightgray }");
        }
        if (inputs[i] > 0) {
            io_ibutton[i]->setStyleSheet("QPushButton{ background-color: green }");
        } else if (inputs[i] == 0) {
            io_ibutton[i]->setStyleSheet("QPushButton{ background-color: gray }");
        } else {
            io_ibutton[i]->setStyleSheet("QPushButton{ background-color: lightgray }");
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

void MainWindow::on_plusminus_clicked()
{
    countButtonPos();
    timer_autoclick->start(300);
}

void MainWindow::sendServoPos(void)
{
     socket->setServoManual(ui->sb_module->value(), ui->sb_servo->value(), servo_pos_act);
}

void MainWindow::on_pb_set_stop_clicked()
{
    socket->setServoManualEnd(ui->sb_module->value());
}


void MainWindow::on_pb_connect_clicked()
{
    socket->doConnect(ui->le_address->text(), ui->le_port->text().toInt());
    //ui->pb_connect->setEnabled(false);
}


void MainWindow::on_pb_disconnect_clicked()
{
    socket->doDisconnect();
    ui->pb_connect->setEnabled(true);
    initIOstatus();
}

void MainWindow::on_pb_outs_clicked()
{
    int i;
    for (i=0; i<16; i++) {
        if (((QPushButton *) sender()) == io_obutton[i]) {
            // match normal out
            //qDebug() << (QString("out request ") + QString::number(i));
            int module = ui->sb_module->value();
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

            socket->setServoOuts(ui->sb_module->value(), servo, state);
            // ui->sb_servo->value()
            break;
        }
    }
}

void MainWindow::onSocketConnect()
{
    ui->pb_connect->setEnabled(false);
    socket->subscribeModule(ui->sb_module->value());
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
            for (const QJsonValueConstRef & val : inarr) {
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
