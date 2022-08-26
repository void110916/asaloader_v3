#include "loader.h"

#include <QDateTime>
#include <QThread>
#include <filesystem>
#include <iostream>

#include "asadevice.h"
#include "endianness.h"

////    Loader start
namespace Loader {
Loader::Loader(QSerialPort &serial, int device_type, bool is_flash_prog,
               bool is_ext_flash_prog, bool is_eeprom_prog, bool is_ext_to_int,
               bool is_go_app, QString flash_file, QString ext_flash_file,
               QString eeprom_file, int go_app_delay)
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
      _go_app_delay(go_app_delay) {
  _prepare();
}

Loader::Loader(QSerialPort &serial, int device_type,
               bool is_flash_prog , QString flash_file ,
               bool is_go_app , int go_app_delay)
    : _serial(serial),
      cmd(serial),
      _device_type(device_type),
      _is_flash_prog(is_flash_prog),
      _is_go_app(is_go_app),
      _flash_file(flash_file),
      _go_app_delay(go_app_delay) {
  _prepare();
}
Loader::~Loader() {
  if (_serial.isOpen()) _serial.close();
}

void Loader::_prepare() {
  namespace fs = std::filesystem;
  //   if (_device_type == length())
  //     // throw exception
  //     return;
  if (_is_flash_prog)
    if (!fs::exists(_flash_file.toStdString())) return;

  if (_is_ext_flash_prog)
    if (!fs::exists(_ext_flash_file.toStdString())) return;
  if (_is_eeprom_prog)
    if (!fs::exists(_eep_file.toStdString())) return;

  _prepare_flash();
  _prepare_ext_flash();
  _prepare_eeprom();
  _prepare_device();

  //   stage

  if (_is_flash_prog) {
    _stg_vec.push_back(Stage::FLASH_PROG);
    _total_steps += _flash_pages.size();
  }
  if (_is_ext_flash_prog) {
    _stg_vec.push_back(Stage::EXT_FLASH_PROG);
    _total_steps += _ext_flash_pages.size();
  }
  if (_is_eeprom_prog) {
    _stg_vec.push_back(Stage::EEP_PROG);
    _total_steps += _eep_pages.size();
  }
  if (_is_ext_to_int) _stg_vec.push_back(Stage::EXT_TO_INT);
  _stg_vec.push_back(Stage::END);
  _total_steps++;
  _stage = _stg_vec.begin();
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
  else if (asa_dev_list[_device_type].protocol_version == 1) {
    if (_device_type != 2 && _device_type != 1)
      ;  // throw exception
    else if (asa_dev_list[_device_type].protocol_version == 1)
      if (detected_device != _device_type)
        ;  // throw exception
  }
  _protocol_version = asa_dev_list[_device_type].protocol_version;
  _device_name = asa_dev_list[_device_type].name;
}

void Loader::_prepare_flash() {
  if (_is_flash_prog) {  // TODO : try-catch
    auto blocks = Ihex::parse(_flash_file.toStdString());

    for (auto &&block : blocks) _flash_size += block.data.size();
    blocks = Ihex::padding_space(blocks, 256, 0xff);

    _flash_pages = Ihex::cut_to_pages(blocks, 256);
  }
}

void Loader::_prepare_ext_flash() {
  if (_is_ext_flash_prog) {  // TODO : try-catch
    auto blocks = Ihex::parse(_ext_flash_file.toStdString());
    for (auto &&block : blocks) _flash_size += block.data.size();
    blocks = Ihex::padding_space(blocks, 256, 0xff);
    _ext_flash_pages = Ihex::cut_to_pages(blocks, 256);
  }
}

void Loader::_prepare_eeprom() {
  if (_is_eeprom_prog) {  // TODO : try-catch
    auto blocks = Ihex::parse(_eep_file.toStdString());
    for (auto &&block : blocks) _flash_size += block.data.size();
    blocks = Ihex::padding_space(blocks, 256, 0xff);
    _ext_flash_pages = Ihex::cut_to_pages(blocks, 256);
  }
}

void Loader::_do_flash_prog_step() {
  if (_protocol_version == 1) {
    // protocol v1 will auto clear flash after command "chk_protocol"
    cmd.cmd_v1_flash_write(_flash_pages[_flash_page_idx].data);
    // time sleep 0.03s
  } else if (_protocol_version == 2) {
    if (_flash_page_idx == 0) {
      if (_device_type == 5)  // asa_m4_v1 takes longer chip erase time
        cmd.cmd_v3_flash_earse_all();
      else
        cmd.cmd_v2_flash_earse_all();
    }
    auto addr = _flash_pages[_flash_page_idx].address;
    auto data = _flash_pages[_flash_page_idx].data;

    cmd.cmd_v2_flash_write(addr, data);
  }
  // for (int i = 254; i <= _flash_pages[_flash_page_idx].data.size(); i++)
  //   auto ss = _flash_pages[_flash_page_idx].data[i];
  _flash_page_idx++;
  _cur_step++;
  if (_flash_page_idx == _flash_pages.size()) _stage++;
}

void Loader::_do_ext_flash_prog_step() {
  if (_protocol_version == 1) {
    // this command do not support protocol v1 currently
    ;
  } else if (_protocol_version == 2) {
    if (_flash_page_idx == 0)
      cmd.cmd_v3_ext_flash_prepare(
          _ext_flash_file.toStdString().substr(0, 7).data());
    cmd.cmd_v2_ext_flash_write(_flash_pages[_flash_page_idx].address,
                               _flash_pages[_flash_page_idx].data);
  }
  _flash_page_idx++;
  _cur_step++;
  if (_flash_page_idx == _flash_pages.size()) _stage++;
}

void Loader::_do_ext_to_int_prog_step() {
  if (_protocol_version == 1) {
    // this command do not support protocol v1 currently
    ;
  } else if (_protocol_version == 2) {
    // TODO: 修改為燒錄特定編號檔案
    if (_flash_page_idx == 0) {
      if (_device_type == 5)  //
        cmd.cmd_v3_flash_earse_all();
      else
        cmd.cmd_v2_flash_earse_all();
    }
    cmd.cmd_v2_prog_ext_to_int();
  }
  // _flash_page_idx++;
  // _cur_step++;
  // if (_flash_page_idx == _flash_pages.size()) _stage++;
}

void Loader::_do_eep_prog_step() {
  if (_protocol_version == 2)
    cmd.cmd_v2_eep_write(_eep_pages[_eep_page_idx].data);
  _eep_page_idx++;
  _cur_step++;
  if (_eep_page_idx == _eep_pages.size()) _stage++;
}

void Loader::_do_prog_end_step() {
  if (_protocol_version == 1)
    cmd.cmd_v1_prog_end();
  else if (_protocol_version == 2) {
    if (_is_go_app) {
      cmd.cmd_v2_prog_set_go_app_delay(_go_app_delay);
      cmd.cmd_v2_prog_end_and_go_app();
    } else
      cmd.cmd_v2_prog_end();
  }
  _cur_step++;
}

void Loader::do_step() {
  switch (*_stage) {
    case Stage::FLASH_PROG:
      _do_flash_prog_step();
      break;
    case Stage::EEP_PROG:
      _do_eep_prog_step();
      break;
    case Stage::EXT_FLASH_PROG:
      _do_ext_flash_prog_step();
      break;
    case Stage::EXT_TO_INT:
      _do_ext_to_int_prog_step();
      break;
    case Stage::END:
      _do_prog_end_step();
      break;
  }
}
////    Loader end

////    CMD start

CMD::CMD(QSerialPort &serial) : _serial(serial) { decoder = new Decoder(); }

CMD::~CMD() { delete (decoder); }

// void CMD::_get_packet(Command &cmd_out, uint8_t *&data_out)
// {
//   bool exit_flag = false;
//   char ch;
//   auto canRead = _serial.waitForReadyRead(3000);
//   while (!exit_flag)
//   {
//     if (_serial.read(&ch, 1))
//       decoder->step(ch);
//     if (decoder->isDone)
//     {
//       decoder->getPacket(cmd_out, data_out);
//       exit_flag = true;
//     }
//     else if (decoder->isError)
//     {
//       exit_flag = true;
//     }
//   }
// }

void CMD::_get_packet(Command &cmd_out, std::vector<uint8_t> &data_out) {
  bool exit_flag = false;
  char ch = 0;
  // QThread::usleep(10);
  _serial.waitForReadyRead(3000);
  _serial.bytesAvailable();
  while (!exit_flag) {
    auto rd = _serial.read(&ch, 1);
    if (rd > 0)
      decoder->step(reinterpret_cast<uint8_t &>(ch));
    else {
      // std::cout << "no full packet" << std::endl;
      _serial.waitForReadyRead(3000);
    }
    if (decoder->isDone) {
      decoder->getPacket(cmd_out, data_out);
      exit_flag = true;
    } else if (decoder->isError) {
      exit_flag = true;
    }
  }
}

int CMD::_put_packet(Command cmd, std::vector<uint8_t> data) {
  std::vector<uint8_t> &&raw = Decoder::encode(cmd, data);
  int ret =
      _serial.write(reinterpret_cast<const char *>(raw.data()), raw.size());
  _serial.waitForBytesWritten(3000);
  return ret;
}
int CMD::_put_packet(Command cmd, const char *data) {
  const uint8_t *pdata = reinterpret_cast<const uint8_t *>(data);

  std::vector<uint8_t> &&raw =
      Decoder::encode(cmd, std::vector<uint8_t>(pdata, pdata + strlen(data)));

  int ret =
      _serial.write(reinterpret_cast<const char *>(raw.data()), raw.size());
  _serial.waitForBytesWritten(3000);
  return ret;
}

bool CMD::_base_cmd(Command cmd, const char *pac, int millisec) {
  // const char pac[] = "";
  _put_packet(cmd, pac);
  QThread::msleep(millisec);
  Command ret_cmd = Command::Null;
  std::vector<uint8_t> data;
  _get_packet(ret_cmd, data);
  bool cmp = (ret_cmd == cmd) && (data[0] == 0);
  return cmp;
}

bool CMD::_base_cmd(Command cmd, std::vector<uint8_t> pac) {
  _put_packet(cmd, pac);
  Command ret_cmd = Command::Null;
  std::vector<uint8_t> data;
  _get_packet(ret_cmd, data);
  bool cmp = (ret_cmd == cmd) && (data[0] == 0);
  return cmp;
}
std::vector<uint8_t> CMD::_advance_cmd(Command cmd, std::vector<uint8_t> pac) {
  // const char pac[] = "";
  _put_packet(cmd, pac);
  Command ret_cmd = Command::Null;
  std::vector<uint8_t> data;
  _get_packet(ret_cmd, data);
  bool &&cmp = (ret_cmd == cmd) && (data[0] == 0);
  // for(auto s:data)

  if (cmp)
    return std::move(data);
  else {
    return std::vector<uint8_t>();
  }
}

int8_t CMD::cmd_chk_protocol() {
  int8_t ret = 0;
  _put_packet(Command::CHK_PROTOCOL, "test");
  Command cmd = Command::Null;
  std::vector<uint8_t> data;
  _get_packet(cmd, data);
  if ((cmd == Command::ACK1) && (memcmp(data.data(), "OK!!", 4) == 0))
    ret = 1;
  else if ((cmd == Command::CHK_PROTOCOL) && (data[0] == 0))
    ret = data[1];
  return ret;
}

bool CMD::cmd_v1_enter_prog() {
  int8_t &&version = cmd_chk_protocol();
  return version == 1 ? true : false;
}

bool CMD::cmd_v1_flash_write(std::vector<uint8_t> data) {
  return _put_packet(Command::DATA, data) > 0;
}

bool CMD::cmd_v1_prog_end() {
  _put_packet(Command::PROG_END, "");
  Command ret_cmd = Command::Null;
  std::vector<uint8_t> data;
  _get_packet(ret_cmd, data);
  bool &&cmp =
      (ret_cmd == Command::PROG_END) && (memcmp(data.data(), "OK!!", 4) == 0);
  return cmp ? true : false;
}

bool CMD::cmd_v2_enter_prog() {
  int8_t &&version = cmd_chk_protocol();
  return version == 2 ? true : false;
}

int CMD::cmd_v2_prog_chk_device() {
  return *(_advance_cmd(Command::PROG_CHK_DEVICE).data() + 1);
}

bool CMD::cmd_v2_prog_end() { return _base_cmd(Command::PROG_END); }

bool CMD::cmd_v2_prog_end_and_go_app() {
  return _base_cmd(Command::PROG_END_AND_GO_APP);
}

bool CMD::cmd_v2_prog_set_go_app_delay(uint16_t time) {
  char pac[sizeof(time) + 1] = "";
  auto le = htole16(time);
  memcpy(pac, (void *)&le, sizeof(time));
  return _base_cmd(Command::PROG_SET_GO_APP_DELAY, pac);
}

bool CMD::cmd_v2_flash_set_pgsz(int32_t size) {
  char pac[sizeof(size) + 1] = "";
  auto le = htole32(size);
  memcpy(pac, (void *)&le, sizeof(size));
  return _base_cmd(Command::FLASH_SET_PGSZ, pac);
}

int CMD::cmd_v2_flash_get_pgsz() {
  void *pac = _advance_cmd(Command::FLASH_GET_PGSZ).data() + 1;

  return le32toh(*reinterpret_cast<int32_t *>(pac));
}

bool CMD::cmd_v2_flash_write(uint32_t page_addr, std::vector<uint8_t> data) {
  std::vector<uint8_t> payload;
  payload.reserve(sizeof(uint32_t) + data.size());
  uint32_t le32 = htole32(page_addr);
  uint8_t *ple32 = reinterpret_cast<uint8_t *>(&le32);
  payload.insert(payload.begin(), ple32, ple32 + 4);
  // memcpy(payload, &le32, 4);
  payload.insert(payload.end(), make_move_iterator(data.cbegin()),
                 make_move_iterator(data.cend()));
  // delete[] payload;
  return _base_cmd(Command::FLASH_WRITE, payload);
}

std::vector<uint8_t> CMD::cmd_v2_flash_read() {
  std::vector<uint8_t> &&ret = _advance_cmd(Command::FLASH_READ);

  return std::vector(make_move_iterator(ret.begin() + 1),
                     make_move_iterator(ret.end()));
}  // WARNING : allocate release issue

uint32_t CMD::cmd_v2_flash_earse_sector(uint16_t num) {
  std::vector<uint8_t> payload;
  payload.reserve(sizeof(uint16_t));
  // char *payload = new char[sizeof(uint16_t) + 1]();
  uint16_t le16 = htole16(num);
  uint8_t *ple16 = reinterpret_cast<uint8_t *>(&le16);
  payload.insert(payload.begin(), ple16, ple16 + 2);
  // memcpy(payload, &le16, 2);
  void *pac = _advance_cmd(Command::FLASH_READ, payload).data() + 1;
  if (pac)
    return le32toh(*reinterpret_cast<int *>(pac));
  else
    return 0;
}

bool CMD::cmd_v2_flash_earse_all() {
  return _base_cmd(Command::FLASH_EARSE_ALL, "", 2200);
}

bool CMD::cmd_v2_ext_flash_write(uint32_t page_addr,
                                 std::vector<uint8_t> data) {
  std::vector<uint8_t> payload;
  payload.reserve(sizeof(page_addr) + sizeof(data) + 1);
  // char *payload = new char[sizeof(page_addr) + sizeof(data) + 1]();
  uint32_t le32 = htole32(page_addr);
  uint8_t *ple32 = reinterpret_cast<uint8_t *>(&le32);
  payload.insert(payload.begin(), ple32, ple32 + 4);
  // memcpy(payload, &le32, 4);
  payload.insert(payload.end(), make_move_iterator(data.cbegin()),
                 make_move_iterator(data.cend()));
  // memcpy(payload + 4, data, sizeof(data));
  // delete[] payload;
  return _base_cmd(Command::PROG_SET_GO_APP_DELAY, payload);
}

bool CMD::cmd_v2_ext_flash_read() { return true; }

bool CMD::cmd_v2_prog_ext_to_int() {
  return _base_cmd(Command::PROG_EXT_TO_INT);
}

bool CMD::cmd_v2_eep_set_pgsz(uint32_t size) {
  std::vector<uint8_t> payload(sizeof(size));
  uint32_t le32 = htole32(size);
  uint8_t *ple32 = reinterpret_cast<uint8_t *>(&le32);
  payload.insert(payload.end(), ple32, ple32 + 4);
  return _base_cmd(Command::EEPROM_SET_PGSZ, payload);
}

uint16_t CMD::cmd_v2_eep_get_pgsz() {
  void *pac = _advance_cmd(Command::EEPROM_GET_PGSZ).data() + 1;
  if (pac)
    return le16toh(*reinterpret_cast<uint16_t *>(pac));
  else
    return 0;
}

//  CMD start

uint32_t CMD::cmd_v2_eep_write(std::vector<uint8_t> page_data) {
  void *pac = _advance_cmd(Command::EEPROM_WRITE, page_data).data() + 1;
  if (pac)
    return le32toh(*reinterpret_cast<uint32_t *>(pac));
  else
    return 0;
}

uint32_t CMD::cmd_v2_eep_read() {
  void *pac = _advance_cmd(Command::EEPROM_READ).data() + 1;
  if (pac)
    return le32toh(*reinterpret_cast<uint32_t *>(pac));
  else
    return 0;
}

uint32_t CMD::cmd_v2_eep_earse() {
  void *pac = _advance_cmd(Command::EEPROM_EARSE).data() + 1;
  if (pac)
    return le32toh(*reinterpret_cast<uint32_t *>(pac));
  else
    return 0;
}

bool CMD::cmd_v2_eep_earse_all() {
  return _base_cmd(Command::EEPROM_EARSE_ALL);
}

bool CMD::cmd_v3_flash_earse_all() {  // unuse timer sleep
  return _base_cmd(Command::FLASH_EARSE_ALL, "", 3000);
}

bool CMD::cmd_v3_ext_flash_prepare(const char *filename) {
  _put_packet(Command::EXT_FLASH_FOPEN, filename);
  Command cmd;
  std::vector<uint8_t> data;
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
  auto now = QDateTime::currentDateTime();
  std::vector<uint8_t> payload;
  payload.push_back(now.time().minute());
  payload.push_back(now.time().hour());
  payload.push_back(now.date().day());
  payload.push_back(now.date().month());
  payload.push_back(now.date().year() - 2000);
  return _base_cmd(Command::EXT_FLASH_FCLOSE, payload);
}
}  // namespace Loader
   ////    CMD end
