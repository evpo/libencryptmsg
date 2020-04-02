#include "gtest/gtest.h"
#include <fstream>
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

            ArmorContext context;
            ArmorHeaderReader header_reader(context);
            ArmorReader reader(context);

            // Act

            SafeVector buf(asc_of_plain_file_.begin(), asc_of_plain_file_.end());
            header_reader.GetInStream().Push(buf);
            auto result_after_header = header_reader.Read(false);
            auto status_after_header = context.status;
            buf.resize(header_reader.GetInStream().GetCount());
            header_reader.GetInStream().Read(buf.data(), buf.size());

            reader.GetInStream().Push(buf);
            auto out = MakeOutStream(buf);
            out->Reset();
            auto result_after_reader = reader.Read(*out);
            auto status_after_reader = context.status;
            auto result_after_finish = reader.Finish();

            // Assert

            EXPECT_EQ(EmsgResult::Success, result_after_header);
            EXPECT_EQ(ArmorStatus::Payload, status_after_header);
            EXPECT_EQ(EmsgResult::Success, result_after_reader);
            EXPECT_EQ(ArmorStatus::Payload, status_after_reader);
            EXPECT_EQ(EmsgResult::Success, result_after_finish);

            ASSERT_EQ(plain_file_.size(), buf.size());
            ASSERT_TRUE(std::equal(buf.begin(), buf.end(), plain_file_.begin()));
        }
    }
}
