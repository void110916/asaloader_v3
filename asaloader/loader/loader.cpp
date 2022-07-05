#include "loader.h"
#include<time.h>
using namespace Loader;

uint8_t* CMD::_get_packet() {
  uint8_t* pac;
  bool exit_flag=false;
this->_serial.read(1);
  return pac;
}

void CMD::_put_packet(enum Command cmd, uint8_t* data) {
    auto raw=Decoder::encode(cmd,data);
    auto a=QByteArray((char*)raw);
    this->_serial.write(a);
}