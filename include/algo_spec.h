#pragma once
#include <string>

namespace LibEncryptMsg
{
    enum class Compression
    {
        Uncompressed = 0,
        ZIP = 1,
        ZLIB = 2,
        BZip2 = 3,
    };

    enum class CipherAlgo
    {
        Unknown = -1,
        TripleDES = 2,
        CAST5 = 3,
        AES128 = 7,
        AES256 = 9,
    };

    enum class HashAlgo
    {
        Unknown = -1,
        SHA160 = 2,
        SHA256 = 8,
        SHA512 = 10,
    };

    struct AlgoSpec
    {
        CipherAlgo cipher_algo;
        std::string botan_name;
        int block_size;
        int key_size;
    };

    struct HashSpec
    {
        HashAlgo hash_algo;
        std::string botan_name;
    };

    const AlgoSpec &GetAlgoSpec(CipherAlgo cipher_algo);
    const HashSpec &GetHashSpec(HashAlgo hash_algo);

    const HashAlgo kDefaultHashAlgo = HashAlgo::SHA256;
    const CipherAlgo kDefaultCipherAlgo = CipherAlgo::AES256;
    const int kDefaultIterations = 1015808;
    const int kMaxIterations = 65011712;
    const Compression kDefaultCompression = Compression::ZLIB;
}
