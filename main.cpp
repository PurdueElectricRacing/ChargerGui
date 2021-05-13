#include "chargergui.h"

#include <QApplication>
#include <QThread>
#include <QComboBox>
#include <QObject>
#include <QCommandLinkButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <string>
#include <thread>
#include <exception>

#include "can_api.h"

#define ELCON_WRITE_ADDR 0x1806E5F4
#define ELCON_READ_ADDR  0x18FF50E5

#define GREAT 1
#define PER GREAT

void chargeButtonPressed();

struct GuiItems
{
  ChargerGui * gui;
  CanInterface * can_if;
  QComboBox * device_box;
  QComboBox * baud_combobox;
  QCommandLinkButton * connect_button;
  QCommandLinkButton * refresh_button;
  QCommandLinkButton * charge_button;

  QDoubleSpinBox * request_current;
  QDoubleSpinBox * target_voltage;

  QLabel * voltage_label;
  QLabel * current_label;
  QThread * reader_thread;
  QThread * charger_thread;
  std::mutex can_mtx;
  bool terminate = false;


  void init(ChargerGui &w);
  virtual ~GuiItems() {
    if (can_if)
    {
      terminate = true;
      gui = 0;
      device_box = 0;
      baud_combobox = 0;
      connect_button = 0;
      refresh_button = 0;
      charge_button = 0;
      request_current = 0;
      target_voltage = 0;
      voltage_label = 0;
      current_label = 0;

      can_mtx.lock();
      can_if->Close();
      can_if = 0;
      can_mtx.unlock();
      
      dev_open = false;
      charge_enable = false;
    }
  }
  
  bool charge_enable = false;
  bool dev_open = false;
} gui_items;



int getBaudRate(int baud_index)
{
  int baud = 500000;

  switch (baud_index)
  {
    // 1mbps
    case 0:
      baud = 1000000;
      break;
    // 500kbps
    case 1:
      baud = 500000;
      break;
    // 250kbps
    case 2:
      baud = 250000;
      break;
    // 125kbps
    case 3:
      baud = 125000;
      break;
  }
  return baud;
}


/// @brief: Slot for handling when the baud rate is changed
///         Disconnects the CAN device and resets the GUI
void baudRateChanged(int idx)
{
  CanInterface * can_if = gui_items.can_if;

  can_if->setBaudRate(getBaudRate(idx));
}



/// @brief: thread that reads the CAN bus and displays the values to the GUI
///         if we get a message from the address we care about
void readTheCanBusYo()
{
  while (!gui_items.terminate)
  {
    CanInterface * can_if = gui_items.can_if;
    while (gui_items.dev_open)
    {
      double actual_current = 0;
      double actual_voltage = 0;
      CanFrame rx_data;

      gui_items.can_mtx.lock();
      if (can_if)
      {
        rx_data = can_if->readCanData();
      }
      gui_items.can_mtx.unlock();

      uint32_t id = rx_data.getId();
      if (id == ELCON_READ_ADDR)
      {
        actual_voltage = ((double) ((rx_data.data[0] << 8)
                         | (rx_data.data[1]))) / 10;
        actual_current = ((double) ((rx_data.data[2] << 8)
                         | (rx_data.data[3]))) / 10;
        if (!gui_items.terminate)
        {
          gui_items.voltage_label->setText(QString().setNum(actual_voltage));
          gui_items.current_label->setText(QString().setNum(actual_current));
        }
      }
    }
  }
}



/// @brief: Thread that sends the charge command
///         Will send charge enable = 1 when the start charging 
///         button is clicked, 0 otherwise.
void elconsChargeTheCarYo()
{
  CanInterface * can_if = gui_items.can_if;
  uint8_t to_send[5];

  while (!gui_items.terminate)
  {
    if (gui_items.charge_enable && gui_items.dev_open)
    {
      uint16_t target_voltage = 0;
      uint16_t request_current = 0;
      // don't access these pointers anymore pls
      if (!gui_items.terminate)
      {
        request_current = gui_items.request_current->value() * 10;
        target_voltage = gui_items.target_voltage->value() * 10;
      }

      to_send[0] = target_voltage >> 8;
      to_send[1] = target_voltage;
      to_send[2] = request_current >> 8;
      to_send[3] = request_current;
      to_send[4] = 0;

      if (can_if)
      {
        can_if->writeCanData(ELCON_WRITE_ADDR, 5, to_send);
      }
    }
    else if (gui_items.dev_open)
    {
      to_send[0] = 0;
      to_send[1] = 0;
      to_send[2] = 0;
      to_send[3] = 0;
      to_send[4] = 1;
      if (can_if)
      {
        can_if->writeCanData(ELCON_WRITE_ADDR, 5, to_send);
      }
    }
    // delay 1 second
    QThread::currentThread()->msleep(1000);
  }
    
}




/// @brief: slot for when the connect button is pressed
void connectButtonPressed()
{
  CanInterface * can = gui_items.can_if;

  if (!gui_items.dev_open)
  {
    int baud_idx = gui_items.baud_combobox->currentIndex();
    int dev_index = gui_items.device_box->currentIndex();
    int baud_rate = getBaudRate(baud_idx);

    gui_items.can_mtx.lock();
    can->Open(dev_index, baud_rate);
    gui_items.can_mtx.unlock();

    gui_items.dev_open = true;
    gui_items.connect_button->setText("Disconnect");
    gui_items.connect_button->setStyleSheet("color:rgb(170,0,0);");
    gui_items.charge_button->setEnabled(true);
  }
  else
  {
    gui_items.connect_button->setText("Connect");
    gui_items.dev_open = false;
    gui_items.charge_enable = false;
    gui_items.connect_button->setStyleSheet("color:rgb(0,85,0);");
    
    gui_items.can_mtx.lock();
    can->Close();
    gui_items.can_mtx.unlock();
    gui_items.charge_button->setEnabled(false);

    // toggle back to start charging
    if (gui_items.charge_button->text() == "Stop Charging")
    {
        chargeButtonPressed();
    }
  }
  
}



/// @brief: Slot that disconnects the CAN device if one is connected and the 
///         device combobox changes indexes
void deviceIndexChanged(int idx)
{
  if (gui_items.dev_open)
  {
    connectButtonPressed();
  }
}


/// @brief: slot for when the charge_enable button is clicked
void chargeButtonPressed()
{
  if (gui_items.charge_button->text() == "Start Charging")
  {
    gui_items.charge_enable = true;
    gui_items.charge_button->setText("Stop Charging");
    gui_items.charge_button->setStyleSheet("color:rgb(170,0,0);");
    
  }
  else
  {
    gui_items.charge_button->setText("Start Charging");
    gui_items.charge_enable = false;
    gui_items.charge_button->setStyleSheet("color:rgb(0,85,0);");
  }
}




/// @brief: Slot that updates the device list when the refresh button is clicked
void refreshButtonPressed()
{
  auto dev_list = gui_items.can_if->getDevices();
  QComboBox * dev_combobox = gui_items.device_box;

  // insert the devices into the device list.
  for (auto dev = dev_list.begin(); dev != dev_list.end(); dev++)
  {
    std::string name = *dev;
    QString devname = name.c_str();

    // don't add duplicates to the list
    if (dev_combobox->findText(devname) == -1)
    {
      dev_combobox->addItem(name.c_str());
    }
  }
}




void GuiItems::init(ChargerGui &w)
{
  gui_items.gui = &w;
  baud_combobox = w.findChild<QComboBox *>("BaudRateBox");
  device_box = w.findChild<QComboBox *>("CanDevices");
  connect_button = w.findChild<QCommandLinkButton *>("CanConnectButton");
  charge_button = w.findChild<QCommandLinkButton *>("ChargeButton");
  refresh_button = w.findChild<QCommandLinkButton *>("RefreshButton");

  // set the button text to green
  connect_button->setStyleSheet("color:rgb(0,85,0);");
  charge_button->setEnabled(false);

  charger_thread = QThread::create(elconsChargeTheCarYo);
  reader_thread = QThread::create(readTheCanBusYo);

  // I guess findChild doesn't check more than 1 level of recursion
  QWidget * subframe = w.findChild<QWidget *>("widget");
  voltage_label = subframe->findChild<QLabel*>("VoltageLabel");
  current_label = subframe->findChild<QLabel*>("CurrentLabel");
  target_voltage = w.findChild<QDoubleSpinBox *>("TargetVoltageBox");
  request_current = w.findChild<QDoubleSpinBox *>("TargetCurrentBox");
  int baud = getBaudRate(baud_combobox->currentIndex());
  can_if = NewCanDevice(baud);

  QObject::connect(baud_combobox, 
                   QOverload<int>::of(&QComboBox::currentIndexChanged), 
                   baudRateChanged);
  QObject::connect(connect_button, &QCommandLinkButton::clicked, 
                   connectButtonPressed);
  QObject::connect(charge_button, &QCommandLinkButton::clicked,     
                   chargeButtonPressed); 
  QObject::connect(refresh_button, &QCommandLinkButton::clicked,
                   refreshButtonPressed);
  QObject::connect(device_box, 
                   QOverload<int>::of(&QComboBox::currentIndexChanged),
                   deviceIndexChanged);

  refreshButtonPressed();
  charger_thread->start();
  reader_thread->start();
}


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  ChargerGui w;
  gui_items.init(w);

  w.show();
  return a.exec();
}
