//**********************************************************************************
//EncryptPad Copyright 2018 Evgeny Pokhilko 
//<https://evpo.net/libencryptmsg>
//
//libencryptmsg is released under the Simplified BSD License (see license.txt)
//**********************************************************************************
#pragma once
#include "botan/secmem.h"

namespace LibEncryptMsg
{
    using SafeVector = Botan::secure_vector<uint8_t>;
}
