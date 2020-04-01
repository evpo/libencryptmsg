#include "armor_reader.h"
#include <vector>
#include <algorithm>
#include "botan/hash.h"
#include "botan/base64.h"
#include "plog/Log.h"
#include "session_state.h"
#include "assert.h"


namespace
{
    bool is_space(char c)
    {
        return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
    }
}

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

        if(state_.armor_status == ArmorStatus::Unknown)
        {
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

            assert(buffer_.size() == 0);
            AppendToBuffer(in_stm_, buffer_);
            buffer_.insert(buffer_.begin(), received.begin(), received.end());
        }

        AppendToBuffer(in_stm_, buffer_);
        bool empty_line_found = false;
        auto it = std::find(buffer_.begin(), buffer_.end(), '\n');
        while(it != buffer_.end())
        {
            std::vector<uint8_t> line(buffer_.begin(), it);
            //for logging only
            std::string line_str(line.begin(), line.end());
            LOG_INFO << "found line: " << line_str;
            if(std::all_of(line.begin(), line.end(), is_space))
            {
                empty_line_found = true;
            }

            it ++;
            buffer_.erase(buffer_.begin(), it);
            if(empty_line_found)
                break;

            it = std::find(buffer_.begin(), buffer_.end(), '\n');
        }

        if(empty_line_found)
        {
            in_stm_.Push(buffer_);
            buffer_.clear();
            state_.armor_status = ArmorStatus::Payload;
            return EmsgResult::Success;
        }
        else
        {
            return EmsgResult::Pending;
        }
    }

    EmsgResult ArmorReader::Read(OutStream &out)
    {
        using namespace Botan;
        PushBackToBuffer(in_stm_, buffer_);
        auto it = std::find(buffer_.begin(), buffer_.end(), '\n');
        bool is_crc_found = false;
        std::string line;
        while(it != buffer_.end())
        {
            line.assign(buffer_.begin(), it);
            LOG_INFO << "line= " << line;
            it++;
            buffer_.erase(buffer_.begin(), it);
            if(line.size() > 1 && line[0] == '=' && line[1] != '=')
            {
                LOG_INFO << "CRC found";
                is_crc_found = true;
                break;
            }
            assert(line.size() % 4 == 0);
            auto decoded_buf = base64_decode(line, false);
            out.Write(decoded_buf.data(), decoded_buf.size());

            it = std::find(buffer_.begin(), buffer_.end(), '\n');
        }
        if(is_crc_found)
        {
            LOG_INFO << "CRC: " << line;
            return EmsgResult::UnexpectedError;
        }
        return EmsgResult::Success;
    }

    EmsgResult ArmorReader::Finish()
    {
        return EmsgResult::UnexpectedError;
    }

    ArmorReader::ArmorReader(SessionState &state):
        state_(state),
        crc24_(Botan::HashFunction::create_or_throw("CRC24"))
    {
    }

    ArmorReader::~ArmorReader()
    {
    }
}
