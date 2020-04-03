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
    enum class ArmorState {
        Unknown,
        BeginHeader,
        Header,
        Payload,
        CRC,
        Tail,
        TailFound,
        Disabled,
    };

    class ArmorReader
    {
        public:
            enum class ArmorReaderResult
            {
                None,
                Pending,
                MinHeader,
                Disabled,
                UnexpectedFormat,
                Success,
            };

            struct LineResult
            {
                bool success = false;
                bool max_length_reached = false;
                std::vector<uint8_t> line;
            };
        private:
            ArmorState state_ = ArmorState::Unknown;
            std::string label_;
            InBufferStream in_stm_;
            SafeVector buffer_;
            std::unique_ptr<Botan::HashFunction> crc24_;
            EmsgResult emsg_result_;
            bool finish_;

            bool ValidateCRC(const std::string &crc);
            LineResult NextLine();
            void SetState(ArmorState state);
            ArmorReaderResult ReadUnknown();
            ArmorReaderResult ReadBeginHeader();
            ArmorReaderResult ReadHeader();
            ArmorReaderResult ReadPayload(OutStream &out);
            ArmorReaderResult ReadCRC();
            ArmorReaderResult ReadTail();

            bool Continue();

        public:
            ArmorReader();
            ~ArmorReader();
            EmsgResult Read(OutStream &out);
            EmsgResult Finish(OutStream &out);
            InBufferStream &GetInStream()
            {
                return in_stm_;
            }

            ArmorState GetState()
            {
                return state_;
            }
    };
}
