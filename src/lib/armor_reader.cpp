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
    static const unsigned int kArmorBlockSize = 4;
    static const std::string kMinHeader = "-----BEGIN PGP ";
    static const std::string kMinTail = "-----END PGP ";
    static const std::string kSeparator = "-----";

}

namespace EncryptMsg
{
    using Result = ArmorReader::ArmorReaderResult;

    ArmorReader::ArmorReader():
        state_(ArmorState::Unknown),
        crc24_(Botan::HashFunction::create_or_throw("CRC24")),
        emsg_result_(EmsgResult::None),
        finish_(false)
    {
    }

    ArmorReader::~ArmorReader()
    {
    }

    void ArmorReader::SetState(ArmorState state)
    {
        LOG_INFO << "Transition to state: " << static_cast<int>(state);
        state_ = state;
    }

    bool ArmorReader::Continue()
    {
        switch(emsg_result_)
        {
            case EmsgResult::Pending:
            case EmsgResult::UnexpectedError:
            case EmsgResult::UnexpectedFormat:
                return false;
            default:
                break;
        }

        switch(state_)
        {
            case ArmorState::Disabled:
            case ArmorState::TailFound:
                return false;
            default:
                break;
        }

        return true;
    }

    EmsgResult ArmorReader::Read(OutStream &out)
    {
        Result result = Result::None;
        emsg_result_ = EmsgResult::None;

        while(Continue())
        {
            switch(state_)
            {
                case ArmorState::Unknown:
                    result = ReadUnknown();
                    switch(result)
                    {
                        case Result::Disabled:
                            emsg_result_ = EmsgResult::Success;
                            SetState(ArmorState::Disabled);
                            break;
                        case Result::Pending:
                            emsg_result_ = EmsgResult::Pending;
                            break;
                        case Result::MinHeader:
                            SetState(ArmorState::BeginHeader);
                            break;
                        default:
                            emsg_result_ = EmsgResult::UnexpectedError;
                            assert(false);
                            break;
                    }
                    break;
                case ArmorState::BeginHeader:
                    result = ReadBeginHeader();
                    switch(result)
                    {
                        case Result::UnexpectedFormat:
                            emsg_result_ = EmsgResult::UnexpectedFormat;
                            break;
                        case Result::Pending:
                            emsg_result_ = EmsgResult::Pending;
                            break;
                        case Result::Success:
                            SetState(ArmorState::Header);
                            break;
                        default:
                            emsg_result_ = EmsgResult::UnexpectedError;
                            assert(false);
                            break;

                    }
                    break;
                case ArmorState::Header:
                    result = ReadHeader();
                    switch(result)
                    {
                        case Result::UnexpectedFormat:
                            emsg_result_ = EmsgResult::UnexpectedFormat;
                            break;
                        case Result::Pending:
                            emsg_result_ = EmsgResult::Pending;
                            break;
                        case Result::Success:
                            SetState(ArmorState::Payload);
                            break;
                        default:
                            emsg_result_ = EmsgResult::UnexpectedError;
                            assert(false);
                            break;
                    }
                    break;
                case ArmorState::Payload:
                    result = ReadPayload(out);
                    switch(result)
                    {
                        case Result::UnexpectedFormat:
                            emsg_result_ = EmsgResult::UnexpectedFormat;
                            break;
                        case Result::Pending:
                            emsg_result_ = EmsgResult::Pending;
                            break;
                        case Result::Success:
                            SetState(ArmorState::CRC);
                            break;
                        default:
                            emsg_result_ = EmsgResult::UnexpectedError;
                            assert(false);
                            break;
                    }
                    break;

                case ArmorState::CRC:
                    result = ReadCRC();
                    switch(result)
                    {
                        case Result::UnexpectedFormat:
                            emsg_result_ = EmsgResult::UnexpectedFormat;
                            break;
                        case Result::Pending:
                            emsg_result_ = EmsgResult::Pending;
                            break;
                        case Result::Success:
                            SetState(ArmorState::Tail);
                            break;
                        default:
                            emsg_result_ = EmsgResult::UnexpectedError;
                            assert(false);
                            break;

                    }
                    break;
                case ArmorState::Tail:
                    result = ReadTail();
                    switch(result)
                    {
                        case Result::UnexpectedFormat:
                            emsg_result_ = EmsgResult::UnexpectedFormat;
                            break;
                        case Result::Pending:
                            emsg_result_ = EmsgResult::Pending;
                            break;
                        case Result::Success:
                            emsg_result_ = EmsgResult::Success;
                            SetState(ArmorState::TailFound);
                            break;
                        default:
                            emsg_result_ = EmsgResult::UnexpectedError;
                            assert(false);
                            break;
                    }
                    break;
                default:
                    emsg_result_ = EmsgResult::UnexpectedError;
                    assert(false);
                    break;
            }
        }

        if(emsg_result_ == EmsgResult::None)
            emsg_result_ = EmsgResult::UnexpectedError;

        return emsg_result_;
    }

    Result ArmorReader::ReadUnknown()
    {
        // when we finish, we return the data to in_stm_
        // in other states we keep the data in buffer_

        if(in_stm_.GetCount() < kMinHeader.size())
        {
            if(!finish_)
                return Result::Pending;
        }

        SafeVector received(kMinHeader.size(), 0);
        in_stm_.Read(received.data(), received.size());
        SafeVector remainder;
        AppendToBuffer(in_stm_, remainder);
        in_stm_.Push(received);
        in_stm_.Push(remainder);
        return std::equal(kMinHeader.begin(), kMinHeader.end(), received.begin()) ?
            Result::MinHeader : Result::Disabled;
    }

    Result ArmorReader::ReadBeginHeader()
    {
        auto line_result = NextLine();
        if(line_result.max_length_reached)
            return Result::UnexpectedFormat;
        if(!line_result.success)
            return Result::Pending;

        auto &line = line_result.line;
        assert(line.size() >= kMinHeader.size());
        int label_size = static_cast<int>(line.size()) - kMinHeader.size() - kSeparator.size();
        if(label_size < 0 || label_size > static_cast<int>(kMaxLineLength))
            return Result::UnexpectedFormat;

        auto label_it = line.begin() + kMinHeader.size();
        label_.assign(label_it, label_it + label_size);
        LOG_INFO << "label : " << label_;
        return std::equal(kSeparator.begin(), kSeparator.end(), label_it + label_size) ?
            Result::Success : Result::UnexpectedFormat;
    }

    Result ArmorReader::ReadHeader()
    {
        auto line_result = NextLine();
        while(line_result.success)
        {
            if(std::all_of(line_result.line.begin(), line_result.line.end(), is_space))
                return Result::Success;
            line_result = NextLine();
        }

        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }

        return Result::Pending;
    }

    Result ArmorReader::ReadPayload(OutStream &out)
    {
        using namespace Botan;
        auto line_result = NextLine();
        while(line_result.success)
        {
            auto &line = line_result.line;
            if(line.size() > 0 && line[0] == '=')
            {
                line.push_back('\n');
                buffer_.insert(
                        buffer_.begin(),
                        line.begin(),
                        line.end());
                return Result::Success;
            }
            else if(line.size() % 4 != 0)
            {
                return Result::UnexpectedFormat;
            }
            else
            {
                std::vector<uint8_t> buf(base64_decode_max_output(line.size()), 0);
                size_t decode_res = base64_decode(buf.data(),
                        reinterpret_cast<char*>(line.data()), line.size(), false);
                buf.resize(decode_res);
                crc24_->update(buf.data(), buf.size());
                out.Write(buf.data(), buf.size());
            }
            line_result = NextLine();
        }

        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }

        return Result::Pending;
    }

    Result ArmorReader::ReadCRC()
    {
        using namespace Botan;
        auto line_result = NextLine();
        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }

        if(!line_result.success)
            return Result::Pending;

        if(line_result.line.size() != kArmorBlockSize + 1 || line_result.line[0] != '=')
            return Result::UnexpectedFormat;
        auto it = line_result.line.begin();
        it ++;
        std::string crc_str(it, line_result.line.end());
        auto is_valid = ValidateCRC(crc_str);
        if(!is_valid)
        {
            LOG_WARNING << "CRC is invalid";
            return Result::UnexpectedFormat;
        }
        return Result::Success;
    }

    Result ArmorReader::ReadTail()
    {
        auto line_result = NextLine();
        if(line_result.max_length_reached)
        {
            LOG_WARNING << "UnexpectedFormat: armor line is too long";
            return Result::UnexpectedFormat;
        }
        if(!line_result.success)
            return Result::Pending;

        std::string tail = kMinTail + label_ + kSeparator;
        bool valid = (line_result.line.size() == tail.size());
        valid = valid && std::equal(line_result.line.begin(), line_result.line.end(),
                tail.begin());

        if(!valid)
        {
            LOG_WARNING << "UnexpectedFormat: tail is invalid";
            return Result::UnexpectedFormat;
        }

        return Result::Success;
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

    EmsgResult ArmorReader::Finish(OutStream &out)
    {
        finish_ = true;
        if(state_ == ArmorState::TailFound)
        {
            emsg_result_ = EmsgResult::Success;
            return emsg_result_;
        }

        emsg_result_ = Read(out);

        if(emsg_result_ != EmsgResult::Success)
            return emsg_result_;

        if(state_ != ArmorState::TailFound)
        {
            LOG_WARNING << "UnexpectedError: invalid state after finish";
            return EmsgResult::UnexpectedError;
        }
        return EmsgResult::Success;
    }

    ArmorReader::LineResult ArmorReader::NextLine()
    {
        ArmorReader::LineResult result;
        PushBackToBuffer(in_stm_, buffer_);
        auto it = std::find(buffer_.begin(), buffer_.end(), '\n');
        if(it == buffer_.end())
        {
            if(buffer_.size() > kMaxLineLength)
                result.max_length_reached = true;
            return result;
        }

        result.success = true;
        result.line.assign(buffer_.begin(), it);
        it ++;
        buffer_.erase(buffer_.begin(), it);
        return result;
    }

}
