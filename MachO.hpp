
//------------------------------------------------------------------------------
// MachO.hpp
//------------------------------------------------------------------------------

#pragma once

#include <ostream>
#include <vector>

namespace macho {
    void write(std::ostream& out, std::vector<uint8_t>& buff);
};
