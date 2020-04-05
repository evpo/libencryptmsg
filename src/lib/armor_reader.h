#pragma once
#include "memory_stream.h"
#include "emsg_types.h"
#include "packet_reader.h"
#include <vector>
#include <string>

namespace Botan
{
    class HashFunction;
    class Pipe;
}
namespace LightStateMachine
{
    class StateMachineContext;
    class StateMachine;

}
namespace EncryptMsg
{
    enum class ArmorStatus
    {
        Unknown,
        Disabled,
        Enabled,
    };

    class ArmorReaderImpl;
    class ArmorReader final : public NonCopyableNonMovable
    {
        private:
            std::shared_ptr<ArmorReaderImpl> pimpl_;
            std::unique_ptr<LightStateMachine::StateMachineContext> context_;
            std::unique_ptr<LightStateMachine::StateMachine> state_machine_;
        public:
            ArmorReader();
            ~ArmorReader();
            EmsgResult Read(OutStream &out);
            EmsgResult Finish(OutStream &out);
            InBufferStream &GetInStream();
            ArmorStatus GetStatus() const;
    };
}
