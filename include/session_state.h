#pragma once
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include "botan/secmem.h"
#include "utility.h"
#include "message_parameters.h"
#include "packet_parsers.h"
#include "packet_reader.h"
#include "emsg_utility.h"
#include "message_config.h"
#include "passphrase_provider.h"

namespace LibEncryptMsg
{
    using PacketChain = std::vector<PacketType>;

    // Session state will persist between calls to Update, Context will not
    struct SessionState : public NonCopyableNonMovable
    {
        MessageConfig message_config;
        Salt salt;
        PacketChain packet_chain;
        PacketChain::iterator packet_chain_it;
        SymmetricKeyProvider *key_provider;
        std::unique_ptr<EncryptionKey> encryption_key;
        // Stack of buffer that should be processed from top onwards
        // The invariant is that there are no empty buffers. If a buffer is empty, it should be popped.
        std::stack<Botan::secure_vector<uint8_t> > buffer_stack;
        Botan::secure_vector<uint8_t> output;
        PacketFactory packet_factory;
        PacketResult packet_result;
        bool is_message_analyzed;
        bool finish_packets;

        SessionState();
    };
}
