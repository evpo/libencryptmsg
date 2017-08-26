#pragma once
#include <string>
#include <vector>
#include "emsg_mem.h"
#include "algo_spec.h"
#include "botan/secmem.h"
#include "botan/symkey.h"

namespace LibEncryptMsg
{
    enum class PacketResult
    {
        Success = 0,
        Pending,
        UnexpectedError,
        InvalidSurrogateIV,
        Empty,
        UnexpectedFormat,
        UnsupportedPacketType,
        UnsupportedAlgo,
        UnsupportedS2K,
        UnsupportedCompressionAlgo,
        IOError,
        IOErrorInput,
        IOErrorOutput,
        MDCError,
        CompressionError,
        KeyFileNotSpecified,
        IOErrorKeyFile,
        InvalidKeyFile,
        CurlIsNotFound,
        CurlExitNonZero,
        InvalidWadFile,
        InvalidPassphrase,
        KeyIsRequiredForSaving,
        InvalidKeyFilePassphrase,
        None,
    };

    using FileDate = unsigned;

    using Salt = std::vector<uint8_t>;

    using EncryptionKey = Botan::SymmetricKey;
}
