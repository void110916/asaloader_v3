#include <QCoreApplication>
#include <QLocale>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTranslator>
#include <iostream>

#include "asadevice.h"
#include "loader.h"
#include "prog_arg.h"

void processbar_step(float progress);
void errorHandle(QSerialPort::SerialPortError error);

// template <typename... Args>
// std::string string_format(const std::string &format, Args... args);
QTextStream qout(stdout);
int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages)
  {
    const QString baseName = "asaloader_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName))
    {
      a.installTranslator(&translator);
      // QCoreApplication::installTranslator(&translator);
      break;
    }
  }
  auto arg = a.arguments();
  for (QList<QString>::const_iterator str = arg.constBegin() + 1;
       str != arg.constEnd(); ++str)
  {
    auto s = (*str).toStdString();
    if (*str == u"pd"_qs || *str == u"print-devices"_qs)
    {
      qout << QObject::tr("Available device list:") << "\n";
      qout << QObject::tr("    device name   \t num \t note") << "\n";
      for (auto dev : Loader::asa_dev_list)
        qout << QObject::tr("  - %1  \t %2\t %3")
                    .arg(dev.name, 11)
                    .arg(dev.dev_type)
                    .arg(dev.note)
             << "\n";
      qout.flush();
    }
    else if (*str == u"pp"_qs || *str == u"print-ports"_qs)
    {
      for (auto &&serial : QSerialPortInfo::availablePorts())
      {
        qout << QString("%1    desc: %2    hwid: %3")
                    .arg(serial.portName(), 20)
                    .arg(serial.description(), 15)
                    .arg(serial.vendorIdentifier())
             << "\n";
      }
      qout.flush();
      // a.exit();
    }
    else if (*str == u"prog"_qs)
    {
      {
        Loader::prog_args_t prog_args(arg);
        QSerialPort port;
        port.setPortName(prog_args.portName);
        port.setBaudRate(QSerialPort::Baud38400);
        port.setParity(QSerialPort::NoParity);
        port.setDataBits(QSerialPort::Data8);

        port.setStopBits(QSerialPort::OneStop);
        port.setFlowControl(QSerialPort::NoFlowControl);
        port.setReadBufferSize(500);
        port.open(QSerialPort::ReadWrite);

        port.setDataTerminalReady(true);
        port.setRequestToSend(true);
        port.clear();

        Loader::Loader l(port, prog_args.dev_type, prog_args.is_flash_prog,
                         prog_args.is_ext_flash_prog, prog_args.is_eeprom_prog,
                         prog_args.is_ext2int, prog_args.is_go_app,
                         prog_args.flash_file, prog_args.ext_flash_file,
                         prog_args.eep_file, prog_args.go_app_delay);
        qout << QObject::tr("Device is ") << QString("\"%1\"").arg(Loader::asa_dev_list[l.device_type()].name)
             << "\n";
        qout << QObject::tr("Flash hex size is %1 KB (%2 bytes)")
                    .arg((float)l.flash_size() / 1024, 4)
                    .arg(l.flash_size())
             << "\n";

        qout << QObject::tr("Externel Flash hex size is %1 KB (%2 bytes)")
                    .arg((float)l.ext_flash_size() / 1024, 4)
                    .arg(l.ext_flash_size())
             << "\n";

        qout << QObject::tr("EEPROM hex size is %1 bytes.").arg(l.eep_size())
             << "\n";

        qout << QObject::tr("Estimated time is %1 s.").arg(l.prog_time())
             << "\n";

        qout.flush();
        for (int i = 0; i < l.total_steps(); ++i)
        {
          l.do_step();
          processbar_step((float)(i + 1) / l.total_steps());
        }
        break;
      }
    }
    else
      ; // TODO: help info
  }
 

  return 0;
}

void processbar_step(float progress)
{
  // static float progress = 0.0;
  // while (progress < 1.0) {
  int barWidth = 70;

  qout << "[";
  int pos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i)
  {
    if (i < pos)
      qout << "=";
    else if (i == pos)
      qout << ">";
    else
      qout << " ";
  }
  qout << "] " << int(progress * 100.0) << " %\r";
  qout.flush();

  // progress += 0.16;  // for demonstration only
  // }
  // std::cout << std::endl;
}

void errorHandle(QSerialPort::SerialPortError error)
{
  std::cout << "Error: " << error << std::endl;
}
