#pragma once
#include "botan/secmem.h"
#include "assert.h"
#include "emsg_utility.h"
#include "emsg_types.h"
#include <algorithm>

namespace LibEncryptMsg
{
    class InBufferStream;

    class InBufferStream : public NonCopyableNonMovable
    {
        private:
            size_t count_;
            Botan::secure_vector<uint8_t> buffer_;
            Botan::secure_vector<uint8_t> length_buffer_;
            uint8_t *begin_;
            uint8_t *end_;
            uint8_t *pos_;
            bool partial_length_;

            size_t GetPartialCount() const;

            void SetPartialCount(size_t count);

            Botan::secure_vector<uint8_t>::const_iterator ReadLength(Botan::secure_vector<uint8_t>::const_iterator it, Botan::secure_vector<uint8_t>::const_iterator end);

            // Reads from the source directly without updating counters.
            size_t ReadFromSource(uint8_t *out_it, size_t length);
        public:
            InBufferStream();

            bool GetPartialLength() const;

            // Set partial length flag and set the length if it's already read
            void SetPartialLength(bool flag, size_t length = 0);

            size_t GetCount() const;

            void Push(Botan::secure_vector<uint8_t> &buf);

            bool IsEOF() const;

            // Check for -1 to see if the stream is depleted
            int Get();

            // Reads bytes2read bytes to out_it iterator
            size_t Read(uint8_t *out_it, size_t bytes2read);
    };

    class OutStream : public NonCopyableNonMovable
    {
    private:
        Botan::secure_vector<uint8_t> &out_;
        uint8_t *ptr_;
        void Resize(size_t size);
    public:
        OutStream(Botan::secure_vector<uint8_t> &out);
        void Reset();
        bool Put(uint8_t b);
        bool Write(const uint8_t *in_it, size_t bytes2write);
    };

    std::unique_ptr<OutStream> MakeOutStream(Botan::secure_vector<uint8_t> &cnt);

    void AppendToBuffer(InBufferStream &stm, Botan::secure_vector<uint8_t> &buf);
}
