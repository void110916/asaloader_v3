#include "loader.h"

#include <endian.h>
#include <time.h>

#include <filesystem>
#include <iostream>

#include "device.h"

#define length(ptr) sizeof(ptr) / sizeof((ptr)[0])

////    Loader start
namespace Loader {
Loader::Loader(QSerialPort &serial, int device_type, bool is_flash_prog,
               bool is_ext_flash_prog, bool is_eeprom_prog, bool is_ext_to_int,
               bool is_go_app, std::string flash_file,
               std::string ext_flash_file, std::string eeprom_file,
               int go_app_delay)
    : _serial(serial),
      cmd(serial),
      _device_type(device_type),
      _is_flash_prog(is_flash_prog),
      _is_ext_flash_prog(is_ext_flash_prog),
      _is_eeprom_prog(is_eeprom_prog),
      _is_ext_to_int(is_ext_to_int),
      _is_go_app(is_go_app),
      _flash_file(flash_file),
      _ext_flash_file(ext_flash_file),
      _eep_file(eeprom_file),
      _go_app_delay(go_app_delay),
      _stage(Stage::PREPARE) {
  _prepare();
}

Loader::~Loader() {}

void Loader::_prepare() {
  namespace fs = std::filesystem;
  //   if (_device_type == length())
  //     // throw exception
  //     return;
  if (_is_flash_prog)
    if (!fs::exists(_flash_file)) return;
  if (_is_ext_flash_prog)
    if (!fs::exists(_ext_flash_file)) return;
  if (_is_eeprom_prog)
    if (!fs::exists(_eep_file)) return;

  _prepare_flash();
  _prepare_ext_flash();
  _prepare_eeprom();
  _prepare_device();

  //   stage
  std::vector<Stage> stg_vec;
  if (_is_flash_prog) {
    stg_vec.push_back(Stage::FLASH_PROG);
    _total_steps += _flash_pages.size();
  }
  if (_is_ext_flash_prog) {
    stg_vec.push_back(Stage::EXT_FLASH_PROG);
    _total_steps += _ext_flash_pages.size();
  }
  if (_is_eeprom_prog) {
    stg_vec.push_back(Stage::EEP_PROG);
    _total_steps += _eep_pages.size();
  }
  if (_is_ext_to_int) stg_vec.push_back(Stage::EXT_TO_INT);
  stg_vec.push_back(Stage::END);
  _total_steps++;

  //   prog time
  if (_device_type == 1 && _device_type == 2)
    _prog_time = _flash_pages.size() * 0.047 + _eep_pages.size() * 0.05 + 0.23;
  else if (_device_type == 3)  //  asa_m128_v2
    _prog_time = _flash_pages.size() * 0.047 + _eep_pages.size() * 0.05 + 0.23;
  else if (_device_type == 4)  //  asa_m3_v1
    _prog_time = _flash_pages.size() * 0.5 + _eep_pages.size() * 0.05 + 0.23;
  else if (_device_type == 5)  //  asa_m4_v1
    _prog_time =
        _flash_pages.size() * 0.5 + _eep_pages.size() * 0.05 + 0.23 + 3;
}

void Loader::_prepare_device() {
  auto ver = cmd.cmd_chk_protocol();
  int detected_device = 0;
  if (ver == 1) {
    /*
        protocol v1 dosn't have "chk_device" command
        m128_v1 or m128_v2
        use m128_v2 for default
    */
    detected_device = 2;
  } else if (ver == 2) {
    detected_device = cmd.cmd_v2_prog_chk_device();
    if (detected_device)
      ;  //  throw exception

  } else
    ;  // throw exception
  // auto detect device
  if (asa_dev_list[_device_type].protocol_version == 0)
    _device_type = detected_device;
  else if (asa_dev_list[_device_type].protocol_version == 1)
    if (_device_type != 2 && _device_type != 1)
      ;  // throw exception
    else if (asa_dev_list[_device_type].protocol_version == 1)
      if (detected_device != _device_type)
        ;  // throw exception

  _protocol_version = asa_dev_list[_device_type].protocol_version;
  _device_name = asa_dev_list[_device_type].name;
}

void Loader::_prepare_flash() {
  if (_is_flash_prog)  
  ;// TODO : try-catch
  // if()
}

void Loader::_prepare_ext_flash(){}

void Loader::_prepare_eeprom(){}

void Loader::_do_flash_prog_step(){}

void Loader::_do_ext_flash_prog_step(){}

void Loader::_do_ext_to_int_prog_step(){}

void Loader::_do_eep_prog_step(){}

void Loader::_do_prog_end_step(){}
////    Loader end

////    CMD start

CMD::CMD(QSerialPort &serial) : _serial(serial) { decoder = new Decoder(); }

CMD::~CMD() { delete (decoder); }

void CMD::_get_packet(Command &cmd_out, uint8_t *data_out) {
  bool exit_flag = false;
  char ch;
  while (!exit_flag) {
    if (_serial.read(&ch, 1)) decoder->step(ch);
    if (decoder->isDone) {
      decoder->getPacket(cmd_out, data_out);
      exit_flag = true;
    } else if (decoder->isError) {
      exit_flag = true;
    }
  }
}

void CMD::_get_packet(Command &cmd_out, char *data_out) {
  bool exit_flag = false;
  char ch;
  while (!exit_flag) {
    if (_serial.read(&ch, 1)) decoder->step(reinterpret_cast<uint8_t &>(ch));
    if (decoder->isDone) {
      decoder->getPacket(cmd_out, reinterpret_cast<uint8_t *>(data_out));
      exit_flag = true;
    } else if (decoder->isError) {
      exit_flag = true;
    }
  }
}

int CMD::_put_packet(Command cmd, const uint8_t *data) {
  uint8_t *raw = Decoder::encode(cmd, data);
  int &&ret = _serial.write(reinterpret_cast<char *&>(raw));
  delete raw;
  return ret;
}
int CMD::_put_packet(Command cmd, const char *data) {
  char *raw = reinterpret_cast<char *>(
      Decoder::encode(cmd, reinterpret_cast<const uint8_t *>(data)));
  int &&ret = _serial.write(raw);
  delete raw;
  return ret;
}

bool CMD::_base_cmd(Command cmd, const char *pac) {
  // const char pac[] = "";
  _put_packet(cmd, pac);
  Command ret_cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (ret_cmd == cmd) && (data[0] == 0);
  return cmp ? true : false;
}

bool CMD::_base_cmd(Command cmd,
                    int (*cmp_func)(const char *, const char *, size_t s),
                    const char *pac) {
  // const char pac[] = "";
  _put_packet(cmd, pac);
  Command ret_cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (ret_cmd == cmd) && (cmp_func(data, "OK!!", 4) == 0);
  return cmp ? true : false;
}

char *CMD::_advance_cmd(Command cmd, const char *pac) {
  // const char pac[] = "";
  _put_packet(cmd, pac);
  Command ret_cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (ret_cmd == cmd) && (data[0] == 0);
  if (cmp)
    return data + 1;
  else {
    return NULL;
  }
}

int8_t CMD::cmd_chk_protocol() {
  int8_t ret = 0;
  _put_packet(Command::CHK_PROTOCOL, "test");
  Command cmd;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::ACK1) && (strncmp(data, "OK!!", 4) == 0))
    ret = 1;
  else if ((cmd == Command::CHK_PROTOCOL) && (data[0] == 0))
    ret = data[1];
  return ret;
}

bool CMD::cmd_v1_enter_prog() {
  int8_t &&version = cmd_chk_protocol();
  return version == 1 ? true : false;
}

bool CMD::cmd_v1_flash_write(uint8_t *data) {
  return _put_packet(Command::DATA, data) > 0;
}

bool CMD::cmd_v1_prog_end() { return _base_cmd(Command::PROG_END, strncmp); }

bool CMD::cmd_v2_enter_prog() {
  int8_t &&version = cmd_chk_protocol();
  return version == 2 ? true : false;
}

int CMD::cmd_v2_prog_chk_device() {
  return _base_cmd(Command::PROG_CHK_DEVICE);
}

bool CMD::cmd_v2_prog_end() { return _base_cmd(Command::PROG_END); }

bool CMD::cmd_v2_prog_end_and_go_app() {
  return _base_cmd(Command::PROG_END_AND_GO_APP);
}

bool CMD::cmd_v2_prog_set_go_app_delay(uint16_t time) {
  char pac[sizeof(time) + 1] = "";
  memcpy(pac, (void *)htole16(time), sizeof(time));
  return _base_cmd(Command::PROG_SET_GO_APP_DELAY, pac);
}

bool CMD::cmd_v2_flash_set_pgsz(int32_t size) {
  char pac[sizeof(size) + 1] = "";
  memcpy(pac, (void *)htole32(size), sizeof(size));
  return _base_cmd(Command::FLASH_SET_PGSZ, pac);
}

int CMD::cmd_v2_flash_get_pgsz() {
  void *pac = _advance_cmd(Command::FLASH_GET_PGSZ);

  return le32toh(*reinterpret_cast<int32_t *>(pac));
}

bool CMD::cmd_v2_flash_write(uint32_t page_addr, uint8_t *data) {
  char *payload = new char[sizeof(uint32_t) + sizeof(data) + 1]();
  uint32_t &&le32 = htole32(page_addr);
  memcpy(payload, &le32, 4);
  memcpy(payload + 4, data, sizeof(data));
  bool &&ret = _base_cmd(Command::FLASH_WRITE, payload);
  delete payload;
  return ret;
}

char *CMD::cmd_v2_flash_read() { return _advance_cmd(Command::FLASH_READ); }

uint32_t CMD::cmd_v2_flash_earse_sector(uint16_t num) {
  char *payload = new char[sizeof(uint16_t) + 1]();
  char &&le16 = htole16(num);
  memcpy(payload, &le16, 2);
  char *pac = _advance_cmd(Command::FLASH_READ, payload);
  delete payload;
  if (pac)
    return le32toh(*reinterpret_cast<int *>(pac));
  else
    return 0;
}

bool CMD::cmd_v2_flash_earse_all() {
  return _base_cmd(Command::FLASH_EARSE_ALL);
}

bool CMD::cmd_v2_ext_flash_write(uint32_t page_addr, uint8_t *data) {
  char *payload = new char[sizeof(page_addr) + sizeof(data) + 1]();
  uint32_t &&le32 = htole32(page_addr);
  memcpy(payload, &le32, 4);
  memcpy(payload + 4, data, sizeof(data));
  bool &&ret = _base_cmd(Command::PROG_SET_GO_APP_DELAY, payload);
  delete payload;
  return ret;
}

bool CMD::cmd_v2_ext_flash_read() { return true; }

bool CMD::cmd_v2_prog_ext_to_int() {
  return _base_cmd(Command::PROG_EXT_TO_INT);
}

bool CMD::cmd_v2_eep_set_pgsz(uint32_t size) {
  char *payload = new char[sizeof(size) + 1]();
  uint32_t &&le32 = htole32(size);
  memcpy(payload, &le32, 4);
  bool &&ret = _base_cmd(Command::EEPROM_SET_PGSZ, payload);
  delete payload;
  return ret;
}

uint16_t CMD::cmd_v2_eep_get_pgsz() {
  char *pac = _advance_cmd(Command::EEPROM_GET_PGSZ);
  if (pac)
    return le16toh(*reinterpret_cast<uint16_t *>(pac));
  else
    return 0;
}

//  CMD start

uint32_t CMD::cmd_v2_eep_write(uint8_t *page_data) {
  char *pac =
      _advance_cmd(Command::EEPROM_WRITE, reinterpret_cast<char *>(page_data));
  if (pac)
    return le32toh(*reinterpret_cast<uint32_t *>(pac));
  else
    return 0;
}

uint32_t CMD::cmd_v2_eep_read() {
  char *pac = _advance_cmd(Command::EEPROM_READ);
  if (pac)
    return le32toh(*reinterpret_cast<uint32_t *>(pac));
  else
    return 0;
}

uint32_t CMD::cmd_v2_eep_earse() {
  char *pac = _advance_cmd(Command::EEPROM_EARSE);
  if (pac)
    return le32toh(*reinterpret_cast<uint32_t *>(pac));
  else
    return 0;
}

bool CMD::cmd_v2_eep_earse_all() {
  return _base_cmd(Command::EEPROM_EARSE_ALL);
}

bool CMD::cmd_v3_flash_earse_all() {  // unuse timer sleep
  return _base_cmd(Command::FLASH_EARSE_ALL);
}

bool CMD::cmd_v3_ext_flash_prepare(const char *filename) {
  _put_packet(Command::EXT_FLASH_FOPEN, filename);
  Command cmd;
  char *data;
  _get_packet(cmd, data);
  if (cmd == Command::EXT_FLASH_FOPEN) {
    if (data[0] != 2) return false;
    if (data[0] == 0) cmd_v3_ext_flash_hex_delete();

    return true;
  }

  return false;
}

bool CMD::cmd_v3_ext_flash_hex_delete() {
  return _base_cmd(Command::EXT_FLASH_HEX_DEL);
}

bool CMD::cmd_v3_ext_flash_finish() {
  char *payload;  // add date, time
  return _base_cmd(Command::EXT_FLASH_FCLOSE, payload);
}
}  // namespace Loader
   ////    CMD end