#ifndef WINLIST_H
#define WINLIST_H

#include <QDialog>
#include <QStandardItemModel>
#include <QList>
#include "MtbModules.h"

QT_BEGIN_NAMESPACE
namespace Ui { class WinList; }
QT_END_NAMESPACE

class WinList : public QDialog
{
    Q_OBJECT

public:
    explicit WinList(QWidget *parent = nullptr);
    ~WinList();

private:
    Ui::WinList *ui;
    TMtbModuleConfigGeneric *actcfg;
};

#endif // WINLIST_H
