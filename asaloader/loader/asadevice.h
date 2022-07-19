#ifndef ASADEVICE_H
#define ASADEVICE_H
#include <string>

namespace Loader {

struct asaDevice_t {
  const char *name;
  int dev_type;
  int protocol_version;
  int userapp_start;
  int userapp_size;
  const char *note;
};

const asaDevice_t asa_dev_list[] = {
    {.name = "auto",
     .dev_type = 0,
     .protocol_version = 0,
     .userapp_start = 0,
     .userapp_size = 0,
     .note = "Default, auto detect device type."},

    {.name = "asa_m128_v1",
     .dev_type = 1,
     .protocol_version = 1,
     .userapp_start = 0x0000'0000,
     .userapp_size = 0x0001'F000,
     .note = ""},

    {.name = "asa_m128_v2",
     .dev_type = 2,
     .protocol_version = 1,
     .userapp_start = 0x0000'0000,
     .userapp_size = 0x0001'F000,
     .note = ""},

    {.name = "asa_m128_v3",
     .dev_type = 3,
     .protocol_version = 2,
     .userapp_start = 0x0000'0000,
     .userapp_size = 0x0001'F000,
     .note = ""},

    {.name = "asa_m3_v1",
     .dev_type = 4,
     .protocol_version = 2,
     .userapp_start = 0x0000'1000,
     .userapp_size = 0x0007'F000,
     .note = ""},

    {.name = "asa_m4_v1",
     .dev_type = 5,
     .protocol_version = 2,
     .userapp_start = 0x0001'0000,
     .userapp_size = 0x000F'0000,
     .note = ""}};
}  // namespace Loader

#endif