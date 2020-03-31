#include "armor_reader.h"
#include <algorithm>
#include "session_state.h"
#include "assert.h"

namespace EncryptMsg
{
    EmsgResult ArmorHeaderReader::Read(bool finish_packets)
    {
        static const std::string kMinHeader = "-----BEGIN PGP MESSAGE-----";

        if(in_stm_.GetCount() < kMinHeader.size() && !finish_packets)
            return EmsgResult::Pending;

        if(finish_packets && in_stm_.GetCount() < kMinHeader.size())
        {
            state_.armor_status = ArmorStatus::Disabled;
            return EmsgResult::Success;
        }
        SafeVector received(kMinHeader.size(), 0);
        in_stm_.Read(received.data(), received.size());
        if(!std::equal(kMinHeader.begin(), kMinHeader.end(), received.begin()))
        {
            SafeVector remainder;
            AppendToBuffer(in_stm_, remainder);
            in_stm_.Push(received);
            in_stm_.Push(remainder);
            state_.armor_status = ArmorStatus::Disabled;
            return EmsgResult::Success;
        }

        state_.armor_status = ArmorStatus::Header;
        // implement
        assert(false);
        return EmsgResult::UnexpectedError;
    }

    EmsgResult ArmorReader::DoRead(OutStream &out)
    {
        return EmsgResult::UnexpectedError;
    }

    EmsgResult ArmorReader::DoFinish()
    {
        return EmsgResult::UnexpectedError;
    }
}
