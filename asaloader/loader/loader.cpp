#include "loader.h"

#include <endian.h>
#include <time.h>
using namespace Loader;

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
  return _serial.write(reinterpret_cast<char *&>(raw));
}
int CMD::_put_packet(Command cmd, const char *data) {
  char *raw = reinterpret_cast<char *>(
      Decoder::encode(cmd, reinterpret_cast<const uint8_t *>(data)));
  return _serial.write(raw);
}

bool CMD::_base_cmd(Command cmd, int (*cmp_func)(const void *, const void *),
                    const void *arg1, const void *arg2);
{
  const char pac[] = "";
  _put_packet(cmd, pac);
  Command ret_cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (ret_cmd == cmd) && (cmp_func(arg1, arg2) == 0);
  delete data;
  return cmp ? true : false;
}

int8_t CMD::cmd_chk_protocol() {
  _put_packet(Command::CHK_PROTOCOL, "test");
  Command cmd;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::ACK1) && (strncmp(data, "OK!!", 4) == 0))
    return 1;
  else if ((cmd == Command::CHK_PROTOCOL) && (data[0] == 0))
    return data[1];
  else
    return 0;
}

bool CMD::cmd_v1_enter_prog() {
  int8_t &&version = cmd_chk_protocol();
  return version == 1 ? true : false;
}

bool CMD::cmd_v1_flash_write(uint8_t *data) {
  return _put_packet(DATA, data) > 0;
}

bool CMD::cmd_v1_prog_end() {
  const char pac[] = "";
  _put_packet(Command::DATA, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::ACK2) && (strncmp(data, "OK!!", 4) == 0))
    return true;
  else
    return false;
}

bool CMD::cmd_v2_enter_prog() {
  int8_t &&version = cmd_chk_protocol();
  return version == 2 ? true : false;
}

int CMD::cmd_v2_prog_chk_device() {
  const char pac[] = "";
  _put_packet(Command::PROG_CHK_DEVICE, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::PROG_CHK_DEVICE) && (data[0] == 0))
    return true;
  else
    return false;
}

bool CMD::cmd_v2_prog_end() {
  const char pac[] = "";
  _put_packet(Command::PROG_END, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::PROG_END) && (data[0] == 0))
    return true;
  else
    return false;
}

bool CMD::cmd_v2_prog_end_and_go_app() {
  const char pac[] = "";
  _put_packet(Command::PROG_END_AND_GO_APP, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::PROG_END_AND_GO_APP) && (data[0] == 0))
    return true;
  else
    return false;
}

bool CMD::cmd_v2_prog_set_go_app_delay() {
  const char pac[] = "";
  _put_packet(Command::PROG_SET_GO_APP_DELAY, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  if ((cmd == Command::PROG_SET_GO_APP_DELAY) && (data[0] == 0))
    return true;
  else
    return false;
}

bool CMD::cmd_v2_flash_set_pgsz(int size) {
  const char pac[] = "";
  _put_packet(Command::FLASH_SET_PGSZ, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (cmd == Command::FLASH_SET_PGSZ) && (data[0] == 0);
  delete data;
  return cmp ? true : false;
}

int CMD::cmd_v2_flash_get_pgsz() {
  const char pac[] = "";
  _put_packet(Command::FLASH_GET_PGSZ, pac);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (cmd == Command::FLASH_GET_PGSZ) && (data[0] == 0);
  delete data;
  return cmp ? true : false;
}

bool CMD::cmd_v2_flash_write(uint32_t page_addr, uint8_t *data_in) {
  uint8_t *payload = new uint8_t[sizeof(uint32_t) + sizeof(data_in)];
  uint32_t &&le32 = htole32(page_addr);
  memcpy(payload, &le32, 4);
  memcpy(payload, data_in, sizeof(data_in));
  _put_packet(Command::FLASH_WRITE, payload);
  Command cmd = Command::Null;
  char *data;
  _get_packet(cmd, data);
  bool &&cmp = (cmd == Command::FLASH_WRITE) && (data[0] == 0);
  delete data;
  return cmp ? true : false;
}

uint8_t *CMD::cmd_v2_flash_read() {}

int CMD::cmd_v2_flash_earse_sector(uint16_t num) {}

bool CMD::cmd_v2_flash_earse_all() {}

bool CMD::cmd_v2_ext_flash_write(uint32_t page_addr, uint8_t *data) {}

bool CMD::cmd_v2_ext_flash_read() {}

bool CMD::cmd_v2_prog_ext_to_int() {}

bool CMD::cmd_v2_eep_set_pgsz(uint32_t size) {}

bool CMD::cmd_v2_eep_get_pgsz() {}

int CMD::cmd_v2_eep_write(uint8_t *page_data) {}

int CMD::cmd_v2_eep_read() {}

int CMD::cmd_v2_eep_earse() {}

bool CMD::cmd_v2_eep_earse_all() {}

bool CMD::cmd_v3_flash_earse_all() {}

bool CMD::cmd_v3_ext_flash_prepare(const char *) {}

bool CMD::cmd_v3_ext_flash_hex_delete() {}

bool CMD::cmd_v3_ext_flash_finish() {}
