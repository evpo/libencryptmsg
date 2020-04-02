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
    static const unsigned int kMaxLineLength = 76;
    static const std::string kMinHeader = "-----BEGIN PGP ";
    static const std::string kMinFooter = "-----END PGP ";
    static const std::string kSeparator = "-----";

}

namespace EncryptMsg
{
    EmsgResult ArmorHeaderReader::Read(bool finish_packets)
    {
        if(in_stm_.GetCount() < kMinHeader.size() && !finish_packets)
            return EmsgResult::Pending;

        if(finish_packets && in_stm_.GetCount() < kMinHeader.size())
        {
            context_.status = ArmorStatus::Disabled;
            return EmsgResult::Success;
        }

        if(context_.status == ArmorStatus::Unknown)
        {
            SafeVector received(kMinHeader.size(), 0);
            in_stm_.Read(received.data(), received.size());
            if(!std::equal(kMinHeader.begin(), kMinHeader.end(), received.begin()))
            {
                SafeVector remainder;
                AppendToBuffer(in_stm_, remainder);
                in_stm_.Push(received);
                in_stm_.Push(remainder);
                context_.status = ArmorStatus::Disabled;
                return EmsgResult::Success;
            }
            context_.status = ArmorStatus::Header;

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
            if(context_.label.size() == 0)
            {
                //this must be the first line
                assert(line_str.size() >= kMinHeader.size());
                int label_size = static_cast<int>(line_str.size()) - kMinHeader.size() - kSeparator.size();
                if(label_size < 0 || label_size > static_cast<int>(kMaxLineLength))
                    return EmsgResult::UnexpectedFormat;

                auto label_it = line_str.begin() + kMinHeader.size();
                context_.label.assign(label_it, label_it + label_size);
                LOG_INFO << "label : " << context_.label;
            }

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
            context_.status = ArmorStatus::Payload;
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

        // we should be in Payload status
        if(context_.status != ArmorStatus::Payload)
            return EmsgResult::UnexpectedError;

        if(is_footer_found_ && in_stm_.GetCount() > 0)
            return EmsgResult::UnexpectedFormat;

        std::string footer = kMinFooter + context_.label + kSeparator;

        PushBackToBuffer(in_stm_, buffer_);
        auto it = std::find(buffer_.begin(), buffer_.end(), '\n');
        bool is_footer_found = false;
        std::string line;
        while(it != buffer_.end())
        {
            line.assign(buffer_.begin(), it);
            it++;
            buffer_.erase(buffer_.begin(), it);
            if(line.size() > 0 && line[0] == '=')
            {
                // Another CRC
                if(received_crc_.size() > 0)
                    return EmsgResult::UnexpectedFormat;

                line.erase(line.begin());
                received_crc_ = line;
                if(received_crc_.size() == 0 || !ValidateCRC(received_crc_))
                    return EmsgResult::UnexpectedFormat;
            }
            else if(received_crc_.size() > 0 && line == footer)
            {
                is_footer_found = true;
                break;
            }
            else
            {
                // Only footer should follow CRC
                if(received_crc_.size() > 0)
                    return EmsgResult::UnexpectedFormat;

                if(line.size() % 4 != 0)
                    return EmsgResult::UnexpectedFormat;

                auto decoded_buf = base64_decode(line, false);
                crc24_->update(decoded_buf.data(), decoded_buf.size());
                out.Write(decoded_buf.data(), decoded_buf.size());
            }

            it = std::find(buffer_.begin(), buffer_.end(), '\n');
        }

        if(buffer_.size() > kMaxLineLength)
            return EmsgResult::UnexpectedFormat;

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
        // we should be in Payload status
        if(context_.status != ArmorStatus::Payload)
            return EmsgResult::UnexpectedError;
        if(!is_footer_found_)
            return EmsgResult::UnexpectedFormat;
        return EmsgResult::Success;
    }

    ArmorReader::ArmorReader(ArmorContext &context):
        context_(context),
        crc24_(Botan::HashFunction::create_or_throw("CRC24")),
        is_footer_found_(false)
    {
    }

    ArmorReader::~ArmorReader()
    {
    }
}
