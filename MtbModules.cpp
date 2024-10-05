#include "MtbModules.h"

TMtbModuleState::TMtbModuleState(int _type)
{
    address = 0;
    type = _type;
    name = QString("bezjmena");
    active = false;
    warning = false;
    error = false;
    locator = false;
    switch (_type) {
    case 0x50:
        config = new TMtbModuleConfigUNIS();
        break;
    default:
        config = new TMtbModuleConfigUNI();
        break;
    }
}

const QStringList TMtbModuleTypes::names = {
    "UNI v2",
    "UNI v2 no IR",
    "UNI v4.0",
    "UNI v4.2",
    "BOOST",
    "MTB-RC",
    "MTB-LC",
    "UNIS"
};
const int TMtbModuleTypes::map[8] = {0x10, 0x11, 0x15, 0x16, 0x20, 0x30, 0x40, 0x50};

// output types:
const int TMtbModuleTypes::outputTypeMap[2] = {0, 1};
const int TMtbModuleTypes::OutputTypeCount = 2;
const QStringList TMtbModuleTypes::OutputTypeNames = {
    "plain",
    "s-com"
};

// input types:
const int TMtbModuleTypes::InputTypeMap[2] = {0, 1};
const int TMtbModuleTypes::InputTypeCount = 2;
const QStringList TMtbModuleTypes::InputTypeNames = {
    "plain",
    "ir"
};

/*****************************************************************************/
QJsonObject TMtbModuleConfigGeneric::getJson()
{
    return QJsonObject();
}

void TMtbModuleConfigGeneric::setJson(QJsonObject json)
{
    json;
}

/*****************************************************************************/
TMtbModuleConfigUNI::TMtbModuleConfigUNI()
{
    //this->mtbType = 0x10;
    for(int i = 0; i < 16; i++) {
        this->inputsDelay[i] = 0;
        this->inputsType[i] = 0;
        this->outputsSafe[i].type = 0;
        this->outputsSafe[i].value = 0;
    }
}
QJsonObject TMtbModuleConfigUNI::getJson()
{
    QJsonArray inputsDelay;
    QJsonArray inputsType;
    QJsonArray outputSafe;
    QJsonObject *outputSafeItem;
    QJsonObject docModuleConfig;

    for(int j = 0; j < 16; j++) {
        inputsDelay.append(QJsonValue(this->inputsDelay[j] / 10.0));
    }
    for(int j = 0; j < 16; j++) {
        outputSafeItem = new QJsonObject;
        outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(this->outputsSafe[j].type)));
        outputSafeItem->insert("value", QJsonValue(this->outputsSafe[j].value));
        outputSafe.append(QJsonValue(*outputSafeItem));
    }
    docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
    docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object

    return docModuleConfig;
}

/*****************************************************************************/
TMtbModuleConfigUNIS::TMtbModuleConfigUNIS()
{
    //this->mtbType = 0x50;
    for(int i = 0; i < 16; i++) {
        this->inputsDelay[i] = 0;
        this->outputsSafe[i].type = 0;
        this->outputsSafe[i].value = 0;
    }
    for(int i = 16; i < 28; i++) {
        this->outputsSafe[i].type = 0;
        this->outputsSafe[i].value = 0;
    }
    for(int i = 0; i < 6; i++) {
        this->servoSpeed[i] = 20;
        this->servoPosition[i].posA = 80;
        this->servoPosition[i].posB = 120;
    }
}

QJsonObject TMtbModuleConfigUNIS::getJson()
{
    QJsonArray inputsDelay;
    QJsonArray inputsType;
    QJsonArray outputSafe;
    QJsonObject *outputSafeItem;
    QJsonArray servoPosition;
    QJsonArray servoSpeed;
    QJsonObject docModuleConfig;

    for(int j = 0; j < 16; j++) {
        inputsDelay.append(QJsonValue(this->inputsDelay[j] / 10.0));
    }
    for(int j = 0; j < 16; j++) {
        outputSafeItem = new QJsonObject;
        outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(this->outputsSafe[j].type)));
        outputSafeItem->insert("value", QJsonValue(this->outputsSafe[j].value));
        outputSafe.append(QJsonValue(*outputSafeItem));
    }
    for(int j = 16; j < 28; j++) {
        outputSafeItem = new QJsonObject;
        outputSafeItem->insert("type",  QJsonValue(TMtbModuleTypes::ModuleUniOutputTypeGetName(0)));
        outputSafeItem->insert("value", QJsonValue(0));
        outputSafe.append(QJsonValue(*outputSafeItem));
    }
    for(int j = 0; j < 6; j++) {
        servoPosition.append(this->servoPosition[j].posA);
        servoPosition.append(this->servoPosition[j].posB);
    }
    for(int j = 0; j < 6; j++) {
        servoSpeed.append(this->servoSpeed[j]);
    }

    docModuleConfig.insert("inputsDelay", QJsonValue(inputsDelay)); // array of int
    docModuleConfig.insert("outputsSafe", QJsonValue(outputSafe)); // array of object
    docModuleConfig.insert("servoEnabledMask", QJsonValue(this->servoEnabledMask)); // int
    docModuleConfig.insert("servoPosition", QJsonValue(servoPosition)); // array of int
    docModuleConfig.insert("servoSpeed", QJsonValue(servoSpeed)); // array of int

    return docModuleConfig;
}

void TMtbModuleConfigUNIS::setJson(QJsonObject jsonConf)
{
    QJsonArray outputsSafe;
    QJsonArray inputsDelay;
    QJsonArray inputsType;
    QJsonArray servoPosition;
    QJsonArray servoSpeed;
    inputsDelay = jsonConf.value("inputsDelay").toArray();
    for(int j = 0; j < 16; j++) {
        if (j < inputsDelay.count()) {
            this->inputsDelay[j] = trunc(inputsDelay.at(j).toDouble(0.2) * 10.0);
        } else {
            //use default value
            this->inputsDelay[j] = 5;
        }
    }

    outputsSafe = jsonConf.value("outputsSafe").toArray();
    for(int j = 0; j < 28; j++) {
        if (j < outputsSafe.count()) {
            QJsonObject outputSafeItem = outputsSafe.at(j).toObject();
            QString outputSafeItemType = outputSafeItem.value("type").toString();
            int outputSafeItemTypeVal = TMtbModuleTypes::ModuleUniOutputTypeGetType(outputSafeItemType);
            this->outputsSafe[j].type = limit(outputSafeItemTypeVal, 0, 1);
            this->outputsSafe[j].value = limit(outputSafeItem.value("value").toInt(0), 0, 255);
        } else {
            //use default value
            this->outputsSafe[j].value = 0;
            this->outputsSafe[j].type = 0;
        }
    }
    servoPosition = jsonConf.value("servoPosition").toArray();
    servoSpeed = jsonConf.value("servoSpeed").toArray();
    for(int j = 0; j < 6; j++) {
        if (j < servoSpeed.count()) {
            this->servoSpeed[j] = limit(servoSpeed[j].toInt(20), 1, 255);
        } else {
            this->servoSpeed[j] = 20; // default value
        }
        if (((j*2)+1) < servoPosition.count()) {
            this->servoPosition[j].posA = limit(servoPosition[j*2+0].toInt(80), 0, 255);
            this->servoPosition[j].posB = limit(servoPosition[j*2+1].toInt(120), 0, 255);
        } else {
            this->servoPosition[j].posA = 80;
            this->servoPosition[j].posB = 120;
        }
    }
    this->servoEnabledMask = limit(jsonConf.value("servoEnabledMask").toInt(0), 0, 63);
}


/*****************************************************************************/
int TMtbModuleTypes::IndexToType(int index)
{
    if((index >= 0) && (index < count)) {
        return map[index];
    }
    return -1;
}

int TMtbModuleTypes::TypeToIndex(int type)
{
    for(int i = 0; i < count; i++) {
        if (map[i] == type) return i;
    }
    return 0;
}

QString TMtbModuleTypes::GetNameByIndex(int index)
{
    if((index >= 0) && (index < count)) {
        return names[index];
    }
    return QString("-");
}

QString TMtbModuleTypes::GetNameByType(int type)
{
    int index = TypeToIndex(type);
    if (index > -1) {
        return GetNameByIndex(index);
    }
    return QString("-");
}

QString TMtbModuleTypes::ModuleUniOutputTypeGetName(int type)
{
    int index = ModuleUniOutputTypeToIndex(type);
    if (index > -1) {
        return OutputTypeNames.at(index);
    }
    return QString("bad_type");
}

QString TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(int index)
{
    if ((index > -1) && (index < OutputTypeCount)) {
        return OutputTypeNames.at(index);
    }
    return QString("bad_index");
}

QString TMtbModuleTypes::ModuleUniInputTypeGetName(int type)
{
    int index = ModuleUniInputTypeToIndex(type);
    if ((index > -1)) {
        return InputTypeNames.at(index);
    }
    return QString("bad_type");
}

QString TMtbModuleTypes::ModuleUniInputTypeGetNameFromIndex(int index)
{
    //int index = ModuleUniInput
    if ((index > -1)) {
        return InputTypeNames.at(index);
    }
    return QString("bad_index");
}


int TMtbModuleTypes::ModuleUniOutputTypeGetType(QString typeName)
{
    for(int i = 0; i < OutputTypeCount; i++) {
        if (OutputTypeNames.at(i) == typeName) return i;
    }
    return 0;
}

int TMtbModuleTypes::ModuleUniInputTypeGetType(QString typeName)
{
    for(int i = 0; i < InputTypeCount; i++) {
        if (InputTypeNames.at(i) == typeName) return i;
    }
    return 0;
}

int TMtbModuleTypes::ModuleUniOutputTypeToIndex(int type)
{
    for(int i = 0; i < OutputTypeCount; i++) {
        if (outputTypeMap[i] == type) {
            return i;
        }
    }
    return -1;
}

int TMtbModuleTypes::ModuleUniOutputIndexToType(int index)
{
    if (index < OutputTypeCount) {
        return outputTypeMap[index];
    } else {
        return -1;
    }
}

int TMtbModuleTypes::ModuleUniInputTypeToIndex(int type)
{
    for(int i = 0; i < InputTypeCount; i++) {
        if (InputTypeMap[i] == type) {
            return i;
        }
    }
    return -1;
}

int TMtbModuleTypes::ModuleUniInputIndexToType(int index)
{
    if (index < InputTypeCount) {
        return InputTypeMap[index];
    } else {
        return -1;
    }
}

int TMtbModuleConfigGeneric::limit(int val, int min, int max)
{
    if (val < min) val=min;
    if (val > max) val=max;
    return val;
}

float TMtbModuleConfigGeneric::limit(float val, float min, float max)
{
    if (val < min) val=min;
    if (val > max) val=max;
    return val;
}
