#pragma once
#include "memory_stream.h"
#include "emsg_types.h"
#include "packet_reader.h"
#include <vector>

namespace Botan
{
    class HashFunction;
    class Pipe;
}
namespace EncryptMsg
{
    class ArmorHeaderReader
    {
        private:
            InBufferStream in_stm_;
            SessionState &state_;
            SafeVector buffer_;
        public:
            ArmorHeaderReader(SessionState &state):
                state_(state)
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
            SessionState &state_;
            SafeVector buffer_;
            std::unique_ptr<Botan::HashFunction> crc24_;
        public:
            ArmorReader(SessionState &state);
            ~ArmorReader();
            EmsgResult Read(OutStream &out);
            EmsgResult Finish();
            InBufferStream &GetInStream()
            {
                return in_stm_;
            }
    };
}
