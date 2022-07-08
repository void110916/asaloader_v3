#ifndef LOADER_H
#define LOADER_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include <functional>

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
  virtual void _get_packet(Command &cmd_out, uint8_t *data_out);
  virtual void _get_packet(Command &cmd_out, char *data_out);
  virtual int _put_packet(Command cmd, const uint8_t *data);
  virtual int _put_packet(Command cmd, const char *data);
  virtual bool _base_cmd(Command cmd,
                         int (*cmp_func)(const void *, const void *));

 public:
  Decoder *decoder;

  CMD(QSerialPort &serial);
  ~CMD();

  int8_t cmd_chk_protocol();

  bool cmd_v1_enter_prog();
  bool cmd_v1_flash_write(uint8_t *data);
  bool cmd_v1_prog_end();

  bool cmd_v2_enter_prog();
  int cmd_v2_prog_chk_device();
  bool cmd_v2_prog_end();
  bool cmd_v2_prog_end_and_go_app();
  bool cmd_v2_prog_set_go_app_delay();
  bool cmd_v2_flash_set_pgsz(int size);
  int cmd_v2_flash_get_pgsz();
  bool cmd_v2_flash_write(uint32_t page_addr, uint8_t *data);
  uint8_t *cmd_v2_flash_read();
  int cmd_v2_flash_earse_sector(uint16_t num);
  bool cmd_v2_flash_earse_all();

  bool cmd_v2_ext_flash_write(uint32_t page_addr, uint8_t *data);
  virtual bool cmd_v2_ext_flash_read();
  bool cmd_v2_prog_ext_to_int();
  bool cmd_v2_eep_set_pgsz(uint32_t size);
  bool cmd_v2_eep_get_pgsz();
  int cmd_v2_eep_write(uint8_t *page_data);
  int cmd_v2_eep_read();
  int cmd_v2_eep_earse();
  bool cmd_v2_eep_earse_all();

  bool cmd_v3_flash_earse_all();
  bool cmd_v3_ext_flash_prepare(const char *);
  bool cmd_v3_ext_flash_hex_delete();
  bool cmd_v3_ext_flash_finish();
};

}  // namespace Loader
QT_END_NAMESPACE
#endif  // LOADER_H