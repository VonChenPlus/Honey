#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include "BASE/BasicTypes.h"

namespace UTILS
{
    namespace STRING
    {
        #ifdef _MSC_VER
        #pragma warning (disable:4996)
        #define strncasecmp _strnicmp
        #define strcasecmp _strcmpi
        #endif

        #ifdef BLACKBERRY
        // QNX Does not have an implementation of vasprintf
        static inline int vasprintf(char **rResult, const char *aFormat, va_list aAp) {
            int rVal;
            char *result;
            va_list ap;

            result = (char *) malloc(16);
            if (result == NULLPTR) return -1;

            va_copy(ap, aAp);
            rVal = vsnprintf(result, 16, aFormat, ap);
            va_end(ap);

            if (rVal == -1) {
                free(result);
                return rVal;
            }
            else if (rVal >= 16) {
                free(result);
                result = (char *) malloc(rVal + 1);
                if (result == NULLPTR) return -1;

                va_copy(ap, aAp);
                rVal = vsnprintf(result, rVal + 1, aFormat, aAp);
                va_end(ap);
            }

            *rResult = result;
            return rVal;
        }
        #endif

        // Dumb wrapper around itoa, providing a buffer. Declare this on the stack.
        class ITOA
        {
        public:
            char buffer[16];
            const char *p(int i) {
                sprintf(buffer, "%i", i);
                return &buffer[0];
            }
        };

        // Other simple string utilities.

        inline bool StartsWith(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            return str.substr(0, what.size()) == what;
        }

        inline bool EndsWith(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
          return str.substr(str.size() - what.size()) == what;
        }

        // Only use on strings where you're only concerned about ASCII.
        inline bool StartsWithNoCase(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            return strncasecmp(str.c_str(), what.c_str(), what.size()) == 0;
        }

        inline bool EndsWithNoCase(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            const Size offset = str.size() - what.size();
            return strncasecmp(str.c_str() + offset, what.c_str(), what.size()) == 0;
        }

        void DataToHexString(const uint8 *data, Size size, std::string *output);
        inline void StringToHexString(const std::string &data, std::string *output) {
            DataToHexString((uint8 *)(&data[0]), data.size(), output);
        }


        // highly unsafe and not recommended.
        unsigned int ParseHex(const char* _szValue);


        // Suitable for inserting into maps, unlike char*, and cheaper than std::string.
        // Strings must be constant and preferably be stored in the read-only part
        // of the binary.
        class ConstString
        {
        public:
            ConstString(const char *ptr)
            {
                ptr_ = ptr;
            }
            bool operator <(const ConstString &other) const
            {
                return strcmp(ptr_, other.ptr_) < 0;
            }
            bool operator ==(const ConstString &other) const
            {
                return ptr_ == other.ptr_ || !strcmp(ptr_, other.ptr_);
            }
            const char *get() const { return ptr_; }
        private:
            const char *ptr_;
        };

        std::string StringFromFormat(const char* format, ...);
        std::string StringFromInt(int value);
        std::string StringFromBool(bool value);

        std::string ArrayToString(const uint8 *data, uint32 size, int line_len = 20, bool spaces = true);

        std::string StripSpaces(const std::string &s);
        std::string StripQuotes(const std::string &s);

        bool TryParse(const std::string &str, bool *const output);
        bool TryParse(const std::string &str, uint32 *const output);

        template <typename N>
        static bool TryParse(const std::string &str, N *const output)
        {
            std::istringstream iss(str);

            N tmp = 0;
            if (iss >> tmp)
            {
                *output = tmp;
                return true;
            }
            else
                return false;
        }
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
