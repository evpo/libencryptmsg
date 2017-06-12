#pragma once

#include <vector>
#include <string>
#include "botan/secmem.h"
#include "botan/symkey.h" // SymmetricKey type
#include "algo_spec.h"

namespace LibEncryptMsg
{
    using FileDate = unsigned;
    struct MessageParameters
    {
        Compression compression;
        CipherAlgo cipher_algo;
        std::string file_name;
        FileDate file_date;
        bool is_binary;
    };

    struct SecurityParameters
    {
        HashAlgo hash_algo;
        unsigned int iterations;
        std::vector<uint8_t> salt;
    };

    struct Passphrase
    {
        Botan::secure_vector<uint8_t> passphrase;
    };

    struct EncryptionKey
    {
        Botan::secure_vector<uint8_t> key;
    };

    class PacketAnalyzer
    {
        public:
            void Start();
            void Start(const Passphrase &passphrase);
            void Start(const EncryptionKey &encryption_key);
            // Analyze bytes. Returns true when the parameters are received
            bool Update(Botan::secure_vector<uint8_t> &buf, size_t offset = 0);
            void Finish(Botan::secure_vector<uint8_t> &buf, size_t offset = 0);
            const MessageParameters &GetMessageParameters() const;
            const SecurityParameters &GetSecurityParameters() const;
        private:
            MessageParameters message_parameters_;
            SecurityParameters security_parameters_;
    };

    class MessageReader
    {
        public:
            void Start(const Passphrase &passphrase);
            void Start(const EncryptionKey &encryption_key);
            void Update(Botan::secure_vector<uint8_t> &buf, size_t offset = 0);
            void Finish(Botan::secure_vector<uint8_t> &buf, size_t offset = 0);
            const MessageParameters &GetMessageParameters() const;
            const SecurityParameters &GetSecurityParameters() const;
        private:
            MessageParameters message_parameters_;
            SecurityParameters security_parameters_;
    };

    class MessageWriter
    {
        public:
            void Start(MessageParameters message_parameters, SecurityParameters security_parameters, const EncryptionKey &encryption_key);
            void Update(Botan::secure_vector<uint8_t>& buf, size_t offset = 0);
            void Finish(Botan::secure_vector<uint8_t>& buf, size_t offset = 0);
            const MessageParameters &GetMessageParameters() const;
        private:
            MessageParameters message_parameters_;
            SecurityParameters security_parameters_;
    };

    // Generates an encryption key and update salt in security parameters if it was empty
    EncryptionKey GenerateEncryptionKey(const Passphrase &passphrase, SecurityParameters &security_parameters);
}
