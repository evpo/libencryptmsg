#include "hello_name_generator.h"

namespace CppProject
{
    std::string GenerateHelloName(const std::string &name)
    {
        return "Hello World and " + name;
    }
}
