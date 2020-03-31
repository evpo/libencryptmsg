#include "armor_reader.h"
#include <algorithm>
#include "session_state.h"


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
        std::vector<uint8_t> received(kMinHeader.size(), 0);
        in_stm_.Read(received.data(), received.size());
        if(!std::equal(kMinHeader.begin(), kMinHeader.end(), received.begin(), received.end()))
        {
            SafeVector buf1(received.data(), received.data() + received.size());
            SafeVector buf2(in_stm_.GetCount(), 0);
            in_stm_.Read(buf2.data(), buf2.size());
            state_.buffer_stack.push(buf2);
            state_.buffer_stack.push(buf1);
            state_.armor_status = ArmorStatus::Disabled;
            return EmsgResult::Success;
        }
        else
        {
            state_.armor_status = ArmorStatus::Header;
            // implement
            assert(false);
        }
    }

    EmsgResult ArmorReader::DoRead(OutStream &out)
    {

    }

    EmsgResult ArmorReader::DoFinish()
    {

    }
}
