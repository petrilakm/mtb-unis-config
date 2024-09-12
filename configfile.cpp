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
    QJsonArray inputsType;
    QJsonArray servoPosition;
    QJsonArray servoSpeed;
    TMtbModuleConfigUNI *configUni = NULL;
    TMtbModuleConfigUNIS *configUnis = NULL;
    switch (ms->type) {
    case 0x10: // UNI with IR
        configUni = new TMtbModuleConfigUNI();
        inputsType = jsonConf.value("irs").toArray();
        for(int j = 0; j < 16; j++) {
            if (j < inputsType.count()) {
                configUni->inputsType[j] = limit(inputsType.at(j).toInt(0), 0, 1);
            } else {
                //use default value
                configUni->inputsDelay[j] = 20;
            }
        }
        [[fallthrough]];
    case 0x11 ... 0x21: // UNI (no IR)
        if (!configUni) configUni = new TMtbModuleConfigUNI();
        //configUni->mtbType = ms->type;
        inputsDelay = jsonConf.value("inputsDelay").toArray();
        for(int j = 0; j < 16; j++) {
            if (j < inputsDelay.count()) {
                configUni->inputsDelay[j] = limit(trunc(inputsDelay.at(j).toDouble(0.2) * 10.0), 0, 9);
            } else {
                //use default value
                configUni->inputsDelay[j] = 20;
            }
        }

        outputsSafe = jsonConf.value("outputsSafe").toArray();
        for(int j = 0; j < 16; j++) {
            if (j < outputsSafe.count()) {
                QJsonObject outputSafeItem = outputsSafe.at(j).toObject();
                QString outputSafeItemType = outputSafeItem.value("type").toString();
                int outputSafeItemValue = outputSafeItem.value("value").toInt(0);
                int outputSafeItemTypeVal = TMtbModuleTypes::ModuleUniOutputTypeGetType(outputSafeItemType);
                configUni->outputsSafe[j].type = limit(outputSafeItemTypeVal, 0, 1);
                configUni->outputsSafe[j].value = limit(outputSafeItemValue, 0, 255);

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
                configUnis->inputsDelay[j] = trunc(inputsDelay.at(j).toDouble(0.2) * 10.0);
            } else {
                //use default value
                configUnis->inputsDelay[j] = 20;
            }
        }

        outputsSafe = jsonConf.value("outputsSafe").toArray();
        for(int j = 0; j < 28; j++) {
            if (j < outputsSafe.count()) {
                QJsonObject outputSafeItem = outputsSafe.at(j).toObject();
                QString outputSafeItemType = outputSafeItem.value("type").toString();
                int outputSafeItemTypeVal = TMtbModuleTypes::ModuleUniOutputTypeGetType(outputSafeItemType);
                configUnis->outputsSafe[j].type = limit(outputSafeItemTypeVal, 0, 1);
                configUnis->outputsSafe[j].value = limit(outputSafeItem.value("value").toInt(0), 0, 255);
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
                configUnis->servoSpeed[j] = limit(servoSpeed[j].toInt(50), 0, 255);
            } else {
                configUnis->servoSpeed[j] = 50; // default value
            }
            configUnis->servoPosition[j].posA = limit(servoPosition[j*2+0].toInt(70), 0, 255);
            configUnis->servoPosition[j].posB = limit(servoPosition[j*2+1].toInt(110), 0, 255);
        }
        configUnis->servoEnabledMask = limit(jsonConf.value("servoEnabledMask").toInt(0), 0, 63);
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
        QJsonArray inputsType;
        QJsonArray outputSafe;
        QJsonObject *outputSafeItem;
        QJsonArray servoPosition;
        QJsonArray servoSpeed;
        TMtbModuleConfigUNI *configUni;
        TMtbModuleConfigUNIS *configUnis;
        TMtbModuleState *ms;
        ms = &(modules[i]);
        switch (modules.at(i).type) {
        case 0x10: // UNI v2 with IR
            configUni = (TMtbModuleConfigUNI *) ms->config;
            for(int j = 0; j < 16; j++) {
                inputsType.append(QJsonValue(configUni->inputsType[j]));
            }
            /*
            for(int j = 0; j < 16; j++) {
                outputSafeItem = new QJsonObject;
                outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(configUni->outputsSafe[j].type)));
                outputSafeItem->insert("value", QJsonValue(configUni->outputsSafe[j].value));
                outputSafe.append(QJsonValue(*outputSafeItem));
            }
            */
            //docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
            docModuleConfig.insert("irs", QJsonValue(inputsType)); // array of int
            //docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object
            //break;
            [[fallthrough]];
        case 0x11 ... 0x21: // UNI (no IR)
            configUni = (TMtbModuleConfigUNI *) ms->config;
            for(int j = 0; j < 16; j++) {
                inputsDelay.append(QJsonValue(configUni->inputsDelay[j] / 10.0));
            }
            for(int j = 0; j < 16; j++) {
                outputSafeItem = new QJsonObject;
                outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(configUni->outputsSafe[j].type)));
                outputSafeItem->insert("value", QJsonValue(configUni->outputsSafe[j].value));
                outputSafe.append(QJsonValue(*outputSafeItem));
            }
            docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
            docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object
            break;
        case 0x50: // UNIS
            configUnis = (TMtbModuleConfigUNIS *) ms->config;
            for(int j = 0; j < 16; j++) {
                inputsDelay.append(QJsonValue(configUnis->inputsDelay[j] / 10.0));
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
            docModuleConfig.insert("servoEnabledMask", QJsonValue(configUnis->servoEnabledMask)); // int
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
    if (modules[pos].config) delete modules[pos].config;
    switch (type) {
    case 0x10:
    case 0x11:
    case 0x20:
    case 0x21:
        modules[pos].config = new TMtbModuleConfigUNI();
        break;
    case 0x50:
        modules[pos].config = new TMtbModuleConfigUNIS();
        break;
    default:
        modules[pos].config = new TMtbModuleConfigGeneric();
    }
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


int configfile::limit(int val, int min, int max)
{
    if (val < min) val=min;
    if (val > max) val=max;
    return val;
}

float configfile::limit(float val, float min, float max)
{
    if (val < min) val=min;
    if (val > max) val=max;
    return val;
}
