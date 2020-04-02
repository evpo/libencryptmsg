#pragma once
#include "memory_stream.h"
#include "emsg_types.h"
#include "packet_reader.h"
#include <vector>
#include <string>

namespace Botan
{
    class HashFunction;
    class Pipe;
}
namespace EncryptMsg
{
    enum class ArmorStatus {
        Unknown,
        Disabled,
        Header,
        Payload,
    };

    struct ArmorContext
    {
        ArmorStatus status = ArmorStatus::Unknown;
        std::string label;
    };

    class ArmorHeaderReader
    {
        private:
            InBufferStream in_stm_;
            ArmorContext &context_;
            SafeVector buffer_;

            EmsgResult ReadUnknownFormat(bool finish_packets);
        public:
            ArmorHeaderReader(ArmorContext &context):
                context_(context)
            {
            }

            InBufferStream &GetInStream()
            {
                return in_stm_;
            }

            EmsgResult Read(bool finish_packets);
    };

    class ArmorReader
    {
        private:
            InBufferStream in_stm_;
            ArmorContext &context_;
            SafeVector buffer_;
            std::unique_ptr<Botan::HashFunction> crc24_;
            std::string received_crc_;
            bool is_footer_found_;
            bool ValidateCRC(const std::string &crc);
        public:
            ArmorReader(ArmorContext &context);
            ~ArmorReader();
            EmsgResult Read(OutStream &out);
            EmsgResult Finish();
            InBufferStream &GetInStream()
            {
                return in_stm_;
            }
    };
}
