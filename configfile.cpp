#include "configfile.h"

configfile::configfile(QObject *parent)
    : QObject{parent}
{
    //doc = new QJsonDocument();
}

void configfile::loadfromfile(QString filename) {
    // load json to ram
    QFile fil;
    QJsonParseError jpe;
    fil.setFileName(filename);
    bool ok = fil.open(QIODevice::ReadOnly);
    if (!ok) qDebug("json - can't open file");
    QByteArray fileData = fil.readAll();
    doc = QJsonDocument::fromJson(fileData, &jpe);
    fil.close();
    if (doc.isNull()) {
        // document error
        qDebug("json - %d, %s", jpe.offset, jpe.errorString().toLatin1().toStdString().c_str());
        return;
    }


    //parse module records
    if (!doc.isObject()) {
        qDebug("json - no object, can't parse");
        return;
    }
    QJsonObject jsonObject = doc.object();
    if (!jsonObject.contains("modules")){
        qDebug("json - no \"modules\" section, can't parse");
        return;
    }
    QJsonValue jsonModulesVal = jsonObject["modules"];
    if (!jsonModulesVal.isObject()){
        qDebug("json - section \"modules\", don't contain any objects");
        return;
    }
    QJsonObject jsonModules = jsonModulesVal.toObject();
    QStringList jsonModulesAddr = jsonModules.keys();
/*
    if(jsonModules["016"].isArray())  qDebug("json - \"modules\" is array");
    if(jsonModules["016"].isObject()) qDebug("json - \"modules\" is object");
    if(jsonModules["016"].isNull())   qDebug("json - \"modules\" is null");
*/

    modules.clear();
    for(int i = 0; i < jsonModulesAddr.count(); i++) {
        //qDebug("json - key: %s", .toStdString().c_str());
        // iterate over all modules
        TMtbModuleState ms;
        QString moduleAddr = jsonModulesAddr[i];
        QString moduleName = jsonModules[moduleAddr].toObject().value("name").toString();
        int moduleType = jsonModules[moduleAddr].toObject().value("type").toInt();
        ms.address = moduleAddr.toInt();
        ms.name = moduleName;
        ms.type = moduleType;
        modules.append(ms);
    }
/*
    qDebug("json - array count: %lld", doc.array().count());
    qDebug("json - object count: %lld", doc.object().count());
*/
}

void configfile::savetofile(QString filename) {
    QFile fil;
    fil.setFileName(filename);
    fil.open(QIODevice::WriteOnly);
    fil.write(doc.toJson(QJsonDocument::Compact));
}

void configfile::sendJson(QJsonObject json)
{
    doc.setObject(json);
}

void configfile::subscribeModule(int module)
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
    tmpArr.append(QJsonValue(module));
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module_subscribe");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["addresses"] =  tmpArr;
    tmpObj["id"] = QJsonValue(this->id++);
    sendJson(tmpObj);
    //getOutputs(module); // get state now
}

void configfile::setServoOuts(int module, int servo, int state)
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

