#ifndef LOADER_H
#define LOADER_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include <functional>
#include <string>
#include <vector>

#include "ihex.h"
#include "protcol.h"
QT_BEGIN_NAMESPACE
namespace Loader {
class CMD {
 private:
  const uint8_t HEADER[3] = {0xfc, 0xfc, 0xfc};
  QSerialPort &_serial;
  virtual void _get_packet(Command &cmd_out, uint8_t *data_out);
  virtual void _get_packet(Command &cmd_out, char *data_out);
  virtual int _put_packet(Command cmd, const uint8_t *data);
  virtual int _put_packet(Command cmd, const char *data);
  virtual bool _base_cmd(Command cmd, const char *pac = "");
  virtual bool _base_cmd(Command cmd,
                         int (*cmp_func)(const char *, const char *, size_t s),
                         const char *pac = "");
  virtual char *_advance_cmd(Command cmd, const char *pac = "");
  // virtual bool _advence_cmd(Command cmd, uint8_t *pac);

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
  bool cmd_v2_prog_set_go_app_delay(uint16_t time);
  bool cmd_v2_flash_set_pgsz(int32_t size);
  int cmd_v2_flash_get_pgsz();
  bool cmd_v2_flash_write(uint32_t page_addr, uint8_t *data);
  char *cmd_v2_flash_read();
  uint32_t cmd_v2_flash_earse_sector(uint16_t num);
  bool cmd_v2_flash_earse_all();

  bool cmd_v2_ext_flash_write(uint32_t page_addr, uint8_t *data);
  virtual bool cmd_v2_ext_flash_read();
  bool cmd_v2_prog_ext_to_int();
  bool cmd_v2_eep_set_pgsz(uint32_t size);
  uint16_t cmd_v2_eep_get_pgsz();
  uint32_t cmd_v2_eep_write(uint8_t *page_data);
  uint32_t cmd_v2_eep_read();
  uint32_t cmd_v2_eep_earse();
  bool cmd_v2_eep_earse_all();

  bool cmd_v3_flash_earse_all();
  bool cmd_v3_ext_flash_prepare(const char *);
  bool cmd_v3_ext_flash_hex_delete();
  bool cmd_v3_ext_flash_finish();
};
class Loader {
 private:
  enum Stage { PREPARE, FLASH_PROG, EEP_PROG, EXT_FLASH_PROG, EXT_TO_INT, END };

  QSerialPort &_serial;
  Stage _stage;
  uint16_t _total_steps = 0;
  uint16_t _cur_step = 0;
  std::vector<char *> _flash_pages;
  uint16_t _flash_page_idx;
  std::vector<char *> _ext_flash_pages;
  uint16_t _ext_flash_page_idx;
  std::vector<char *> _eep_pages;
  uint16_t _eep_page_idx;

  uint16_t _flash_size = 0;
  uint16_t _ext_flash_size = 0;
  uint16_t _eep_size = 0;
  float _prog_time = 0.f;

  int _device_type;
  int _protocol_version;
  const char *_device_name;
  bool _is_flash_prog;
  bool _is_ext_flash_prog;
  bool _is_eeprom_prog;
  bool _is_ext_to_int;
  bool _is_go_app;
  std::string _flash_file;
  std::string _ext_flash_file;
  std::string _eep_file;
  uint16_t _go_app_delay;

  CMD cmd;

  void _prepare();
  void _prepare_device();
  void _prepare_flash();
  void _prepare_ext_flash();
  void _prepare_eeprom();
  void _do_flash_prog_step();
  void _do_ext_flash_prog_step();
  void _do_ext_to_int_prog_step();
  void _do_eep_prog_step();
  void _do_prog_end_step();

 public:
  Loader(QSerialPort &serial, int device_type = 0, bool is_flash_prog = false,
         bool is_ext_flash_prog = false, bool is_eeprom_prog = false,
         bool is_ext_to_int = false, bool is_go_app = false,
         std::string flash_file = "", std::string ext_flash_file = "",
         std::string eeprom_file = "", int go_app_delay = 0);
  ~Loader();
  void do_step();
};

}  // namespace Loader
QT_END_NAMESPACE
#endif  // LOADER_H