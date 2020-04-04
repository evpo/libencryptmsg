//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//LightStateMachine is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include <string>
#include <memory>
#include "state_machine_utility.h"
#include "state_machine_type_erasure.h"

namespace LightStateMachine
{
    using StateMachineStateID = int;

    //StateID
    //The caller is recommended to use typed "enum class" and cast to int
    //before interacting with LightStateMachine as below:
    //enum class StateID : StateMachineStateID
    //{
    //    Init,
    //    Stop,
    //};

    //Inherit this class to add the application specific context
    //or use SetExtra and GetExtra<T> to attach it
    class StateMachineContext : public NonCopyable
    {
        private:
            bool failed_;
            TypeErasure extra_context_;
        public:
            bool GetFailed() const;
            void SetFailed(bool value);
            StateMachineContext();

            template<class T>
            StateMachineContext(std::shared_ptr<T> extra_context):
                extra_context_(extra_context)
            {
            }

            template<class T>
            void SetExtra(std::shared_ptr<T> extra_context)
            {
                extra_context_ = extra_context;
            }

            template<class T>
            T &Extra()
            {
                return extra_context_.Get<T>();
            }

            template<class T>
            T &Extra() const
            {
                return extra_context_.Get<T>();
            }
    };

    // Inherit this class to trace application specific state ids
    class StateIDToStringConverter: public NonCopyable
    {
        public:
            virtual std::string Convert(StateMachineStateID state_id) = 0;
            virtual std::string StateMachineName() = 0;
            virtual ~StateIDToStringConverter(){}
    };
}
