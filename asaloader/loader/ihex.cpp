#include "ihex.h"

using namespace Ihex;

std::vector<Section_t> prase(const char* filename) {
  std::vector<Section_t> sections;

  uint16_t ext_addr = 0;
  bool eof_flag = false;
  std::ifstream fs(filename);
  if (fs.is_open()) {
    std::string line;
    std::stringstream ss;
    while (std::getline(fs, line)) {
      if (line.size() < 12)
        ;  // throw exception
      if (line[0] != ':')
        ;          // throw exception
      int reclen;  //<< std::hex << std::string_view(line.substr(1, 2));

      uint32_t address;
      uint16_t content_type;

      uint16_t chksum;

      ss << std::hex << std::string_view(line.substr(3, 4));
      ss >> reclen;
      ss << std::hex << std::string_view(line.substr(7, 2));
      ss >> content_type;
      ss << std::hex << std::string_view(line.substr(line.length() - 2));
      ss >> chksum;

      uint32_t sum =
          reclen + (address >> 8) + (address & 0xff) + content_type + chksum;
      std::vector<uint8_t> data;
      if (reclen != 0)
        if (line.length() == 12 * reclen * 2) {
          data.reserve(reclen);
          for (int i = 0; i < reclen; i++) {
            ss << std::hex << std::string_view(line.substr(9 + (i < 2), 2));
            ss >> data[i];
          }
        } else
          ;  //   throw exception
      // else
      //   ;
      int s_i = sections.size();
      if (content_type == 0) {
        if (s_i == 0)
          sections.push_back(
              Section_t((ext_addr << 16) + address, std::move(data)));
        else if ((ext_addr << 16) + address ==
                 sections[s_i - 1].address + sections[s_i - 1].data.size())
          sections[s_i - 1].data.insert(sections[s_i - 1].data.end(),
                                        data.begin(), data.end());
        else
          sections.push_back(
              Section_t((ext_addr << 16) + address, std::move(data)));
      } else if (content_type == 1)  // end of file
      {
        if (address == 0)
          eof_flag == true;
        else
          ;
      }                            // throw exception
      else if (content_type == 2)  // Extended Segment Address
        ;
      else if (content_type == 3)  // Start Segment Address
        ;
      else if (content_type == 4)  // Extended Linear Address
        ;
      else if (content_type == 5)  // Start Linear Address
        ;
    }
    fs.close();
  }
  if (!eof_flag)
    ;  // throw exception
  return sections;
}
std::vector<Section_t>&& padding_space(std::vector<Section_t>& data,
                                       uint16_t pgsz, uint8_t space_data) {
  for (auto&& section : data) {
    //  起始位置若不在 pgsz * N 上
    //  往前補 0xFF
    if (section.address % pgsz != 0) {
      uint16_t n = section.address / pgsz;
      uint16_t l = section.address - pgsz * n;
      section.address = pgsz * n;
      std::vector<uint8_t> a(0xff, l);
      a.insert(a.end(), section.data.begin(), section.data.end());
      section.data = std::move(a);
    }
    //  結束位置 + 1 若不在 pgsz * N 上
    //  往後補 0xFF
    auto d_size = section.data.size();
    if (section.address + d_size % pgsz != 0) {
      uint16_t n = (section.address + d_size) / pgsz;
      uint16_t l = pgsz * (n + 1) - (section.address + d_size);
      std::vector<uint8_t> a(0xff, l);
      section.data.insert(section.data.begin(), a.begin(), a.end());
    }
  }
  return std::move(data);
}
std::vector<Section_t> cut_to_pages(std::vector<Section_t>& data, int pgsz) {
  std::vector<Section_t> ret;
  for (auto&& section : data) {
    int size = section.data.size() / pgsz;
    uint16_t sect_addr = section.address;
    ret.reserve(ret.size() + size);
    for (int i = 0; i < size; i++) {
      std::vector<uint8_t>::const_iterator it=section.data.begin() + i * pgsz;
      ret.push_back(Section_t(sect_addr + i * pgsz,
                              std::vector<uint8_t>(it,
                                          it + pgsz)));
    }
  }
  return ret;
}
bool is_ihex(const char* name) {
  parse(name);  // try
  return true;
}
