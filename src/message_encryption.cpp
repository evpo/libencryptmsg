#include "message_encryption.h"
#include <vector>
#include <memory>
#include "assert.h"

using namespace std;
using namespace Botan;
namespace
{
    typedef std::streamoff stream_length_type;
    enum class PacketType
    {
        Unknown = -1,
        Header = -2,
        SymmetricKeyESK = 3,
        Symmetric = 9,
        SymmetricIntegProtected = 18,
        Compressed = 8,
        MDC = 19,
        Literal = 11,
    };

    struct PacketHeader
    {
        PacketType packet_type;
        stream_length_type body_length;
        bool is_new_format;
        bool is_partial_length;
        PacketHeader():
            packet_type(PacketType::Unknown),
            body_length(0),
            is_new_format(false),
            is_partial_length(false){}

    };

    class MessagePacket;

    struct PacketStack
    {
        vector<MessagePacket*> packets;
    };
    struct SessionState
    {
        PacketStack packet_stack;
        PacketHeader packet_header;
    };
    class MessagePacket
    {
        protected:
            PacketType packet_type_;
            SessionState &state_;
            virtual void DoUpdate(Botan::secure_vector<uint8_t> &in, Botan::secure_vector<uint8_t> &out) = 0;
        public:
            MessagePacket(SessionState &state):state_(state)
            {
            }
            // Update the stream. After the function:
            // if in is not empty, the excess is a sibling packet.
            // if out is not empty, there is a child packet inside unless it's literal
            void Update(Botan::secure_vector<uint8_t> &in, Botan::secure_vector<uint8_t> &out)
            {
                DoUpdate(in, out);
            }

            PacketType GetPacketType() const
            {
                return packet_type_;
            }
    };

    class PacketHeaderPacket : public MessagePacket
    {
        public:
            PacketHeaderPacket(SessionState &state):MessagePacket(state)
            {
                packet_type_ = PacketType::Header;
            }
        private:
            secure_vector<uint8_t> buffer_;
        protected:
            void DoUpdate(Botan::secure_vector<uint8_t> &in, Botan::secure_vector<uint8_t> &out) final
            {
            }
    };

    std::unique_ptr<SessionState> CreateSessionState()
    {
        std::unique_ptr<SessionState> ret_val(new SessionState());
        ret_val->packet_stack.packets.push_back(new PacketHeaderPacket(*ret_val));
        return ret_val;
    }

    MessagePacket *CreatePacket(SessionState &session_state, PacketType type)
    {
        switch(type)
        {
            case PacketType::Header:
                return new PacketHeaderPacket(session_state);
            default:
                assert(false); // TODO: throw unknown type
                return nullptr;
        }
    }

    void PushPackets(SessionState &session_state, Botan::secure_vector<uint8_t> &buf)
    {
        const unsigned kBufferSize = 1024;
        PacketStack &packet_stack = session_state.packet_stack;
        secure_vector<uint8_t> buf_in;
        secure_vector<uint8_t> buf_out;
        buf_in = buf;
        for(size_t i = 0; i < packet_stack.packets.size(); i++)
        {
            while(!buf_in.empty())
            {
                packet_stack.packets[i]->Update(buf_in, buf_out);
                if(!buf_in.empty() && packet_stack.packets[i]->GetPacketType() == PacketType::Header)
                {
                    // we have just finished header
                    delete packet_stack.packets[i];
                    packet_stack.packets[i] = CreatePacket(session_state, session_state.packet_header.packet_type);
                }
                else if(!buf_in.empty())
                {
                    // we have finished a packet, need a new header
                    delete packet_stack.packets[i];
                    packet_stack.packets[i] = new PacketHeaderPacket(session_state);
                }
            }
            buf_in.swap(buf_out);
        }
        buf.swap(buf_in);
    }

}
namespace LibEncryptMsg
{
    void PacketAnalyzer::Start()
    {
    }
    void PacketAnalyzer::Start(const Passphrase &passphrase)
    {
    }
    void PacketAnalyzer::Start(const EncryptionKey &encryption_key)
    {
    }
    bool PacketAnalyzer::Update(const Botan::secure_vector<uint8_t> &buf, size_t offset)
    {
        return false;
    }
    void PacketAnalyzer::Finish(Botan::secure_vector<uint8_t> &buf, size_t offset)
    {
    }
    const MessageParameters &PacketAnalyzer::GetMessageParameters() const
    {
        return message_parameters_;
    }
    const SecurityParameters &PacketAnalyzer::GetSecurityParameters() const
    {
        return security_parameters_;
    }

    void MessageReader::Start(const Passphrase &passphrase)
    {
    }
    void MessageReader::Start(const EncryptionKey &encryption_key)
    {
    }
    void MessageReader::Update(Botan::secure_vector<uint8_t> &buf, size_t offset)
    {
    }
    void MessageReader::Finish(Botan::secure_vector<uint8_t> &buf, size_t offset)
    {
    }
    const MessageParameters &MessageReader::GetMessageParameters() const
    {
        return message_parameters_;
    }
    const SecurityParameters &MessageReader::GetSecurityParameters() const
    {
        return security_parameters_;
    }

    void MessageWriter::Start(MessageParameters message_parameters, SecurityParameters security_parameters, const EncryptionKey &encryption_key)
    {
    }
    void MessageWriter::Update(Botan::secure_vector<uint8_t>& buf, size_t offset)
    {
    }
    void MessageWriter::Finish(Botan::secure_vector<uint8_t>& buf, size_t offset)
    {
    }
    const MessageParameters &MessageWriter::GetMessageParameters() const
    {
        return message_parameters_;
    }

    // Generates an encryption key and update salt in security parameters if it was empty
    EncryptionKey GenerateEncryptionKey(const Passphrase &passphrase, SecurityParameters &security_parameters)
    {
        return EncryptionKey();
    }
}
