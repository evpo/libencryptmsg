//**********************************************************************************
//LibEncryptMsg Copyright 2018 Evgeny Pokhilko
//<https://evpo.net/libencryptmsg>
//
//LibEncryptMsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <memory>
#include <string>
#include "emsg_types.h"
#include "algo_spec.h"
#include "emsg_mem.h"

namespace LibEncryptMsg
{
    class PassphraseProvider
    {
        public:
            virtual ~PassphraseProvider() {}
            virtual std::unique_ptr<SafeVector> GetPassphrase(std::string description, bool &cancelled) = 0;
    };

    class SymmetricKeyProvider
    {
        public:
            virtual ~SymmetricKeyProvider() {}
            virtual std::unique_ptr<EncryptionKey> GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt,
                    std::string description, bool &cancelled) = 0;
    };

    struct Passphrase
    {
        const SafeVector data;
        Passphrase(const SafeVector &data_p);
    };

}

