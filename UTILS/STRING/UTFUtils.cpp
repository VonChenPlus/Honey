#include "UTFUtils.h"

#include <cstring>
#include <cstdlib>

#include "EXTERNALS/ConvertUTF/ConvertUTF.h"
#include "BASE/Honey.h"

namespace UTILS
{
    namespace STRING
    {
        void UTF8ToUTF16(const std::string &utf8, std::u16string &outUtf16) {
            if (utf8.empty()) {
                outUtf16.clear();
                return;
            }

            const size_t utf16Bytes = (utf8.length()+1) * sizeof(char16_t);
            char16_t* utf16 = (char16_t*)malloc(utf16Bytes);
            memset(utf16, 0, utf16Bytes);

            char* utf16ptr = reinterpret_cast<char*>(utf16);
            const UTF8* error = NULLPTR;

            if (!llvm::ConvertUTF8toWide(2, utf8, utf16ptr, error)) {
                throw _HException_Normal("ConvertUTF8toWide Error!");
            }

            outUtf16 = utf16;
            free(utf16);
        }

        void UTF8ToUTF32(const std::string &utf8, std::u32string &outUtf32) {
            if (utf8.empty()) {
                outUtf32.clear();
                return;
            }

            const size_t utf32Bytes = (utf8.length()+1) * sizeof(char32_t);
            char32_t* utf32 = (char32_t*)malloc(utf32Bytes);
            memset(utf32, 0, utf32Bytes);

            char* utf32ptr = reinterpret_cast<char*>(utf32);
            const UTF8* error = NULLPTR;

            if (!llvm::ConvertUTF8toWide(4, utf8, utf32ptr, error)) {
                throw _HException_Normal("ConvertUTF8toWide Error!");
            }

            outUtf32 = utf32;
            free(utf32);
        }

        void UTF8ToWString(const std::string &utf8, std::wstring &outWsting) {
            std::u16string utf16;
            UTF8ToUTF16(utf8, utf16);
            UTF16ToWString(utf16, outWsting);
        }

        std::wstring UTF8ToWString(const std::string &utf8) {
            std::u16string utf16;
            UTF8ToUTF16(utf8, utf16);
            std::wstring wstring;
            UTF16ToWString(utf16, wstring);
            return wstring;
        }

        void UTF16ToUTF8(const std::u16string& utf16, std::string& outUtf8) {
            if (utf16.empty()) {
                outUtf8.clear();
                return;
            }

            if (!llvm::convertUTF16ToUTF8String(utf16, outUtf8)) {
                throw _HException_Normal("convertUTF16ToUTF8String Error!");
            }
        }

        void UTF16ToWString(const std::u16string &utf16, std::wstring &outWsting) {
            if (utf16.empty()) {
                outWsting.clear();
                return;
            }

            Size length = utf16.length();
            outWsting.resize(length);
            for (Size index = 0; index < length; ++index) {
                outWsting[index] = utf16[index];
            }
        }

        long UTF8StringLength(const std::string& utf8) {
            return getUTF8StringLength((const UTF8*)utf8.c_str());
        }

        bool IsUnicodeSpace(char16_t ch) {
            return  (ch >= 0x0009 && ch <= 0x000D) || ch == 0x0020 || ch == 0x0085 || ch == 0x00A0 || ch == 0x1680
               || (ch >= 0x2000 && ch <= 0x200A) || ch == 0x2028 || ch == 0x2029 || ch == 0x202F
               ||  ch == 0x205F || ch == 0x3000;
        }

        bool IsCJKUnicode(char16_t ch) {
            return (ch >= 0x4E00 && ch <= 0x9FBF)   // CJK Unified Ideographs
                    || (ch >= 0x2E80 && ch <= 0x2FDF)   // CJK Radicals Supplement & Kangxi Radicals
                    || (ch >= 0x2FF0 && ch <= 0x30FF)   // Ideographic Description Characters, CJK Symbols and Punctuation & Japanese
                    || (ch >= 0x3100 && ch <= 0x31BF)   // Korean
                    || (ch >= 0xAC00 && ch <= 0xD7AF)   // Hangul Syllables
                    || (ch >= 0xF900 && ch <= 0xFAFF)   // CJK Compatibility Ideographs
                    || (ch >= 0xFE30 && ch <= 0xFE4F)   // CJK Compatibility Forms
                    || (ch >= 0x31C0 && ch <= 0x4DFF);  // Other exiensions
        }
    }
}
