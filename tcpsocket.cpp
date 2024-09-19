#include "tcpsocket.h"

tcpsocket::tcpsocket(QObject *parent)
    : QObject{parent}
{
    tim = new QTimer(this);
}

void tcpsocket::doConnect(QString addr, int port)
{
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()),this, SLOT(socket_connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(socket_disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));

    qDebug() << "connecting...";

    tim->setInterval(1000);

    qDebug() << QString(addr);
    qDebug() << QString::number(port);

    this->id = 0;

    // this is not blocking call
    socket->connectToHost(addr, port, QAbstractSocket::ReadWrite, QAbstractSocket::IPv4Protocol);

    // we need to wait...
    if(!socket->waitForConnected(1500))
    {
        qDebug() << "Error: " << socket->errorString();
    }
}

void tcpsocket::doDisconnect()
{
    if (isConnected) {
        isConnected = false;
        socket->disconnect();
        delete socket;
        modules.clear();
    }
}

void tcpsocket::socket_connected()
{
    qDebug() << "connected...";
    isConnected = true;
    emit connected();

    // Hey server, tell me about you.
    //socket->write("HEAD / HTTP/1.0\r\n\r\n\r\n\r\n");
}

void tcpsocket::socket_disconnected()
{
    qDebug() << "disconnected...";
    emit disconnected();
    isConnected = false;
    delete socket;
}

void tcpsocket::bytesWritten(qint64 bytes)
{
    //qDebug() << bytes << " bytes written...";
}

void tcpsocket::readyRead()
{
    static QByteArray buf;
    QJsonDocument jsondoc;

    buf.append(socket->readAll());
    if (buf.contains('\n')) {
        QList<QByteArray> chunks = buf.split('\n');
        buf.clear();
        if (!chunks.last().endsWith('\n'))
            buf.append(chunks.takeLast()); // leave only last chunk
        foreach (QByteArray chunk, chunks) {
            if (chunk.length() > 5) { // ignore short messages
                qDebug() << "< incoming:";
                qDebug() << qPrintable(chunk); // show incoming data
                jsondoc = QJsonDocument::fromJson(chunk);
                if (jsondoc.isObject() == false) {
                    qDebug() << "response is bad JSON, can\'t parse.";
                    continue;
                }
                QJsonObject lvl1 = jsondoc.object();
                if (lvl1.contains("command")) {
                    if (lvl1["command"] == "module_set_outputs") {
                        if (lvl1.contains("outputs"))
                            emit getModuleStateOut(lvl1);
                    }

                    if (lvl1["command"] == "modules") {
                        if (lvl1.contains("modules"))
                            parseModuleList(lvl1["modules"].toObject());
                    }
                    if (lvl1["command"] == "module") {
                        if (lvl1.contains("module")) {
                            QJsonObject lvl2 = lvl1["module"].toObject();
                            if (lvl2.constBegin()->isObject()) {
                                parseModuleInfo(lvl2);
                                QJsonObject lvl3 = lvl2.constBegin()->toObject();
                                if (lvl3.contains("state")) {
                                    QJsonObject lvl4 = lvl3["state"].toObject();
                                    emit getModuleStateOut(lvl4);
                                    emit getModuleStateIn(lvl4);
                                }
                            }
                        }
                    }

                    if (lvl1["command"] == "module_beacon") {
                        if (lvl1.contains("beacon")) {
                            parseModuleBeacon(lvl1);
                        }
                    }

                    if (lvl1["command"] == "module_outputs_changed") {
                        if (lvl1.contains("module_outputs_changed")) {
                            QJsonObject lvl2 = lvl1["module_outputs_changed"].toObject();
                            emit getModuleStateOut(lvl2);
                        }
                    }
                    if (lvl1["command"] == "module_inputs_changed") {
                        if (lvl1.contains("module_inputs_changed")) {
                            QJsonObject lvl2 = lvl1["module_inputs_changed"].toObject();
                            emit getModuleStateIn(lvl2);
                        }
                    }

                }
            }
        }
        //qDebug() << "buf length: "+QString::number(buf.length());
    }
}

void tcpsocket::parseModuleList(QJsonObject json)
{
    qDebug("socket: parse module list");
    modules.clear();
    QStringList modkeys = json.keys();
    for (int i = 0; i < modkeys.count(); i++) {
        QString modaddr = modkeys[i];
        QJsonObject mod = json[modaddr].toObject();
        QString modTypeString = mod["type"].toString("MTB-UNIS");
        QJsonObject modConfig;
        int modType = 0x10;
        if (modTypeString == "MTB-UNIS") {
            modType = 0x50;
            modConfig = mod["MTB-UNIS"].toObject()["config"].toObject();

        } else {
            modType = 0x15; // ToDo: odělat navázání na typy modulů
            modConfig = mod["MTB-UNI"].toObject()["config"].toObject();
        }
        TMtbModuleState ms(modType);
        ms.address = modaddr.toInt();
        if (modType == 0x50) {
            static_cast<TMtbModuleConfigUNIS*>(ms.config)->setJson(modConfig);
        } else {
        }

        qDebug("socket: module add %s", modaddr.toStdString().c_str());
        modules.append(ms);
        parseModuleInfo(mod);
    }
    emit responseModuleList();
}

void tcpsocket::parseModuleInfo(QJsonObject json)
{
    QJsonObject mod = json;
    int address = mod["address"].toInt(-1);
    if (address > -1) {
        int pos = -1;
        for (int i = 0; i < modules.count(); i++) {
            if (modules[i].address == address) {
                pos = i;
                break;
            }
        }
        if (pos > -1) {
            //qDebug("socket: module info (pos %d", pos);
            QString modname = mod["name"].toString();
            int modtype = mod["type_code"].toInt();
            bool modstate = mod["state"] == QString("active");
            bool modwarning = mod["warning"].toBool();
            bool moderror = mod["error"].toBool();
            bool modloc = mod["beacon"].toBool();

            TMtbModuleState *ms = &(modules[pos]);
            ms->name = modname;
            ms->type = modtype;
            ms->active = modstate;
            ms->warning = modwarning;
            ms->error = moderror;
            ms->locator = modloc;
            emit responseModuleInfo();
        }
    }
}

void tcpsocket::parseModuleBeacon(QJsonObject json)
{
    QJsonObject mod = json;
    int address = mod["address"].toInt(-1);
    if (address > -1) {
        int pos = -1;
        for (int i = 0; i < modules.count(); i++) {
            if (modules[i].address == address) {
                pos = i;
                break;
            }
        }
        if (pos > -1) {
            //qDebug("socket: module info (pos %d", pos);
            bool modloc = mod["beacon"].toBool();

            TMtbModuleState *ms = &(modules[pos]);
            ms->locator = modloc;
            emit responseModuleInfo();
        }
    }
}

void tcpsocket::getModuleList()
{
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("modules");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["state"] = QJsonValue(false);
    sendJson(tmpObj);
}

void tcpsocket::getModuleInfo(int module)
{
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["state"] = QJsonValue(false);
    qDebug() << "INFO !!!!!!!!!!! INFO";
    //sendJson(tmpObj);
}

void tcpsocket::subscribeModule(int addr)
{
    /*
{
    "command": "module_subscribe"/"module_unsubscribe",
    "type": "request",
    "id": 12,
    "addresses": [10, 11, 20]
}
*/
    QJsonArray tmpArr;
    tmpArr.append(QJsonValue(addr));
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module_subscribe");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["addresses"] =  tmpArr;
    tmpObj["id"] = QJsonValue(this->id++);
    sendJson(tmpObj);
}

void tcpsocket::unsubscribeModule(int addr)
{
    QJsonArray tmpArr;
    tmpArr.append(QJsonValue(addr));
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module_unsubscribe");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["addresses"] =  tmpArr;
    tmpObj["id"] = QJsonValue(this->id++);
    sendJson(tmpObj);
}

void tcpsocket::getOutputs(int module)
{
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["state"] = QJsonValue(true);
    tmpObj["id"] = QJsonValue(this->id++);

    sendJson(tmpObj);
    return;
}

void tcpsocket::setOutputs(int module, int port, int state)
{
    QJsonObject tmpOut;
    QJsonObject tmpOutOne;

    tmpOutOne["type"] = QJsonValue("plain");
    tmpOutOne["value"] = QJsonValue(state);
    tmpOut[QString::number(port)] = tmpOutOne;

    QJsonObject tmpObj;
    tmpObj["type"] = "request";

    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_set_outputs");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["outputs"] = tmpOut;
    tmpObj["state"] = QJsonValue(false);

    sendJson(tmpObj);
    return;
}

void tcpsocket::setServoOuts(int module, int servo, int state)
{
    QJsonObject tmpOut;
    QJsonObject tmpOutOne;
    int out1 = 16+(servo-1)*2;
    int out2 = out1 + 1;
    int state1 = (state) ? 0 : 1; // inversion, state 1 = first output 0, second output 1
    int state2 = 1-state1;

    // output states
    tmpOutOne["type"] = QJsonValue("plain");
    tmpOutOne["value"] = QJsonValue(state1);
    tmpOut[QString::number(out1)] = tmpOutOne;
    tmpOutOne["type"] = QJsonValue("plain");
    tmpOutOne["value"] = QJsonValue(state2);
    tmpOut[QString::number(out2)] = tmpOutOne;

    // main json
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_set_outputs");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["outputs"] = tmpOut;
    tmpObj["state"] = QJsonValue(false); // dont report states

    sendJson(tmpObj);
}

void tcpsocket::setServoManual(int module, int servo, uint8_t position)
{
    // main json
    QJsonObject tmpObj;
    QJsonArray tmpArr;

    tmpArr.push_back(QJsonValue(3));
    tmpArr.push_back(QJsonValue(servo << 1));
    tmpArr.push_back(QJsonValue(position));

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_specific_command");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["data"] = tmpArr;

    sendJson(tmpObj);
}

void tcpsocket::setServoManualEnd(int module)
{
    // main json
    QJsonObject tmpObj;
    QJsonArray tmpArr;

    tmpArr.push_back(QJsonValue(3));
    tmpArr.push_back(QJsonValue(0));

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_specific_command");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["data"] = tmpArr;

    sendJson(tmpObj);
}

void tcpsocket::setModuleAddress(int newAddress)
{
    // main json
    QJsonObject tmpObj;

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("set_address");
    tmpObj["new_address"] = newAddress;

    sendJson(tmpObj);
}

void tcpsocket::setModuleAddress(int oldAddress, int newAddress)
{
    // main json
    QJsonObject tmpObj;

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_set_address");
    tmpObj["address"] = QJsonValue(oldAddress);
    tmpObj["new_address"] = newAddress;

    sendJson(tmpObj);
}

void tcpsocket::setModuleLocator(int module, bool state)
{
    // main json
    QJsonObject tmpObj;

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_beacon");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["beacon"] = state;

    sendJson(tmpObj);
}

void tcpsocket::setModuleConfig(int module, QJsonObject cfg)
{
    QJsonObject tmpObj;

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_set_config");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["config"] = cfg;

    sendJson(tmpObj);
}

void tcpsocket::reboot(int module)
{
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_reboot");
    tmpObj["address"] = QJsonValue(module);

    sendJson(tmpObj);
}

void tcpsocket::loadconfig(void)
{
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("load_config");

    sendJson(tmpObj);
}

void tcpsocket::saveconfig(void)
{
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("save_config");

    sendJson(tmpObj);
}

void tcpsocket::upgrade_fw(int module, QString filename)
{
    QJsonObject tmpObj;
    QJsonObject firmware;

    int type;
    int addr;
    int offset = 0;

    // ":100000000C9446010C9465010C9465010C946501F7"
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            type = line.mid(7, 2).toInt(nullptr, 16);
            addr = offset + line.mid(3, 4).toInt(nullptr, 16);
            qDebug() << line.mid(3, 4) + " - " + QString::number(addr);
            if (type == 2) { // change address
                offset = line.mid(9, 4).toInt(nullptr, 16) << 4;
            }
            if (type == 0) { // data
                line.remove(0, 9).chop(2);
                firmware.insert(QString::number(addr), QJsonValue(line));
            }
        }
        inputFile.close();
    } else {
        // file not found
    }

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_upgrade_fw");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["firmware"] = firmware;

    sendJson(tmpObj);
}

void tcpsocket::sendJson(QJsonObject json)
{
    if (isConnected) {
        QJsonDocument tmpJson(json);
        QByteArray req = tmpJson.toJson(QJsonDocument::Compact);
        qDebug() << " > requests:";
        qDebug() << qPrintable(req); // show requests
        socket->write(req+QByteArray("\n\n"));
    }
}
