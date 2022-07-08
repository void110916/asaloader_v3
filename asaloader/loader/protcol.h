#include <bitset>
#include <cstddef>
#include <cstdint>
namespace Loader {
enum Command : uint8_t {
  // for both v1 and v2
  CHK_PROTOCOL = 0xFA,

  // for version 1 protocol supproted device such as
  //    asa_m128_v1
  //    asa_m128_v2
  ACK1 = 0xFB,
  DATA = 0xFC,
  ACK2 = 0xFD,

  // for version 2 protocol supproted device such as
  //    asa_m128_v3
  //    asa_m3_v1
  PROG_CHK_DEVICE = 0x02,
  PROG_END = 0x03,
  PROG_END_AND_GO_APP = 0x04,
  PROG_SET_GO_APP_DELAY = 0x05,

  PROG_EXT_TO_INT = 0x06,

  FLASH_SET_PGSZ = 0x10,
  FLASH_GET_PGSZ = 0x11,
  FLASH_WRITE = 0x12,
  FLASH_READ = 0x13,
  FLASH_VARIFY = 0x14,
  FLASH_EARSE_SECTOR = 0x15,
  FLASH_EARSE_ALL = 0x16,

  EEPROM_SET_PGSZ = 0x20,
  EEPROM_GET_PGSZ = 0x21,
  EEPROM_WRITE = 0x22,
  EEPROM_READ = 0x23,
  EEPROM_EARSE = 0x24,
  EEPROM_EARSE_ALL = 0x25,

  EXT_FLASH_FOPEN = 0x30,
  EXT_FLASH_FCLOSE = 0x31,
  EXT_FLASH_WRITE = 0x32,
  EXT_FLASH_READ = 0x33,
  EXT_FLASH_VARIFY = 0x34,
  EXT_FLASH_EARSE_SECTOR = 0x35,
  EXT_FLASH_HEX_DEL = 0x36,
  Null
};

class Decoder {
 private:
  int _status;
  int _counter;
  int _command;
  int _length;
  uint8_t *_data;
  int _chksum;

  uint8_t *header_buffer;
  static constexpr uint8_t _HEADER[3] = {0x00, 0x00, 0x00};
  static const uint8_t _TOCKEN = 0x01;
  enum _Status : int {
    HEADER = 0,
    COMMAND = 1,
    TOCKEN = 2,
    LENGTH = 3,
    DATA = 4,
    CHKSUM = 5
  };
  void put_data(uint8_t data);

 public:
  bool isError;
  bool isDone;
  
  Decoder();
  ~Decoder();
  void step(uint8_t data);
  void getPacket(enum Command &cmd_out, uint8_t *data_out);
  /**
   * @details Encode command, data to a package.
   * @param cmd Command in the package.
   * @param data Data in the package.
   * @return uint8_t* Package as bytes.
   */
  static uint8_t *encode(enum Command cmd,const uint8_t *data);
};

}  // namespace Loader
