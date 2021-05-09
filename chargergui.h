#ifndef CHARGERGUI_H
#define CHARGERGUI_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ChargerGui; }
QT_END_NAMESPACE

class ChargerGui : public QMainWindow
{
    Q_OBJECT

public:
    ChargerGui(QWidget *parent = nullptr);
    ~ChargerGui();

private:
    Ui::ChargerGui *ui;
};
#endif // CHARGERGUI_H
