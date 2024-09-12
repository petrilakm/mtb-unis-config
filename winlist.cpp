#include "winlist.h"
#include "ui_winlist.h"

WinList::WinList(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WinList)
{
    ui->setupUi(this);
}

WinList::~WinList()
{
    delete ui;
}
