#pragma once
#include "botan/secmem.h"
#include <limits>

namespace LibEncryptMsg
{
    using SecureVector = Botan::secure_vector<uint8_t>;
    const size_t kMaxStreamLength = std::numeric_limits<size_t>::max();
}
