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

class configfile : public QObject
{
    Q_OBJECT
public:
    explicit configfile(QObject *parent = nullptr);

    void loadfromfile(QString filename);
    void savetofile(QString filename);
	bool isModuleInConfig(uint8_t addr);
    void subscribeModule(int module);
    void setServoOuts(int module, int servo, int state);

    QList<TMtbModuleState> modules;
private:
    QJsonDocument doc;
    int id;
    void sendJson(QJsonObject json);
};

#endif // CONFIGFILE_H
