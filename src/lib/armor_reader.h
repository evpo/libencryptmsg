#pragma once
#include "memory_stream.h"
#include "emsg_types.h"
#include "packet_reader.h"
#include <vector>

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

    class ArmorReader : public PacketRWBase
    {
        public:
            ArmorReader(SessionState &state)
                :PacketRWBase(state, false) // final_packet = false
            {
            }
        protected:
            EmsgResult DoRead(OutStream &out) override;
            EmsgResult DoFinish() override;
    };
}
