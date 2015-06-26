#include "HException.h"

HException::HException(const std::string reason, NType type, std::string file, int line) NOEXCEPT
    : reason_(reason)
    , type_(type)
    , wherefile_(file)
    , whereline_(line)
{
}

HException::~HException() NOEXCEPT
{

}

