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
            state_.message_config.SetArmor(false);
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
                state_.message_config.SetArmor(false);
                return EmsgResult::Success;
            }
            state_.armor_status = ArmorStatus::Header;
            state_.message_config.SetArmor(true);

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
        static const std::string kFooter = "-----END PGP MESSAGE-----";
        static const unsigned int kMaxLineLength = 76;
        if(is_footer_found_ && in_stm_.GetCount() > 0)
            return EmsgResult::UnexpectedFormat;

        PushBackToBuffer(in_stm_, buffer_);
        auto it = std::find(buffer_.begin(), buffer_.end(), '\n');
        bool is_crc_found = false;
        bool is_footer_found = false;
        std::string line;
        while(it != buffer_.end())
        {
            line.assign(buffer_.begin(), it);
            it++;
            buffer_.erase(buffer_.begin(), it);
            if(line.size() > 0 && line[0] == '=')
            {
                is_crc_found = true;
                break;
            }

            if(line == kFooter)
            {
                is_footer_found = true;
                break;
            }

            if(received_crc_.size() > 0 || line.size() % 4 != 0)
            {
                return EmsgResult::UnexpectedFormat;
            }
            auto decoded_buf = base64_decode(line, false);
            crc24_->update(decoded_buf.data(), decoded_buf.size());
            out.Write(decoded_buf.data(), decoded_buf.size());

            it = std::find(buffer_.begin(), buffer_.end(), '\n');
        }

        if(buffer_.size() > kMaxLineLength)
            return EmsgResult::UnexpectedFormat;

        if(is_crc_found)
        {
            // Another CRC
            if(received_crc_.size() > 0)
                return EmsgResult::UnexpectedFormat;

            line.erase(line.begin());
            received_crc_ = line;
            if(received_crc_.size() == 0 || !ValidateCRC(received_crc_))
                return EmsgResult::UnexpectedFormat;
        }

        if(is_footer_found)
        {
            if(received_crc_.size() == 0)
                return EmsgResult::UnexpectedFormat;

            is_footer_found_ = true;
            in_stm_.Push(buffer_);
            buffer_.clear();
        }
        return EmsgResult::Success;
    }

    bool ArmorReader::ValidateCRC(const std::string &crc)
    {
        using namespace Botan;
        auto computed_crc = crc24_->final();
        std::string computed_crc_str(base64_encode_max_output(computed_crc.size()), 0);
        size_t consumed = 0;
        size_t written = base64_encode(&computed_crc_str.front(), computed_crc.data(),
                computed_crc.size(), consumed, true);
        assert(written == computed_crc_str.size());
        assert(crc.size() > 0);
        return crc == computed_crc_str;
    }

    EmsgResult ArmorReader::Finish()
    {
        if(!is_footer_found_)
            return EmsgResult::UnexpectedFormat;
        return EmsgResult::Success;
    }

    ArmorReader::ArmorReader(SessionState &state):
        state_(state),
        crc24_(Botan::HashFunction::create_or_throw("CRC24")),
        is_footer_found_(false)
    {
    }

    ArmorReader::~ArmorReader()
    {
    }
}
