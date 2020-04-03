#include "gtest/gtest.h"
#include "test_helper.h"
#include "armor_writer.h"
#include "armor_reader.h"

namespace EncryptMsg
{
    namespace UnitTests
    {

        class ArmorWriterFixture : public ::testing::Test
        {
            protected:
                std::vector<uint8_t> plain_file_;
                virtual void SetUp() override;
        };

        void ArmorWriterFixture::SetUp()
        {
            LoadFile("simple_text.txt", plain_file_);
        }

        SafeVector Decrypt(SafeVector in)
        {
            SafeVector out;
            auto stm_out = MakeOutStream(out);
            ArmorReader reader;
            reader.GetInStream().Push(in);
            EmsgResult result = reader.Finish(*stm_out);
            assert(result == EmsgResult::Success);
            return out;
        }

        TEST_F(ArmorWriterFixture , When_writing_message_Then_decrypted_output_matches)
        {
            // Arrange

            ArmorWriter writer;
            SafeVector buf(plain_file_.begin(), plain_file_.end());
            writer.GetInStream().Push(buf);
            writer.Start();

            SafeVector out;
            auto out_stm = MakeOutStream(out);

            // Act

            writer.Write(*out_stm, true);

            // Assert

            SafeVector decrypted_buf = Decrypt(out);
            ASSERT_EQ(buf.size(), decrypted_buf.size());
            ASSERT_TRUE(std::equal(buf.begin(), buf.end(), decrypted_buf.begin()));
        }
    }
}
