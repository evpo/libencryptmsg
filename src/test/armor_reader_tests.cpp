#include "gtest/gtest.h"
#include <fstream>
#include <iterator>
#include "armor_reader.h"
#include "test_helper.h"

namespace EncryptMsg
{
    namespace UnitTests
    {
        class ArmorReaderFixture : public ::testing::Test
        {
            protected:
                std::vector<uint8_t> plain_file_;
                std::vector<uint8_t> asc_of_plain_file_;

                virtual void SetUp() override;
        };

        void ArmorReaderFixture::SetUp()
        {
            LoadFile("simple_text.txt", plain_file_);
            LoadFile("simple_text.txt.plain.asc", asc_of_plain_file_);
        }

        TEST_F(ArmorReaderFixture, When_reading_armored_text_Then_output_matches)
        {
            // Arrange

            ArmorReader reader;

            // Act

            SafeVector buf(asc_of_plain_file_.begin(), asc_of_plain_file_.end());
            reader.GetInStream().Push(buf);
            auto out = MakeOutStream(buf);
            out->Reset();
            auto result = reader.Read(*out);
            auto state = reader.GetState();
            auto result_after_finish = reader.Finish(*out);

            // Assert

            EXPECT_EQ(EmsgResult::Success, result);
            EXPECT_EQ(ArmorState::TailFound, state);
            EXPECT_EQ(EmsgResult::Success, result_after_finish);

            ASSERT_EQ(plain_file_.size(), buf.size());
            ASSERT_TRUE(std::equal(buf.begin(), buf.end(), plain_file_.begin()));
        }

        TEST_F(ArmorReaderFixture, When_reading_armored_text_with_small_buffer_Then_output_matches)
        {
            // Arrange

            static const unsigned int kBufSize = 3;
            ArmorReader reader;
            SafeVector buf;
            SafeVector out;
            auto out_stm = MakeOutStream(out);
            EmsgResult result = EmsgResult::None;

            // Act

            auto it = asc_of_plain_file_.begin();

            while(it != asc_of_plain_file_.end())
            {
                size_t range = std::min(
                        static_cast<unsigned int>(std::distance(it, asc_of_plain_file_.end())),
                        kBufSize);
                auto range_end = it;
                std::advance(range_end, range);
                buf.assign(it, range_end);
                it = range_end;
                reader.GetInStream().Push(buf);
                result = reader.Read(*out_stm);

                EXPECT_TRUE(result == EmsgResult::Success || result == EmsgResult::Pending);
            }
            auto result_after_finish = reader.Finish(*out_stm);

            // Assert

            EXPECT_EQ(EmsgResult::Success, result_after_finish);
            ASSERT_EQ(plain_file_.size(), out.size());
            ASSERT_TRUE(std::equal(out.begin(), out.end(), plain_file_.begin()));
        }
    }
}
