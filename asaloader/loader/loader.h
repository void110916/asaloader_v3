#ifndef LOADER_H
#define LOADER_H
#include <QSerialPort>
#include <QSerialPortInfo>

#include "protcol.h"
QT_BEGIN_NAMESPACE
namespace Loader {

class Loader {
 private:
  QSerialPort &_serial;
  Decoder *_decoder;

 public:
  enum _Stage {
    PREPARE,
    FLASH_PROG,
    EEP_PROG,
    EXT_FLASH_PROG,
    EXT_TO_INT,
    END
  };
  _Stage _stage;

  Loader(QSerialPort &serial);
  ~Loader();
};

Loader::Loader(QSerialPort &serial) : _serial(serial) {
  _decoder = new Decoder();
  _stage = PREPARE;
}

Loader::~Loader() { delete (_decoder); }

class CMD {
 private:
  const uint8_t HEADER[3] = {0xfc, 0xfc, 0xfc};
  QSerialPort &_serial;
  virtual uint8_t *_get_packet();
  virtual void _put_packet(enum Command,uint8_t* data);

 public:
  Decoder *decoder;

  CMD(QSerialPort &serial);
  ~CMD();

  bool cmd_chk_protocol();
};

CMD::CMD(QSerialPort &serial) : _serial(serial) { decoder = new Decoder(); }

CMD::~CMD() { delete (decoder); }

}  // namespace Loader
QT_END_NAMESPACE
#endif  // LOADER_H