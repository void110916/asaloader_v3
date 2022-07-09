#include <endian.h>

#include <cstring>

#include "protcol.h"
using namespace Loader;

Decoder::Decoder() : _status{HEADER},_data{NULL} {
  header_buffer = new uint8_t[3]();  // initialize to 0s
}

Decoder::~Decoder() {
  delete header_buffer;
  delete _data;
}


void Decoder::put_data(uint8_t data) {
  static uint8_t* ptr = _data;
  *ptr = data;
  ptr++;
}
void Decoder::step(uint8_t data) {
  static uint8_t* buffer_ptr = header_buffer;
  switch (_status) {
    case HEADER:
      std::memcpy(buffer_ptr,header_buffer+1,2);
      buffer_ptr[2]=data;
      *buffer_ptr = data;
      if (std::memcmp(buffer_ptr, header_buffer, 3) == 0) {
        _chksum = 0;
        _status = COMMAND;
      }
      buffer_ptr++;
      break;
    case COMMAND:
      _command = data;
      _counter = 0;
      _status = TOCKEN;
      break;
    case TOCKEN:
      _status = LENGTH;
      break;
    case LENGTH:
      if (_counter) {
        _length += data;
        _counter = 0;
        delete _data;
        _data = new uint8_t[_length];
        _status = _status ? DATA : CHKSUM;
      } else
        _length = data << 8;
      break;
    case DATA:
      _chksum += data;
      _counter++;
      put_data(data);
      if (_counter == _length) _status = CHKSUM;
      break;
    case CHKSUM:
      if (_chksum != data) isError = true;
      _status = HEADER;
      delete header_buffer;
      header_buffer = new uint8_t[3]();
      isDone = true;
      break;
  }
}

void Decoder::getPacket(enum Command& cmd_out, uint8_t* data_out) {
  if (isDone) {
    cmd_out = (enum Command)_command;
    data_out = _data;
    isDone = false;
  } else {
    cmd_out = Null;
    data_out = NULL;
  }
}

/**
 * Encode command, data to a package.
 *
 * packet format:
 *       |-header-|-cmd-|-tocken-|-data len-|-data-|-chksum-|
 */
uint8_t* Decoder::encode(enum Command cmd,const uint8_t* data) {
  uint16_t &&len = std::strlen((char*)data);

  uint8_t chksum{0};
  for (int i = 0; i < len; i++) chksum = chksum + data[len];

  uint8_t* pac = new uint8_t[8 + len];
  std::memcpy(pac, _HEADER, sizeof(_HEADER));
  pac[3] = cmd;
  pac[4] = _TOCKEN;
  std::memcpy(pac + 5, (void*)htobe16(len), sizeof data);
  std::memcpy(pac + 7, data, sizeof data);
  pac[len + 7] = chksum;

  return pac;
}