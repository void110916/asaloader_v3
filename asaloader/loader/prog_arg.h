#ifndef PROG_ARG_H
#define PROG_ARG_H
#include <QFileInfo>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <string>

#include "asadevice.h"

namespace Loader
{
  class prog_args_t
  {
  public:
    QString flash_file;
    QString eep_file;
    QString ext_flash_file;
    bool is_flash_prog = false;
    bool is_ext_flash_prog = false;
    bool is_eeprom_prog = false;
    bool is_ext2int = false;
    bool is_go_app = false;
    int dev_type=0;
    int go_app_delay = 0;
    QString portName;
    prog_args_t(QStringList arg);
    ~prog_args_t();
  };

  prog_args_t::prog_args_t(QStringList arg)
  {
    for (QList<QString>::const_iterator str = arg.constBegin() + 2;
         str != arg.constEnd(); ++str)
    {
      if (*str == u"-d"_qs || *str == u"--device"_qs)
      {
        bool isexit = true;
        for (auto &&dev : asa_dev_list)
          if ((*++str).toInt() == dev.dev_type)
          {
            dev_type = dev.dev_type;
            isexit = false;
            break;
          }
        if (isexit)
        {
          std::cout << QObject::tr("No Device detect").toStdString() << std::endl;
          exit(1);
        } //  TODO: exception
      }
      else if (*str == u"-p"_qs || *str == u"--port"_qs)
      {
        bool isexit = true;
        for (auto &&port : QSerialPortInfo::availablePorts())
          if (*++str == port.portName())
          {
            portName = port.portName();
            isexit = false;
            break;
          }
        if (isexit)
        {
          std::cout << QObject::tr("No Device detect").toStdString() << std::endl;
          exit(1);
        } //  TODO: error message
      }
      else if (*str == u"-f"_qs || *str == u"--flash"_qs)
      {
        QFileInfo f{*(++str)};
        if (f.isFile() && f.suffix() == "hex")
        {
          is_flash_prog = true;
          flash_file = f.absoluteFilePath();
        }
        else
        {
          std::cout << QObject::tr("No File exit").toStdString() << std::endl;
          exit(1);
        }
      }
      else if (*str == u"-E"_qs || *str == u"--extflash"_qs)
      {
        QFileInfo f{*(++str)};
        if (f.isExecutable() && f.suffix() == "hex")
        {
          is_ext_flash_prog = true;
          flash_file = f.absoluteFilePath();
        }
        else
          exit(1); //  TODO: error message
      }
      else if (*str == u"-i"_qs || *str == u"--ext2int"_qs)
        is_ext2int = true;
      else if (*str == u"-e"_qs || *str == u"--eeprom"_qs)
      {
        QFileInfo f{*(++str)};
        if (f.isExecutable() && f.suffix() == "hex")
        {
          is_eeprom_prog = true;
          flash_file = f.absoluteFilePath();
        }
        else
          exit(1);
      }
      else if (*str == u"-a"_qs || *str == u"--after-prog-go-app")
        is_go_app = true;
      else if (*str == u"-D"_qs || *str == u"--go-app-delay")
        go_app_delay = (*++str).toInt();
    }
  }

  prog_args_t::~prog_args_t() {}

} // namespace Loader
#endif
