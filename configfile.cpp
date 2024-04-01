#include "configfile.h"
//#include "qmessagebox.h"

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

    modules.clear();
    for(int i = 0; i < jsonModulesAddr.count(); i++) {
        //qDebug("json - key: %s", .toStdString().c_str());
        // iterate over all modules
        TMtbModuleState ms;
        QString moduleAddr = jsonModulesAddr[i];
        QJsonObject jsonCurrentModule = jsonModules[moduleAddr].toObject();
        QString moduleName = jsonCurrentModule.value("name").toString();
        int moduleType = jsonCurrentModule.value("type").toInt();
        QJsonObject jsonCurrentModuleConfig = jsonCurrentModule.value("config").toObject();
        ///


        //
        ms.address = moduleAddr.toInt();
        ms.name = moduleName;
        ms.type = moduleType;

        modules.append(ms);
        parseOneConfig(i, jsonCurrentModuleConfig);
    }
    emit moduleListChanged();
}

void configfile::parseOneConfig(int pos, QJsonObject jsonConf)
{
    TMtbModuleState *ms;
    ms = &(modules[pos]);
    QJsonArray outputsSafe;
    QJsonArray inputsDelay;
    QJsonArray servoPosition;
    QJsonArray servoSpeed;
    TMtbModuleConfigUNI *configUni;
    TMtbModuleConfigUNIS *configUnis;
    switch (ms->type) {
    case 0x10 ... 0x2F: // UNI (all versions)
        configUni = new TMtbModuleConfigUNI();
        inputsDelay = jsonConf.value("inputsDelay").toArray();
        for(int j = 0; j < 16; j++) {
            if (j < inputsDelay.count()) {
                configUni->inputsDelay[j] = inputsDelay.at(j).toDouble(0.2);
            } else {
                //use default value
                configUni->inputsDelay[j] = 0.1;
            }
        }

        outputsSafe = jsonConf.value("outputsSafe").toArray();
        for(int j = 0; j < 16; j++) {
            if (j < outputsSafe.count()) {
                QJsonObject outputSafeItem = outputsSafe.at(j).toObject();
                QString outputSafeItemType = outputSafeItem.value("type").toString();
                int outputSafeItemTypeVal = TMtbModuleTypes::ModuleUniOutputTypeGetType(outputSafeItemType);
                configUni->outputsSafe[j].type = outputSafeItemTypeVal;
                configUni->outputsSafe[j].value = outputSafeItem.value("type").toInt(0);
            } else {
                //use default value
                configUni->outputsSafe[j].value = 0;
                configUni->outputsSafe[j].type = 0;
            }
        }
        ms->config = (TMtbModuleConfigGeneric *) configUni;
        break;
    case 0x50: // UNIS
        configUnis = new TMtbModuleConfigUNIS();

        inputsDelay = jsonConf.value("inputsDelay").toArray();
        for(int j = 0; j < 16; j++) {
            if (j < inputsDelay.count()) {
                configUnis->inputsDelay[j] = inputsDelay.at(j).toDouble(0.2);
            } else {
                //use default value
                configUnis->inputsDelay[j] = 0.1;
            }
        }

        outputsSafe = jsonConf.value("outputsSafe").toArray();
        for(int j = 0; j < 28; j++) {
            if (j < outputsSafe.count()) {
                QJsonObject outputSafeItem = outputsSafe.at(j).toObject();
                QString outputSafeItemType = outputSafeItem.value("type").toString();
                int outputSafeItemTypeVal = TMtbModuleTypes::ModuleUniOutputTypeGetType(outputSafeItemType);
                configUnis->outputsSafe[j].type = outputSafeItemTypeVal;
                configUnis->outputsSafe[j].value = outputSafeItem.value("type").toInt(0);
            } else {
                //use default value
                configUnis->outputsSafe[j].value = 0;
                configUnis->outputsSafe[j].type = 0;
            }
        }
        servoPosition = jsonConf.value("servoPosition").toArray();
        servoSpeed = jsonConf.value("servoSpeed").toArray();
        for(int j = 0; j < 6; j++) {
            if (j < servoSpeed.count()) {
                configUnis->servoSpeed[j] = servoSpeed[j].toInt(50);
            } else {
                configUnis->servoSpeed[j] = 50; // default value
            }
            configUnis->servoPosition[j].posA = servoPosition[j*2+0].toInt(70);
            configUnis->servoPosition[j].posB = servoPosition[j*2+1].toInt(110);
            /*
            if ((j*2) < servoPosition.count()) {
                configUnis->servoPosition[j*2] = servoSpeed[j].toInt(50);
            } else {
                configUnis->servoSpeed[j] = 50; // default value
            }
            */
        }
        ms->config = (TMtbModuleConfigGeneric *) configUnis;
        break;

    }
}

void configfile::savetofile(QString filename) {
    if (doc.isEmpty()) {
        // can't create new file
        return;
    }
    QJsonObject jsonObject = doc.object();
    // clear modules section
    if (jsonObject.contains("modules")){
        jsonObject.remove("modules");
    }

    // create new modules array
    QJsonObject docModules;
    for(int i = 0; i < modules.count(); i++) {
        QJsonObject docModule;
        QJsonObject docModuleConfig;
        QJsonArray inputsDelay;
        QJsonArray outputSafe;
        QJsonObject *outputSafeItem;
        QJsonArray servoPosition;
        QJsonArray servoSpeed;
        TMtbModuleConfigUNI *configUni;
        TMtbModuleConfigUNIS *configUnis;
        TMtbModuleState *ms;
        ms = &(modules[i]);
        switch (modules.at(i).type) {
        case 0x10: // UNI v2
            configUni = (TMtbModuleConfigUNI *) ms->config;
            for(int j = 0; j < 16; j++) {
                inputsDelay.append(QJsonValue(configUni->inputsDelay[j]));
            }
            for(int j = 0; j < 16; j++) {
                outputSafeItem = new QJsonObject;
                outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(configUnis->outputsSafe[j].type)));
                outputSafeItem->insert("value", QJsonValue(configUnis->outputsSafe[j].value));
                outputSafe.append(QJsonValue(*outputSafeItem));
            }
            docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
            docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object
            break;
        case 0x11 ... 0x21: // UNI v4
            configUni = (TMtbModuleConfigUNI *) ms->config;
            for(int j = 0; j < 16; j++) {
                inputsDelay.append(QJsonValue(configUni->inputsDelay[j]));
            }
            for(int j = 0; j < 16; j++) {
                outputSafeItem = new QJsonObject;
                outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(configUnis->outputsSafe[j].type)));
                outputSafeItem->insert("value", QJsonValue(configUnis->outputsSafe[j].value));
                outputSafe.append(QJsonValue(*outputSafeItem));
            }
            docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
            docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object
            break;
        case 0x50: // UNIS
            configUnis = (TMtbModuleConfigUNIS *) ms->config;
            for(int j = 0; j < 16; j++) {
                inputsDelay.append(QJsonValue(configUnis->inputsDelay[j]));
            }
            for(int j = 0; j < 16; j++) {
                outputSafeItem = new QJsonObject;
                outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(configUnis->outputsSafe[j].type)));
                outputSafeItem->insert("value", QJsonValue(configUnis->outputsSafe[j].value));
                outputSafe.append(QJsonValue(*outputSafeItem));
            }
            for(int j = 16; j < 28; j++) {
                outputSafeItem = new QJsonObject;
                outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(0)));
                outputSafeItem->insert("value", QJsonValue(0));
                outputSafe.append(QJsonValue(*outputSafeItem));
            }
            for(int j = 0; j < 6; j++) {
                servoPosition.append(configUnis->servoPosition[j].posA);
                servoPosition.append(configUnis->servoPosition[j].posB);
            }
            for(int j = 0; j < 6; j++) {
                servoSpeed.append(configUnis->servoSpeed[j]);
            }

            docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
            docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object
            docModuleConfig.insert("servoEnabledMask", QJsonValue(0)); // int
            docModuleConfig.insert("servoPosition", QJsonValue(servoPosition)); // array of int
            docModuleConfig.insert("servoSpeed", QJsonValue(servoSpeed)); // array of int
            break;
        }
        docModule.insert("config", QJsonValue(docModuleConfig));
        docModule.insert("name", QJsonValue(modules.at(i).name));
        docModule.insert("type", QJsonValue(modules.at(i).type));
        docModules.insert(QString("%1").arg(modules.at(i).address, 3, 10, QLatin1Char('0')), QJsonValue(docModule));
        //docModules.insert(, QJsonValue(docModuleItem));
    }
    jsonObject.insert("modules", QJsonValue(docModules));
    doc.setObject(jsonObject);

    // save to disk
    QFile fil;
    fil.setFileName(filename);
    fil.open(QIODevice::WriteOnly);
    fil.write(doc.toJson(QJsonDocument::Indented));
}

int configfile::findPosByAddress(int addr)
{
    int pos = -1;
    for(int i = 0; i < modules.count(); i++) {
        if (modules.at(i).address == addr) pos = i;
    }
    return (pos);
}

void configfile::setModuleName(int module, QString newname)
{
    int pos;
    if ((pos = findPosByAddress(module)) == -1) return;

    modules[pos].name = newname;
    emit moduleListChanged();
}

void configfile::setModuleType(int module, int type)
{
    int pos;
    if ((pos = findPosByAddress(module)) == -1) return;

    modules[pos].type = type;
    emit moduleListChanged();
}

void configfile::setModuleAddress(int oldAddress, int newAddress)
{
    int pos, pos2;
    pos = findPosByAddress(oldAddress);
    if (pos == -1) {
        // zdrojova adresa je volna, není co přesouvat
        return;
    } else {
        pos2 = findPosByAddress(newAddress);
        if (pos2 == -1) {
            //cílová adresa je volná, může me adresu změnit
            TMtbModuleState *ms;
            ms = &(modules[pos]);

            // find right place for new module
            for (int i = 0; i < modules.count(); i++) {
                if (newAddress < modules.at(i).address) {
                    modules.move(pos, i);
                    ms->address = newAddress;
                    emit moduleListChanged();
                    return;
                }
            }
            modules.move(pos, modules.count());
            ms->address = newAddress;
            emit moduleListChanged();
            return;
        } else {
            // cílová adresa je plná
            return;
        }
    }

}

void configfile::sendJson(QJsonObject json)
{
    doc.setObject(json);
}
