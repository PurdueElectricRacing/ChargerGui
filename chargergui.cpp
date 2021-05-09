#include "chargergui.h"
#include "./ui_chargergui.h"

ChargerGui::ChargerGui(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChargerGui)
{
    ui->setupUi(this);
}

ChargerGui::~ChargerGui()
{
    delete ui;
}

