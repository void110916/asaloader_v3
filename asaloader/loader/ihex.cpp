#include "ihex.h"

#include <fstream>
#include <sstream>

namespace Ihex
{

  std::vector<Section_t> parse(const std::string filename)
  {
    std::vector<Section_t> sections;
    int s_i = 0;
    uint16_t ext_addr = 0;
    bool eof_flag = false;
    std::ifstream fs(filename);
    if (fs.is_open())
    {
      std::string line;
      while (std::getline(fs, line))
      {
        if (line.size() < 12)
          ; // throw exception
        if (line[0] != ':')
          ;         // throw exception
        int reclen; //<< std::hex << std::string_view(line.substr(1, 2));

        uint32_t address;
        uint16_t content_type;

        uint32_t chksum;
        auto a = line.substr(1, 2);
        std::istringstream(a) >> std::hex >> reclen;
        a = line.substr(3, 4);
        std::istringstream(a) >> std::hex >> address;
        a = line.substr(7, 2);
        std::istringstream(a) >> std::hex >> content_type;
        a = line.substr(line.length() - 2);
        std::istringstream(a) >> std::hex >> chksum;

        // uint32_t sum =
        //     reclen + (address >> 8) + (address & 0xff) + content_type + chksum;
        std::vector<uint8_t> data;
        if (reclen != 0)
        {
          if (line.length() == 11 + reclen * 2)
          {
            data.reserve(reclen);
            for (int i = 0; i < reclen; i++)
            {
              a = line.substr(9 + i * 2, 2);
              uint16_t tmp;
              std::istringstream(a) >> std::hex >> tmp;
              data.push_back(static_cast<uint8_t>(tmp));
            }
          }
          else
            ; //   throw exception
        }
        // else
        //   ;

        if (content_type == 0)
        {
          if (s_i == 0)
          {
            sections.push_back(
                Section_t((ext_addr << 16) + address, std::move(data)));
            s_i++;
          }
          else if ((ext_addr << 16) + address ==
                   sections[s_i - 1].address + sections[s_i - 1].data.size())
            sections[s_i - 1].data.insert(sections[s_i - 1].data.end(),
                                          data.begin(), data.end());
          else
          {
            sections.push_back(
                Section_t((ext_addr << 16) + address, std::move(data)));
            s_i++;
          }
        }
        else if (content_type == 1) // end of file
        {
          if (address == 0)
            eof_flag = true;
          else
            ; // throw exception
        }
        else if (content_type == 2) // Extended Segment Address
          ;
        else if (content_type == 3) // Start Segment Address
          ;
        else if (content_type == 4) // Extended Linear Address
          std::istringstream(line.substr(9, 4)) >> std::hex >> ext_addr;
        else if (content_type == 5) // Start Linear Address
          ;
      }
      fs.close();
    }
    if (!eof_flag)
      ; // throw exception
    // auto l = sections[0].data.size();
    return sections;
  }
  std::vector<Section_t> padding_space(std::vector<Section_t> &data,
                                       uint16_t pgsz, uint8_t space_data)
  {
    for (auto &&section : data)
    {
      //  起始位置若不在 pgsz * N 上
      //  往前補 0xFF
      if (section.address % pgsz != 0)
      {
        uint16_t n = section.address / pgsz;
        uint16_t l = section.address - pgsz * n;
        section.address = pgsz * n;
        std::vector<uint8_t> a(l, 0xff);
        // a.insert(a.end(), section.data.begin(), section.data.end());
        section.data.insert(section.data.begin(), a.begin(), a.end());
      }
      //  結束位置 + 1 若不在 pgsz * N 上
      //  往後補 0xFF
      auto d_size = section.data.size();
      if ((section.address + d_size) % pgsz != 0)
      {
        uint16_t n = (section.address + d_size) / pgsz;
        uint16_t l = pgsz * (n + 1) - (section.address + d_size);
        std::vector<uint8_t> a(l, 0xff);
        section.data.insert(section.data.end(), a.begin(), a.end());
      }
    }

    return data;
  }

  std::vector<Section_t> cut_to_pages(std::vector<Section_t> &data, int pgsz)
  {
    std::vector<Section_t> ret;
    for (auto &&section : data)
    {
      int size = section.data.size() / pgsz;
      uint16_t sect_addr = section.address;
      ret.reserve(ret.size() + size);
      for (int i = 0; i < size; i++)
      {
        std::vector<uint8_t>::const_iterator it = section.data.begin() + i * pgsz;
        ret.push_back(
            Section_t(sect_addr + i * pgsz, std::vector<uint8_t>(it, it + pgsz)));
      }
    }

    return ret;
  }

  bool is_ihex(const char *name)
  {
    parse(name); // try
    return true;
  }
} // namespace Ihex
