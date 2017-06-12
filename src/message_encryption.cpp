#include "message_encryption.h"
#include <stack>
using namespace std;
namespace
{
    class MessagePacket;

    class PacketStack
    {
        stack<MessagePacket*> packets;
    };

    class MessagePacket
    {
        protected:
            virtual void DoStart(PacketStack *packet_stack) = 0;
            virtual void DoUpdate(Botan::secure_vector<uint8_t> &buf) = 0;
            virtual void DoFinish(Botan::secure_vector<uint8_t> &buf) = 0;
        public:
            void Start(PacketStack *packet_stack)
            {
                DoStart(packet_stack);
            }
            void Update(Botan::secure_vector<uint8_t> &buf)
            {
                DoUpdate(buf);
            }
            void Finish(Botan::secure_vector<uint8_t> &buf)
            {
                DoFinish(buf);
            }
    };

    class PacketHeader : public MessagePacket
    {
        private:
            PacketStack *packet_stack_;
        protected:
            void DoStart(PacketStack *packet_stack) final
            {
                packet_stack_ = packet_stack;
                packet_stack_->packets.push(this);
            }
    };

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
