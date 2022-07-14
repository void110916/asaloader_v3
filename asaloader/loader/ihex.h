#include <fstream>
#include <sstream>
#include <string>
#include <vector>
namespace Ihex {
struct Section_t {
  uint16_t address;
  std::vector<uint8_t> data;
  Section_t(uint16_t address, std::vector<uint8_t>&& data)
      : address(address), data(std::move(data)) {}
      Section_t(uint16_t address, std::vector<uint8_t>& data):
      address(address), data(data){}
  ~Section_t() {}
};

std::vector<Section_t> parse(const char* filename);
std::vector<Section_t> padding_space(std::vector<Section_t>& data, int pgsz,
                                     uint8_t space_data);
std::vector<Section_t> cut_to_pages(std::vector<Section_t>& data,int pgsz);
bool is_ihex(const char* name);
}  // namespace Ihex
