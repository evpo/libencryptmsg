#include "logger_init.h"
#include "plog/Log.h"

namespace CppProject
{
    void InitLogger(const std::string &file_name)
    {
        plog::init(plog::debug, file_name.data());
        LOG_INFO << "Log instance started";
    }
}
