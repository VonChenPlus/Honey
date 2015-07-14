#ifndef UTFUTILS_H
#define UTFUTILS_H

#include <string>

namespace UTILS
{
    namespace STRING
    {
        void UTF16ToWString(const std::u16string &utf16, std::wstring &outWsting);

        void UTF8ToUTF16(const std::string &utf8, std::u16string &outUtf16);
        void UTF8ToUTF32(const std::string &utf8, std::u32string &outUtf32);
        void UTF8ToWString(const std::string &utf8, std::wstring &outWsting);
        std::wstring UTF8ToWString(const std::string &utf8);

        void UTF16ToUTF8(const std::u16string& utf16, std::string& outUtf8);

        long UTF8StringLength(const std::string& utf8);

        bool IsUnicodeSpace(char16_t ch);
        bool IsCJKUnicode(char16_t ch);
    }
}

#endif // UTFUTILS_H
