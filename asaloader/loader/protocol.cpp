#include "endianness.h"
#include "protcol.h"

#include <cstring>
#include <iterator>

using namespace Loader;

Decoder::Decoder() : _status{HEADER}
{
  // header_buffer = new uint8_t[3](); // initialize to 0s
}

Decoder::~Decoder()
{
  // delete header_buffer;
}

// void Decoder::put_data(uint8_t data)
// {
//   // static uint8_t *ptr = _data;
//   *_data_ptr = data;
//   _data_ptr++;
// }
void Decoder::step(uint8_t data)
{
  switch (_status)
  {
  case _Status::HEADER:
    //    std::memcpy(buffer_ptr, header_buffer + 1, 2);
    //    buffer_ptr[2] = data;
    *buffer_ptr++ = data;
    if (std::memcmp(_HEADER, header_buffer.data(), 3) == 0)
    {
      _chksum = 0;
      _status = _Status::COMMAND;
    }
    //    buffer_ptr;
    break;
  case _Status::COMMAND:
    _command = data;
    _counter = 0;
    _status = TOCKEN;
    break;
  case _Status::TOCKEN:
    _status = _Status::LENGTH;
    break;
  case _Status::LENGTH:
    if (_counter == 1)
    {
      _length += data;
      _counter = 0;
      // delete _data;
      // _data = new uint8_t[_length]();
      _data.clear();
      _data.reserve(_length);
      _data_ptr = _data.begin();
      _status = _status ? _Status::DATA : _Status::CHKSUM;
    }
    else
    {
      _length = data << 8;
      _counter++;
    }
    break;
  case _Status::DATA:
    _chksum += data;
    _counter++;
    *_data_ptr++ = data;

    if (_counter == _length)
      _status = _Status::CHKSUM;
    break;
  case _Status::CHKSUM:
    if (_chksum != data)
      isError = true;
    _status = _Status::HEADER;
    // delete header_buffer;
    // header_buffer = new uint8_t[3]();
    header_buffer.fill(0);
    buffer_ptr = header_buffer.begin();
    isDone = true;
    break;
  }
}

void Decoder::getPacket(enum Command &cmd_out, std::vector<uint8_t> &data_out)
{
  if (isDone)
  {
    cmd_out = static_cast<enum Command>(_command);
    data_out = std::move(_data);
    isDone = false;
  }
  else
  {
    cmd_out = Command::Null;
    data_out = std::vector<uint8_t>();
  }
}
const uint8_t Decoder::_TOCKEN;
/**
 * Encode command, data to a package.
 *
 * packet format:
 *       |-header-|-cmd-|-tocken-|-data len-|-data-|-chksum-|
 */
std::vector<uint8_t> Decoder::encode(enum Command cmd, std::vector<uint8_t> data)
{
  uint16_t &&len = data.size();

  uint8_t chksum = 0;
  for (int i = 0; i < len; ++i)
    chksum += data[i];

  std::vector<uint8_t> pac;

  pac.reserve(8 + len);
  pac.insert(pac.end(), _HEADER, _HEADER + 3);
  pac.push_back(cmd);
  pac.push_back(Decoder::_TOCKEN);
  pac.push_back(len >> 8);
  pac.push_back(len & 0xff);
  pac.insert(pac.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end()));
  pac.push_back(chksum);
  // auto size = pac.size();
  // auto d = pac.data();
  // for (auto s : pac)
  //   auto ss = s;
  return pac;
}
