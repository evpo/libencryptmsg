//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko 
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#include "openpgp_conversions.h"
#include "botan/pgp_s2k.h"

using namespace Botan;

namespace LibEncryptMsg
{
    size_t DecodeS2KIterations(uint8_t count)
    {
        return OpenPGP_S2K::decode_count(count);
    }

    uint8_t EncodeS2KIterations (size_t iterations)
    {
        return OpenPGP_S2K::encode_count(iterations);
    }
}
