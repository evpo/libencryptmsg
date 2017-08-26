#pragma once
#include <string>

namespace LibEncryptMsg
{
    namespace Cli
    {
        void Encrypt(const std::string &input, const std::string &output, const std::string &pwd_file);
        void Decrypt(const std::string &input, const std::string &output, const std::string &pwd_file);
    }
}
