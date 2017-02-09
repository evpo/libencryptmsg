#include <string>
#include "gtest/gtest.h"
#include "hello_name_generator.h"

using namespace std;

namespace CppProject
{
    namespace UnitTests
    {
        class HelloNameFixture : public ::testing::Test
        {
            protected:
                virtual void SetUp()
                {
                }

                virtual void TearDown()
                {
                }
        };

        TEST_F(HelloNameFixture, When_name_passed_Hello_world_name_returned)
        {
            //Arrange
            string name = "Eva";

            //Act
            string result = GenerateHelloName(name);

            //Assert
            ASSERT_EQ("Hello World and Eva", result);
        }
    }
}
