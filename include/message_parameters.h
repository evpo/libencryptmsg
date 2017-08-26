#pragma once
#include <string>
#include <vector>
#include <limits>
#include "algo_spec.h"
#include "emsg_types.h"

namespace LibEncryptMsg
{
    const size_t kMaxPacketChainLength = 5;
    const size_t kSaltSize = 8;

    struct MessageParameters
    {
        Compression compression;
        std::string file_name;
        FileDate file_date;
        bool is_binary;
        // power of two for partial length
        // This value should not be less than 9 according to the specification
        // TODO: enforce the rule
        uint8_t partial_length_power;
    };

    struct SecurityParameters
    {
        CipherAlgo cipher_algo;
        HashAlgo hash_algo;
        uint8_t iterations; // OpenPGP encoded iterations represented with 1 byte
        std::vector<uint8_t> salt;
    };

}
