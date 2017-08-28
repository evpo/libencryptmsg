#include "message_decryption.h"
#include "graph_builder.h"
#include "state_machine.h"
#include "session_state.h"
#include "emsg_symmetric_key.h"

using namespace std;
using namespace LightStateMachine;
using namespace LightStateMachine::Client;
using namespace LibEncryptMsg;

namespace LightStateMachine
{
    class StateMachine;
    namespace Client
    {
        class Context;
    }
}

namespace
{
    class KeyFromPassphraseProvider : public SymmetricKeyProvider
    {
        private:
            PassphraseProvider &passphrase_provider_;
        public:
            KeyFromPassphraseProvider(PassphraseProvider &passphrase_provider);

            std::unique_ptr<EncryptionKey> GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt,
                    std::string description, bool &canceled) override;
    };

    class KnownKeyProvider : public SymmetricKeyProvider
    {
        private:
            std::unique_ptr<EncryptionKey> key_;
        public:
            KnownKeyProvider(std::unique_ptr<EncryptionKey> key);

            std::unique_ptr<EncryptionKey> GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt,
                    std::string description, bool &canceled) override;
    };

    class PassThroughKeyProvider : public SymmetricKeyProvider
    {
        private:
            SymmetricKeyProvider &provider_;
        public:
            PassThroughKeyProvider(SymmetricKeyProvider &provider): provider_(provider)
            {
            }

            std::unique_ptr<EncryptionKey> GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt,
                    std::string description, bool &canceled) override
            {
                return provider_.GetKey(cipher_algo, hash_algo, iterations, salt, description, canceled);
            }
    };

    class PassThroughPassphraseProvider : public PassphraseProvider
    {
        private:
            PassphraseProvider &passphrase_provider_;
        public:
            PassThroughPassphraseProvider(PassphraseProvider &passphrase_provider)
                :passphrase_provider_(passphrase_provider)
            {
            }

            std::unique_ptr<SecureVector> GetPassphrase(std::string description, bool &canceled) override
            {
                return passphrase_provider_.GetPassphrase(description, canceled);
            }
    };

    class KnownPassphraseProvider : public PassphraseProvider
    {
        private:
            std::unique_ptr<SecureVector> passphrase_data_;
        public:
            KnownPassphraseProvider(std::unique_ptr<SecureVector> passphrase_data);
            std::unique_ptr<SecureVector> GetPassphrase(std::string description, bool &canceled) override;
    };

}

namespace LibEncryptMsg
{

    class MessageReaderImpl
    {
        private:
            SessionState session_state_;
            StateGraphInfo graph_info_;
            LightStateMachine::Client::Context context_;
            LightStateMachine::StateMachine state_machine_;
            PacketFactory packet_factory_;
            bool analyze_only_;
            std::unique_ptr<SymmetricKeyProvider> key_provider_;
            std::unique_ptr<PassphraseProvider> passphrase_provider_;
        public:
            void Start();
            void Start(const SecureVector &passphrase);
            void Start(std::unique_ptr<SecureVector> passphrase);
            void Start(PassphraseProvider &passphrase_provider);

            // For known keys
            void Start(SymmetricKeyProvider &key_provider);
            void Start(const EncryptionKey &encryption_key);
            void Start(std::unique_ptr<EncryptionKey> encryption_key);

            // Fo reader
            void Update(SecureVector &buf);
            void Finish(SecureVector &buf);

            // For analyzer
            bool Update(const SecureVector &buf);
            bool Finish(const SecureVector &buf);

            const MessageConfig &GetMessageConfig() const;
            const EncryptionKey &GetEncryptionKey() const;
            const Salt &GetSalt() const;

            MessageReaderImpl(bool analyze_only);
    };

    MessageReaderImpl::MessageReaderImpl(bool analyze_only):
        graph_info_(BuildStateGraph()),
        state_machine_(*graph_info_.state_graph, graph_info_.start_node, graph_info_.fail_node, context_),
        packet_factory_(session_state_), analyze_only_(analyze_only)
    {
        context_.SetState(session_state_);
    }

    void MessageReaderImpl::Start()
    {
        Start(SecureVector());
    }

    void MessageReaderImpl::Start(const SecureVector &passphrase)
    {
        std::unique_ptr<SecureVector> data_uptr(new SecureVector(passphrase));
        passphrase_provider_.reset(new KnownPassphraseProvider(std::move(data_uptr)));
        key_provider_.reset(new KeyFromPassphraseProvider(*passphrase_provider_));
        session_state_.key_provider = key_provider_.get();
    }

    void MessageReaderImpl::Start(std::unique_ptr<SecureVector> passphrase)
    {
        passphrase_provider_.reset(new KnownPassphraseProvider(std::move(passphrase)));
        key_provider_.reset(new KeyFromPassphraseProvider(*passphrase_provider_));
        session_state_.key_provider = key_provider_.get();
    }

    void MessageReaderImpl::Start(PassphraseProvider &passphrase_provider)
    {
        passphrase_provider_.reset(new PassThroughPassphraseProvider(passphrase_provider));
        key_provider_.reset(new KeyFromPassphraseProvider(*passphrase_provider_));
        session_state_.key_provider = key_provider_.get();
    }

    // For known keys
    void MessageReaderImpl::Start(SymmetricKeyProvider &key_provider)
    {
        key_provider_.reset(new PassThroughKeyProvider(key_provider));
        session_state_.key_provider = key_provider_.get();
    }

    void MessageReaderImpl::Start(const EncryptionKey &encryption_key)
    {
        std::unique_ptr<EncryptionKey> key_uptr(new EncryptionKey(encryption_key));
        key_provider_.reset(new KnownKeyProvider(std::move(key_uptr)));
        session_state_.key_provider = key_provider_.get();
    }

    void MessageReaderImpl::Start(std::unique_ptr<EncryptionKey> encryption_key)
    {
        key_provider_.reset(new KnownKeyProvider(std::move(encryption_key)));
        session_state_.key_provider = key_provider_.get();
    }

    void MessageReaderImpl::Update(SecureVector &buf)
    {
        if(buf.empty())
            return;
        session_state_.buffer_stack.push(buf);
        state_machine_.Reset();
        while(state_machine_.NextState())
        {
        }

        if(context_.GetFailed() && (session_state_.packet_result != PacketResult::InvalidPassphrase || !analyze_only_))
            throw EmsgException(session_state_.packet_result);

        buf.swap(session_state_.output);
        session_state_.output.clear();
    }

    bool MessageReaderImpl::Update(const SecureVector &buf)
    {
        SecureVector buf2(buf);
        Update(buf2);
        return session_state_.is_message_analyzed;
    }

    void MessageReaderImpl::Finish(SecureVector &buf)
    {
        Update(buf);
        session_state_.finish_packets = true;
        state_machine_.Reset();
        while(state_machine_.NextState())
        {
        }
        if(context_.GetFailed() && (session_state_.packet_result != PacketResult::InvalidPassphrase || !analyze_only_))
            throw EmsgException(session_state_.packet_result);
        buf.insert(buf.end(), session_state_.output.begin(), session_state_.output.end());
        session_state_.output.clear();
    }

    bool MessageReaderImpl::Finish(const SecureVector &buf)
    {
        SecureVector buf2(buf);
        Finish(buf2);
        return session_state_.is_message_analyzed;
    }

    const EncryptionKey &MessageReaderImpl::GetEncryptionKey() const
    {
        return *session_state_.encryption_key;
    }

    const MessageConfig &MessageReaderImpl::GetMessageConfig() const
    {
        return session_state_.message_config;
    }

    const Salt &MessageReaderImpl::GetSalt() const
    {
        return session_state_.salt;
    }

    // Analyzer
    PacketAnalyzer::PacketAnalyzer():
        impl_(new MessageReaderImpl(true))
    {
    }

    PacketAnalyzer::~PacketAnalyzer()
    {
        delete impl_;
    }

    void PacketAnalyzer::Start()
    {
        impl_->Start();
    }

    void PacketAnalyzer::Start(SecureVector passphrase)
    {
        impl_->Start(passphrase);
    }

    void PacketAnalyzer::Start(std::unique_ptr<SecureVector> passphrase)
    {
        impl_->Start(std::move(passphrase));
    }

    void PacketAnalyzer::Start(PassphraseProvider &passphrase_provider)
    {
        impl_->Start(passphrase_provider);
    }

    // For known keys
    void PacketAnalyzer::Start(SymmetricKeyProvider &key_provider)
    {
        impl_->Start(key_provider);
    }

    void PacketAnalyzer::Start(EncryptionKey encryption_key)
    {
        impl_->Start(encryption_key);
    }

    void PacketAnalyzer::Start(std::unique_ptr<EncryptionKey> encryption_key)
    {
        impl_->Start(std::move(encryption_key));
    }

    bool PacketAnalyzer::Update(const SecureVector &buf)
    {
        return impl_->Update(buf);
    }

    bool PacketAnalyzer::Finish(const SecureVector &buf)
    {
        return impl_->Finish(buf);
    }

    const EncryptionKey &PacketAnalyzer::GetEncryptionKey() const
    {
        return impl_->GetEncryptionKey();

    }

    const MessageConfig &PacketAnalyzer::GetMessageConfig() const
    {
        return impl_->GetMessageConfig();
    }

    const Salt &PacketAnalyzer::GetSalt() const
    {
        return impl_->GetSalt();
    }

    // MessageReader
    MessageReader::MessageReader():impl_(new MessageReaderImpl(false))
    {
    }

    MessageReader::~MessageReader()
    {
        delete impl_;
    }

    void MessageReader::Start(SecureVector passphrase)
    {
        impl_->Start(passphrase);
    }

    void MessageReader::Start(std::unique_ptr<SecureVector> passphrase)
    {
        impl_->Start(std::move(passphrase));
    }

    void MessageReader::Start(PassphraseProvider &passphrase_provider)
    {
        impl_->Start(passphrase_provider);
    }

    // For known keys
    void MessageReader::Start(SymmetricKeyProvider &key_provider)
    {
        impl_->Start(key_provider);
    }

    void MessageReader::Start(EncryptionKey encryption_key)
    {
        impl_->Start(encryption_key);
    }

    void MessageReader::Start(std::unique_ptr<EncryptionKey> encryption_key)
    {
        impl_->Start(std::move(encryption_key));
    }

    void MessageReader::Update(SecureVector &buf)
    {
        impl_->Update(buf);
    }

    void MessageReader::Finish(SecureVector &buf)
    {
        impl_->Finish(buf);
    }

    const EncryptionKey &MessageReader::GetEncryptionKey() const
    {
        return impl_->GetEncryptionKey();
    }

    const MessageConfig &MessageReader::GetMessageConfig() const
    {
        return impl_->GetMessageConfig();
    }

    const Salt &MessageReader::GetSalt() const
    {
        return impl_->GetSalt();
    }
}

namespace
{
    KeyFromPassphraseProvider::KeyFromPassphraseProvider(PassphraseProvider &passphrase_provider)
        :passphrase_provider_(passphrase_provider)
    {
    }

    std::unique_ptr<EncryptionKey> KeyFromPassphraseProvider::GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo,
            uint8_t iterations, Salt salt, std::string description, bool &canceled)
    {
        auto data_uptr = passphrase_provider_.GetPassphrase(description, canceled);
        if(!data_uptr)
        {
            return std::unique_ptr<EncryptionKey>();
        }
        Passphrase pwd(*data_uptr);
        if(canceled)
            return std::unique_ptr<EncryptionKey>();

        return GenerateEncryptionKey(pwd, cipher_algo, hash_algo, iterations, salt);
    }

    KnownPassphraseProvider::KnownPassphraseProvider(std::unique_ptr<SecureVector> passphrase_data)
        :passphrase_data_(std::move(passphrase_data))
    {
    }

    std::unique_ptr<SecureVector> KnownPassphraseProvider::GetPassphrase(std::string description, bool &canceled)
    {
        (void)description;
        canceled = false;
        std::unique_ptr<SecureVector> ret_val;
        if(passphrase_data_->size() == 0)
            return ret_val;

        ret_val.reset(new SecureVector(*passphrase_data_));
        return ret_val;
    }

    KnownKeyProvider::KnownKeyProvider(std::unique_ptr<EncryptionKey> key)
        :key_(std::move(key))
    {

    }

    std::unique_ptr<EncryptionKey> KnownKeyProvider::GetKey(CipherAlgo cipher_algo, HashAlgo hash_algo, uint8_t iterations, Salt salt,
            std::string description, bool &canceled)
    {
        (void)cipher_algo;
        (void)hash_algo;
        (void)iterations;
        (void)salt;
        (void)description;
        canceled = false;
        return std::unique_ptr<EncryptionKey>(new EncryptionKey(*key_));
    }
}
