#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include "BASE/Honey.h"
#include "MATH/Rectangle.h"
#include "MATH/Vector2.h"

namespace UTILS
{
    namespace STRING
    {
        // Other simple string utilities.
        bool StartsWith(const std::string &str, const std::string &what);
        bool EndsWith(const std::string &str, const std::string &what);
        // Only use on strings where you're only concerned about ASCII.
        bool StartsWithNoCase(const std::string &str, const std::string &what);
        bool EndsWithNoCase(const std::string &str, const std::string &what);

        void DataToHexString(const uint8 *data, size_t size, std::string *output);
        void StringToHexString(const std::string &data, std::string *output);
        // highly unsafe and not recommended.
        unsigned int ParseHexString(const char* _szValue);

        MATH::Rectf RectFromString(const std::string& str);
        MATH::Vector2f PointFromString(const std::string& str);
        MATH::Sizef SizeFromString(const std::string& pszContent);

        std::string StringFromFormat(const char* format, ...);
        std::string StringFromInt(int value);
        std::string StringFromBool(bool value);

        void SplitString(const std::string& str, const std::string& token, std::vector<std::string>& output);
        void SplitWithForm(const std::string& content, std::vector<std::string>& output);
    }
}

#endif // STRINGUTILS_H
