//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<https://evpo.net/libencryptmsg>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
namespace LibEncryptMsg
{
    class NonCopyable
    {
        public:
            NonCopyable(){}
            NonCopyable(const NonCopyable &) = delete;
            const NonCopyable &operator=(const NonCopyable&) = delete;
    };
}
