#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "MtbModules.h"
#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QList>

class configfile : public QObject
{
    Q_OBJECT
public:
    explicit configfile(QObject *parent = nullptr);

    void loadfromfile(QString filename);
    void savetofile(QString filename);
    void parseOneConfig(int pos, QJsonObject jsonConf);
	bool isModuleInConfig(uint8_t addr);
    void setServoOuts(int module, int servo, int state);
    QList<TMtbModuleState> modules;
    int findPosByAddress(int addr);

    void setModuleName(int module, QString newname);
    void setModuleType(int module, int type);
    void setModuleAddress(int oldAddress, int newAddress);
private:
    QJsonDocument doc;
    int id;
    //TMtbModuleTypes mtbModuleTypes;
    void sendJson(QJsonObject json);

    int limit(int val, int min, int max);
    float limit(float val, float min, float max);

signals:
    void moduleListChanged();
    void moduleChanged(int pos);


};

#endif // CONFIGFILE_H
