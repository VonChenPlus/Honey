#include "UTFUtils.h"

#include <cstring>
#include <cstdlib>
#include <locale>
using namespace std;

#include "EXTERNALS/ConvertUTF/ConvertUTF.h"
#include "BASE/Honey.h"

namespace UTILS
{
    namespace STRING
    {
        template<class _Codecvt,
        class _Elem = wchar_t,
        class _Walloc = allocator<_Elem>,
        class _Balloc = allocator<char> >
        class wstring_convert
        {	// converts between _Elem (wide) and char (byte) strings
        enum {_BUF_INC = 8, _BUF_MAX = 16};
        void _Init(const _Codecvt *_Pcvt_arg = new _Codecvt)
            {	// initialize the object
            static state_type _State0;
            _State = _State0;
            _Pcvt = _Pcvt_arg;
            _Loc = locale(_Loc, _Pcvt);
            _Nconv = 0;
            }

    public:
        typedef basic_string<char, char_traits<char>, _Balloc> byte_string;
        typedef basic_string<_Elem, char_traits<_Elem>, _Walloc> wide_string;
        typedef typename _Codecvt::state_type state_type;
        typedef typename wide_string::traits_type::int_type int_type;

        wstring_convert()
            : _Has_berr(false), _Has_werr(false), _Has_state(false)
            {	// construct with no error strings
            _Init();
            }

        wstring_convert(const _Codecvt *_Pcvt_arg)
            : _Has_berr(false), _Has_werr(false), _Has_state(false)
            {	// construct with no error strings and codecvt
            _Init(_Pcvt_arg);
            }

        wstring_convert(const _Codecvt *_Pcvt_arg, state_type _State_arg)
            : _Has_berr(false), _Has_werr(false), _Has_state(true)
            {	// construct with no error strings, codecvt, and state
            _Init(_Pcvt_arg);
            _State = _State_arg;
            }

        wstring_convert(const byte_string& _Berr_arg)
            : _Has_berr(true), _Has_werr(false), _Has_state(false),
                _Berr(_Berr_arg)
            {	// construct with byte error string
            _Init();
            }

        wstring_convert(const byte_string& _Berr_arg,
            const wide_string& _Werr_arg)
            : _Has_berr(true), _Has_werr(true), _Has_state(false),
                _Berr(_Berr_arg), _Werr(_Werr_arg)
            {	// construct with byte and wide error strings
            _Init();
            }

        virtual ~wstring_convert()
            {	// destroy the object
            }

        size_t converted() const
            {	// get conversion count
            return (_Nconv);
            }

        state_type state() const
            {	// get state
            return (_State);
            }

        wide_string from_bytes(char _Byte)
            {	// convert a byte to a wide string
            return (from_bytes(&_Byte, &_Byte + 1));
            }

        wide_string from_bytes(const char *_Ptr)
            {	// convert a NTBS to a wide string
            return (from_bytes(_Ptr, _Ptr + strlen(_Ptr)));
            }

        wide_string from_bytes(const byte_string& _Bstr)
            {	// convert a byte string to a wide string
            const char *_Ptr = _Bstr.c_str();
            return (from_bytes(_Ptr, _Ptr + _Bstr.size()));
            }

        wide_string from_bytes(const char *_First, const char *_Last)
            {	// convert byte sequence [_First, _Last) to a wide string
            static state_type _State0;
            wide_string _Wbuf, _Wstr;
            const char *_First_sav = _First;

            if (!_Has_state)
                _State = _State0;	// reset state if not remembered
            _Wbuf.append((size_t)_BUF_INC, (_Elem)'\0');
            for (_Nconv = 0; _First != _Last; _Nconv = _First - _First_sav)
                {	// convert one or more bytes
                _Elem *_Dest = &*_Wbuf.begin();
                _Elem *_Dnext;

                switch (_Pcvt->in(_State,
                    _First, _Last, _First,
                    _Dest, _Dest + _Wbuf.size(), _Dnext))
                    {	// test result of converting one or more bytes
                case _Codecvt::partial:
                case _Codecvt::ok:
                    if (_Dest < _Dnext)
                        _Wstr.append(_Dest, (size_t)(_Dnext - _Dest));
                    else if (_Wbuf.size() < _BUF_MAX)
                        _Wbuf.append((size_t)_BUF_INC, '\0');
                    else if (_Has_werr)
                        return (_Werr);
                    else
                        throw _HException_Normal("bad conversion");
                    break;

                case _Codecvt::noconv:
                    for (; _First != _Last; ++_First)
                        _Wstr.append((size_t)1,
                            (_Elem)(unsigned char)*_First);
                    break;	// no conversion, just copy code values

                default:
                    if (_Has_werr)
                        return (_Werr);
                    else
                        throw _HException_Normal("bad conversion");
                    }
                }
            return (_Wstr);
            }

        byte_string to_bytes(_Elem _Char)
            {	// convert a wide char to a byte string
            return (to_bytes(&_Char, &_Char + 1));
            }

        byte_string to_bytes(const _Elem *_Wptr)
            {	// convert a NTWCS to a byte string
            const _Elem *_Next = _Wptr;
            for (; (int_type)*_Next != 0; ++_Next)
                ;
            return (to_bytes(_Wptr, _Next));
            }

        byte_string to_bytes(const wide_string& _Wstr)
            {	// convert a wide string to a byte string
            const _Elem *_Wptr = _Wstr.c_str();
            return (to_bytes(_Wptr, _Wptr + _Wstr.size()));
            }

        byte_string to_bytes(const _Elem *_First, const _Elem *_Last)
            {	// convert wide sequence [_First, _Last) to a byte string
            static state_type _State0;
            byte_string _Bbuf, _Bstr;
            const _Elem *_First_sav = _First;

            if (!_Has_state)
                _State = _State0;	// reset state if not remembered
            _Bbuf.append((size_t)_BUF_INC, '\0');
            for (_Nconv = 0; _First != _Last; _Nconv = _First - _First_sav)
                {	// convert one or more wide chars
                char *_Dest = &*_Bbuf.begin();
                char *_Dnext;

                switch (_Pcvt->out(_State,
                    _First, _Last, _First,
                    _Dest, _Dest + _Bbuf.size(), _Dnext))
                    {	// test result of converting one or more wide chars
                case _Codecvt::partial:
                case _Codecvt::ok:
                    if (_Dest < _Dnext)
                        _Bstr.append(_Dest, (size_t)(_Dnext - _Dest));
                    else if (_Bbuf.size() < _BUF_MAX)
                        _Bbuf.append((size_t)_BUF_INC, '\0');
                    else if (_Has_berr)
                        return (_Berr);
                    else
                        throw _HException_Normal("bad conversion");
                    break;

                case _Codecvt::noconv:
                    for (; _First != _Last; ++_First)
                        _Bstr.append((size_t)1,
                            (char)(int_type)*_First);
                    break;	// no conversion, just copy code values

                default:
                    if (_Has_berr)
                        return (_Berr);
                    else
                        throw _HException_Normal("bad conversion");
                    }
                }
            return (_Bstr);
            }

        wstring_convert(const wstring_convert&) = delete;
        wstring_convert& operator=(const wstring_convert&) = delete;

    private:
        const _Codecvt *_Pcvt;	// the codecvt facet
        locale _Loc;	// manages reference to codecvt facet
        byte_string _Berr;
        wide_string _Werr;
        state_type _State;	// the remembered state
        bool _Has_berr;
        bool _Has_werr;
        bool _Has_state;
        size_t _Nconv;
        };

        enum codecvt_mode {
            consume_header = 4,
            generate_header = 2,
            little_endian = 1};

        typedef mbstate_t _Statype;
        template<class _Elem,
        unsigned long _Mymax = 0x10ffff,
        codecvt_mode _Mymode = (codecvt_mode)0>
        class codecvt_utf8
        : public codecvt<_Elem, char, _Statype>
        {	// facet for converting between _Elem and UTF-8 byte sequences
    public:
        typedef codecvt<_Elem, char, _Statype> _Mybase;
        typedef typename _Mybase::result result;
        typedef char _Byte;
        typedef _Elem intern_type;
        typedef _Byte extern_type;
        typedef _Statype state_type;

        explicit codecvt_utf8(size_t _Refs = 0)
            : _Mybase(_Refs)
            {	// construct with ref count
            }

        virtual ~codecvt_utf8()
            {	// destroy the object
            }

    protected:
        virtual result do_in(_Statype& _State,
            const _Byte *_First1, const _Byte *_Last1, const _Byte *& _Mid1,
            _Elem *_First2, _Elem *_Last2, _Elem *& _Mid2) const
            {	// convert bytes [_First1, _Last1) to [_First2, _Last)
            char *_Pstate = (char *)&_State;
            _Mid1 = _First1;
            _Mid2 = _First2;

            for (; _Mid1 != _Last1 && _Mid2 != _Last2; )
                {	// convert a multibyte sequence
                unsigned char _By = (unsigned char)*_Mid1;
                unsigned long _Ch;
                int _Nextra;

                if (_By < 0x80)
                    _Ch = _By, _Nextra = 0;
                else if (_By < 0xc0)
                    {	// 0x80-0xdf not first byte
                    ++_Mid1;
                    return (_Mybase::error);
                    }
                else if (_By < 0xe0)
                    _Ch = _By & 0x1f, _Nextra = 1;
                else if (_By < 0xf0)
                    _Ch = _By & 0x0f, _Nextra = 2;
                else if (_By < 0xf8)
                    _Ch = _By & 0x07, _Nextra = 3;
                else
                    _Ch = _By & 0x03, _Nextra = _By < 0xfc ? 4 : 5;

                if (_Nextra == 0)
                    ++_Mid1;
                else if (_Last1 - _Mid1 < _Nextra + 1)
                    break;	// not enough input
                else
                    for (++_Mid1; 0 < _Nextra; --_Nextra, ++_Mid1)
                        if ((_By = (unsigned char)*_Mid1) < 0x80 || 0xc0 <= _By)
                            return (_Mybase::error);	// not continuation byte
                        else
                            _Ch = _Ch << 6 | (_By & 0x3f);

                if (*_Pstate == 0)
                    {	// first time, maybe look for and consume header
                    *_Pstate = 1;

                    if ((_Mymode & consume_header) != 0 && _Ch == 0xfeff)
                        {	// drop header and retry
                        result _Ans = do_in(_State, _Mid1, _Last1, _Mid1,
                            _First2, _Last2, _Mid2);

                        if (_Ans == _Mybase::partial)
                            {	// roll back header determination
                            *_Pstate = 0;
                            _Mid1 = _First1;
                            }
                        return (_Ans);
                        }
                    }

                if (_Mymax < _Ch)
                    return (_Mybase::error);	// code too large
                *_Mid2++ = (_Elem)_Ch;
                }

            return (_First1 == _Mid1 ? _Mybase::partial : _Mybase::ok);
            }

        virtual result do_out(_Statype& _State,
            const _Elem *_First1, const _Elem *_Last1, const _Elem *& _Mid1,
            _Byte *_First2, _Byte *_Last2, _Byte *& _Mid2) const
            {	// convert [_First1, _Last1) to bytes [_First2, _Last)
            char *_Pstate = (char *)&_State;
            _Mid1 = _First1;
            _Mid2 = _First2;

            for (; _Mid1 != _Last1 && _Mid2 != _Last2; )
                {	// convert and put a wide char
                _Byte _By;
                int _Nextra;
                unsigned long _Ch = (unsigned long)*_Mid1;

                if (_Mymax < _Ch)
                    return (_Mybase::error);

                if (_Ch < 0x0080)
                    _By = (_Byte)_Ch, _Nextra = 0;
                else if (_Ch < 0x0800)
                    _By = (_Byte)(0xc0 | _Ch >> 6), _Nextra = 1;
                else if (_Ch < 0x00010000)
                    _By = (_Byte)(0xe0 | _Ch >> 12), _Nextra = 2;
                else if (_Ch < 0x00200000)
                    _By = (_Byte)(0xf0 | _Ch >> 18), _Nextra = 3;
                else if (_Ch < 0x04000000)
                    _By = (_Byte)(0xf8 | _Ch >> 24), _Nextra = 4;
                else
                    _By = (_Byte)(0xfc | (_Ch >> 30 & 0x03)), _Nextra = 5;

                if (*_Pstate == 0)
                    {	// first time, maybe generate header
                    *_Pstate = 1;
                    if ((_Mymode & generate_header) == 0)
                        ;
                    else if (_Last2 - _Mid2 < 3 + 1 + _Nextra)
                        return (_Mybase::partial);	// not enough room for both
                    else
                        {	// prepend header
                        *_Mid2++ = (_Byte)(unsigned char)0xef;
                        *_Mid2++ = (_Byte)(unsigned char)0xbb;
                        *_Mid2++ = (_Byte)(unsigned char)0xbf;
                        }
                    }

                if (_Last2 - _Mid2 < 1 + _Nextra)
                    break;	// not enough room for output

                ++_Mid1;
                for (*_Mid2++ = _By; 0 < _Nextra; )
                    *_Mid2++ = (_Byte)((_Ch >> 6 * --_Nextra & 0x3f) | 0x80);
                }
            return (_First1 == _Mid1 ? _Mybase::partial : _Mybase::ok);
            }

        virtual result do_unshift(_Statype&,
            _Byte *_First2, _Byte *, _Byte *& _Mid2) const
            {	// generate bytes to return to default shift state
            _Mid2 = _First2;
            return (_Mybase::ok);
            }

        virtual int do_length(_Statype& _State, const _Byte *_First1,
            const _Byte *_Last1, size_t _Count) const
            {	// return min(_Count, converted length of bytes [_First1, _Last1))
            size_t _Wchars = 0;
            _Statype _Mystate = _State;

            for (; _Wchars < _Count && _First1 != _Last1; )
                {	// convert another wide character
                const _Byte *_Mid1;
                _Elem *_Mid2;
                _Elem _Ch;

                switch (do_in(_Mystate, _First1, _Last1, _Mid1,
                    &_Ch, &_Ch + 1, _Mid2))
                    {	// test result of single wide-char conversion
                case _Mybase::noconv:
                    return ((int)(_Wchars + (_Last1 - _First1)));

                case  _Mybase::ok:
                    if (_Mid2 == &_Ch + 1)
                        ++_Wchars;	// replacement do_in might not convert one
                    _First1 = _Mid1;
                    break;

                default:
                    return ((int)_Wchars);	// error or partial
                    }
                }

            return ((int)_Wchars);
            }

        virtual bool do_always_noconv()
            {	// return true if conversions never change input
            return (false);
            }

        virtual int do_max_length()
            {	// return maximum length required for a conversion
            return ((_Mymode & (consume_header | generate_header)) != 0
                ? 9 : 6);
            }

        virtual int do_encoding()
            {	// return length of code sequence (from codecvt)
            return ((_Mymode & (consume_header | generate_header)) != 0
                ? -1 : 0);	// -1 => state dependent, 0 => varying length
            }
        };

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
            if (utf8.empty()) {
                outWsting.clear();
                return;
            }

            wstring_convert<codecvt_utf8<wchar_t> > conv;
            outWsting = conv.from_bytes(utf8);
        }

        std::wstring UTF8ToWString(const std::string &utf8) {
            std::wstring wstring;
            if (utf8.empty()) {
                return wstring;
            }

            UTF8ToWString(utf8, wstring);
            return wstring;
        }

        void WStringToUTF8(const std::wstring &wstring, std::string &outUtf8) {
            if (wstring.empty()) {
                outUtf8.clear();
                return;
            }

            wstring_convert<codecvt_utf8<wchar_t> >conv;
            outUtf8 = conv.to_bytes(wstring.c_str());
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

#ifdef _WIN32
        #include <Windows.h>
        void WStringToString(const std::wstring &wstring, std::string &outString) {
            if (wstring.empty()) {
                outString.clear();
                return;
            }

            int nLen = (int) wstring.length();
            int nResult = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) wstring.c_str(), nLen, 0, 0, NULL, NULL);
            outString.resize(nResult);
            WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) wstring.c_str(), nLen, (LPSTR) &outString[0], nResult, NULL, NULL);
        }

        void StringToWstring(const std::string &string, std::wstring &outWstring) {
            if (string.empty()) {
                outWstring.clear();
                return;
            }

            int nLen = (int) string.length();
            int nResult = MultiByteToWideChar(CP_ACP, 0, (LPCSTR) string.c_str(), nLen, 0, 0);
            outWstring.resize(nResult);
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR) string.c_str(), nLen, (LPWSTR) &outWstring[0], nResult);
        }

#endif

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
