#include "StringUtils.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <limits.h>

#include <algorithm>
#include <iomanip>

#include "BASE/HBuffer.h"

namespace UTILS
{
    namespace STRING
    {
        bool StartsWith(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            return str.substr(0, what.size()) == what;
        }

        bool EndsWith(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            return str.substr(str.size() - what.size()) == what;
        }

        bool StartsWithNoCase(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            return strncasecmp(str.c_str(), what.c_str(), what.size()) == 0;
        }

        bool EndsWithNoCase(const std::string &str, const std::string &what) {
            if (str.size() < what.size())
                return false;
            const size_t offset = str.size() - what.size();
            return strncasecmp(str.c_str() + offset, what.c_str(), what.size()) == 0;
        }

        void StringToHexString(const std::string &data, std::string *output) {
            DataToHexString((uint8 *)(&data[0]), data.size(), output);
        }

        unsigned int ParseHexString(const char *_szValue) {
            int Value = 0;
            size_t Finish = strlen(_szValue);
            if (Finish > 8 ) { Finish = 8; }

            for (size_t Count = 0; Count < Finish; Count++) {
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

        bool ParseBoolean(const std::string& value)
        {
            return (value.compare("true")==0);
        }

        void DataToHexString(const uint8 *data, size_t size, std::string *output) {
            HBuffer buffer;
            for (size_t i = 0; i < size; i++) {
                buffer.writeAsFormat((const HBYTE *)"%02x ", data[i]);
                if (i && !(i & 15))
                    buffer.writeAsFormat((const HBYTE *)"\n");
            }
            output->resize(buffer.size());
            buffer.read(buffer.size(), (HBYTE *)&(*output)[0]);
        }

        MATH::Rectf RectFromString(const std::string& str)
        {
            MATH::Rectf result = MATH::RectfZERO;

            if (str.empty()) return result;
            std::string content = str;

            // find the first '{' and the third '}'
            size_t nPosLeft = content.find('{');
            size_t nPosRight = content.find('}');
            for (int i = 1; i < 3; ++i)
            {
                if (nPosRight == std::string::npos)
                {
                    break;
                }
                nPosRight = content.find('}', nPosRight + 1);
            }
            if (nPosLeft == std::string::npos || nPosRight == std::string::npos)
                throw _HException_Normal("Unknow rect string format!");

            content = content.substr(nPosLeft + 1, nPosRight - nPosLeft - 1);
            size_t nPointEnd = content.find('}');
            if (nPointEnd == std::string::npos)
                throw _HException_Normal("Unknow rect string format!");
            nPointEnd = content.find(',', nPointEnd);
            if (nPointEnd == std::string::npos)
                throw _HException_Normal("Unknow rect string format!");

            // get the point string and size string
            const std::string pointStr = content.substr(0, nPointEnd);
            const std::string sizeStr = content.substr(nPointEnd + 1, content.length() - nPointEnd);

            // split the string with ','
            std::vector<std::string> pointInfo;
            SplitWithForm(pointStr, pointInfo);
            std::vector<std::string> sizeInfo;
            SplitWithForm(sizeStr.c_str(), sizeInfo);

            float x = (float) atof(pointInfo[0].c_str());
            float y = (float) atof(pointInfo[1].c_str());
            float width = (float) atof(sizeInfo[0].c_str());
            float height = (float) atof(sizeInfo[1].c_str());

            result = MATH::Rectf(x, y, width, height);
            return result;
        }

        MATH::Vector2f PointFromString(const std::string& str)
        {
            MATH::Vector2f ret = MATH::Vec2fZERO;

            if (str.empty()) return ret;

            std::vector<std::string> strs;
            SplitWithForm(str, strs);

            float x = (float) atof(strs[0].c_str());
            float y = (float) atof(strs[1].c_str());

            ret.set(x, y);
            return ret;
        }

        MATH::Sizef SizeFromString(const std::string& pszContent)
        {
            MATH::Sizef ret = MATH::SizefZERO;
            if (pszContent.empty()) return ret;

            std::vector<std::string> strs;
            SplitWithForm(pszContent, strs);

            float width = (float) atof(strs[0].c_str());
            float height = (float) atof(strs[1].c_str());

            ret = MATH::Sizef(width, height);
            return ret;
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

        void SplitString(const std::string& src, const std::string& token, std::vector<std::string>& output) {
            size_t nend = 0;
            size_t nbegin = 0;
            size_t tokenSize = token.size();
            while (nend != std::string::npos)
            {
                nend = src.find(token, nbegin);
                if (nend == std::string::npos)
                    output.push_back(src.substr(nbegin, src.length() - nbegin));
                else
                    output.push_back(src.substr(nbegin, nend - nbegin));
                nbegin = nend + tokenSize;
            }
        }

        void SplitWithForm(const std::string& content, std::vector<std::string>& output) {
            if (content.empty()) return;

            size_t nPosLeft = content.find('{');
            size_t nPosRight = content.find('}');

            // don't have '{' and '}'
            if (nPosLeft == std::string::npos || nPosRight == std::string::npos)
                throw _HException_Normal("Unknow string format!");
            // '}' is before '{'
            if (nPosLeft > nPosRight) 
                throw _HException_Normal("Unknow string format!");

            const std::string pointStr = content.substr(nPosLeft + 1, nPosRight - nPosLeft - 1);
            // nothing between '{' and '}'
            if (pointStr.length() == 0) return;

            size_t nPos1 = pointStr.find('{');
            size_t nPos2 = pointStr.find('}');
            // contain '{' or '}' 
            if (nPos1 != std::string::npos || nPos2 != std::string::npos)
                throw _HException_Normal("Unknow string format!");

            SplitString(pointStr, ",", output);
            if (output.size() != 2 || output[0].length() == 0 || output[1].length() == 0) {
                throw _HException_Normal("Unknow string format!");
            }
        }
    }
}
