#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "gtest/gtest.h"
#include "message_encryption.h"

using namespace std;
using namespace Botan;

namespace LibEncryptMsg
{
    namespace UnitTests
    {
        class MessageEncryptionFixture : public ::testing::Test
        {
            protected:
                const size_t kBufferSize = 1024;
                vector<uint8_t> plain_file_;
                vector<uint8_t> encrypted_file_;

                virtual void SetUp()
                {
                    const string plain_file_path = "../../test_assets/simple_text.txt";
                    const string encrypted_file_path = "../../test_assets/simple_text.txt.gpg";

                    ifstream stm(plain_file_path);
                    std::copy(istream_iterator<uint8_t>(stm), istream_iterator<uint8_t>(),
                            back_inserter(plain_file_));
                    ifstream stm2(encrypted_file_path);
                    std::copy(istream_iterator<uint8_t>(stm2), istream_iterator<uint8_t>(),
                            back_inserter(encrypted_file_));
                }

                virtual void TearDown()
                {
                }
        };

        TEST_F(MessageEncryptionFixture, When_packet_analyzer_works_Then_headers_are_correct)
        {
            //Arrange
            secure_vector<uint8_t> buf;
            buf.resize(kBufferSize);
            PacketAnalyzer analyzer;
            auto it = plain_file_.begin();

            //Act
            analyzer.Start();
            bool update_result = false;
            while(it != plain_file_.end() && !update_result)
            {
                auto it_next = std::min(it + kBufferSize, plain_file_.end());
                buf.resize(it_next - it);
                std::copy(it, it_next, buf.begin());
                update_result = analyzer.Update(buf);
                it = it_next;
            }

            //Assert
            auto message = analyzer.GetMessageParameters();
            auto security = analyzer.GetSecurityParameters();
            ASSERT_EQ(CipherAlgo::AES256, message.cipher_algo);
            ASSERT_EQ(HashAlgo::SHA256, security.hash_algo);
            ASSERT_EQ(1015808U, security.iterations);
            ostringstream stm;
            auto salt_it = security.salt.begin();
            for(;salt_it != security.salt.end(); salt_it++)
            {
                stm << ios_base::hex << *salt_it;
            }
            string salt = stm.str();
            ASSERT_EQ("04D58C5C071A5B3F", stm.str());
        }
    }
}
