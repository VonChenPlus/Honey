#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include "BASE/Honey.h"

namespace UTILS
{
    namespace STRING
    {
        #ifdef _MSC_VER
            #pragma warning (disable:4996)
            #ifndef strncasecmp
                #define strncasecmp _strnicmp
            #endif

            #ifndef strcasecmp
                #define strcasecmp _strcmpi
            #endif
        #endif

        // Other simple string utilities.
        bool StartsWith(const std::string &str, const std::string &what);
        bool EndsWith(const std::string &str, const std::string &what);

        // Only use on strings where you're only concerned about ASCII.
        bool StartsWithNoCase(const std::string &str, const std::string &what);
        bool EndsWithNoCase(const std::string &str, const std::string &what);

        void DataToHexString(const uint8 *data, Size size, std::string *output);
        void StringToHexString(const std::string &data, std::string *output);
        // highly unsafe and not recommended.
        unsigned int ParseHexString(const char* _szValue);

        std::string StringFromFormat(const char* format, ...);
        std::string StringFromInt(int value);
        std::string StringFromBool(bool value);

        std::string ArrayToString(const uint8 *data, uint32 size, int line_len = 20, bool spaces = true);

        std::string StripSpaces(const std::string &s);
        std::string StripQuotes(const std::string &s);

        void SplitString(const std::string& str, const char delim, std::vector<std::string>& output);

        std::string ReplaceAll(std::string input, const std::string& src, const std::string& dest);

        // Compare two strings, ignore the difference between the ignorestr1 and the ignorestr2 in str1 and str2.
        int StrcmpIgnore(std::string str1, std::string str2, std::string ignorestr1, std::string ignorestr2);

        void StringTrimEndNonAlphaNum(char *str);
        void SkipSpace(const char **ptr);
        void StringUpper(char *str);
        void StringUpper(char *str, int len);
    }
}

#endif // STRINGUTILS_H
