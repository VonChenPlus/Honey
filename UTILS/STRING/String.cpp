#include "String.h"

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <limits.h>

#include <algorithm>
#include <iomanip>

#include "BASE/Buffer.h"

namespace UTILS
{
    namespace STRING
    {
        void StringTrimEndNonAlphaNum(char *str) {
            Size n = strlen(str);
            while (!isalnum(str[n]) && n > 0) {
                str[n--] = '\0';
            }
        }

        void SkipSpace(const char **ptr) {
            while (**ptr && isspace(**ptr)) {
                (*ptr)++;
            }
        }

        void StringUpper(char *str) {
            while (*str) {
                *str = toupper(*str);
                str++;
            }
        }

        void StringUpper(char *str, int len) {
            while (len--) {
                *str = toupper(*str);
                str++;
            }
        }


        unsigned int ParseHex(const char *_szValue) {
            int Value = 0;
            Size Finish = strlen(_szValue);
            if (Finish > 8 ) { Finish = 8; }

            for (Size Count = 0; Count < Finish; Count++) {
                Value = (Value << 4);
                switch( _szValue[Count] ) {
                case '0': break;
                case '1': Value += 1; break;
                case '2': Value += 2; break;
                case '3': Value += 3; break;
                case '4': Value += 4; break;
                case '5': Value += 5; break;
                case '6': Value += 6; break;
                case '7': Value += 7; break;
                case '8': Value += 8; break;
                case '9': Value += 9; break;
                case 'A': Value += 10; break;
                case 'a': Value += 10; break;
                case 'B': Value += 11; break;
                case 'b': Value += 11; break;
                case 'C': Value += 12; break;
                case 'c': Value += 12; break;
                case 'D': Value += 13; break;
                case 'd': Value += 13; break;
                case 'E': Value += 14; break;
                case 'e': Value += 14; break;
                case 'F': Value += 15; break;
                case 'f': Value += 15; break;
                default:
                    Value = (Value >> 4);
                    Count = Finish;
                }
            }
            return Value;
        }

        void DataToHexString(const uint8 *data, Size size, std::string *output) {
            Buffer buffer;
            for (Size i = 0; i < size; i++) {
                buffer.appendFormat("%02x ", data[i]);
                if (i && !(i & 15))
                    buffer.appendFormat("\n");
            }
            output->resize(buffer.size());
            buffer.take(buffer.size(), &(*output)[0]);
        }

        std::string StringFromFormat(const char* format, ...) {
            va_list args;
            char *buf = NULLPTR;
            std::string temp = "";
        #ifdef _WIN32
            int required = 0;

            va_start(args, format);
            required = _vscprintf(format, args);
            buf = new char[required + 1];
            if(vsnprintf(buf, required, format, args) < 0)
                buf[0] = '\0';
            va_end(args);

            buf[required] = '\0';
            temp = buf;
            delete[] buf;
        #else
            va_start(args, format);
            if(vasprintf(&buf, format, args) < 0)
                buf = NULLPTR;
            va_end(args);

            if(buf != NULLPTR) {
                temp = buf;
                free(buf);
            }
        #endif
            return temp;
        }

        std::string StringFromInt(int value) {
            char temp[16];
            sprintf(temp, "%i", value);
            return temp;
        }

        std::string StringFromBool(bool value) {
            return value ? "True" : "False";
        }

        // Turns "  hej " into "hej". Also handles tabs.
        std::string StripSpaces(const std::string &str) {
            const Size s = str.find_first_not_of(" \t\r\n");

            if (str.npos != s)
                return str.substr(s, str.find_last_not_of(" \t\r\n") - s + 1);
            else
                return "";
        }

        // "\"hello\"" is turned to "hello"
        // This one assumes that the string has already been space stripped in both
        // ends, as done by StripSpaces above, for example.
        std::string StripQuotes(const std::string& s) {
            if (s.size() && '\"' == s[0] && '\"' == *s.rbegin())
                return s.substr(1, s.size() - 2);
            else
                return s;
        }

        // For Debugging. Read out an u8 array.
        std::string ArrayToString(const uint8 *data, uint32 size, int line_len, bool spaces) {
            std::ostringstream oss;
            oss << std::setfill('0') << std::hex;

            for (int line = 0; size; ++data, --size) {
                oss << std::setw(2) << (int)*data;
                if (line_len == ++line) {
                    oss << '\n';
                    line = 0;
                }
                else if (spaces)
                    oss << ' ';
            }

            return oss.str();
        }

        void SplitString(const std::string& str, const char delim, std::vector<std::string>& output) {
            std::istringstream iss(str);
            output.resize(1);

            while (std::getline(iss, *output.rbegin(), delim))
                output.push_back("");

            output.pop_back();
        }

        std::string ReplaceAll(std::string result, const std::string& src, const std::string& dest) {
            Size pos = 0;

            if (src == dest)
                return result;

            while(1) {
                pos = result.find(src, pos);
                if (pos == result.npos)
                    break;
                result.replace(pos, src.size(), dest);
                pos += dest.size();
            }
            return result;
        }

        int StrcmpIgnore(std::string str1, std::string str2, std::string ignorestr1, std::string ignorestr2) {
            str1 = ReplaceAll(str1, ignorestr1, ignorestr2);
            str2 = ReplaceAll(str2, ignorestr1, ignorestr2);
            return strcmp(str1.c_str(),str2.c_str());
        }
    }
}
