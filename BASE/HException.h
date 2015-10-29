#ifndef HEXCEPTION_H
#define HEXCEPTION_H

#include <exception>
#include <string>

#if defined(_MSC_VER) && _MSC_VER >= 1900 ||\
    defined(__GNUC__) && __GNUC__ * 10 + __GNUC_MINOR__ >= 49
  #define NOEXCEPT noexcept
#elif defined(__clang__)
  #if __has_feature(cxx_noexcept)
    #define NOEXCEPT noexcept
  #else
    #define NOEXCEPT throw()
#endif  /*  __has_feature   */
#else
  #define NOEXCEPT throw()
#endif  /*  NOEXCEPT    */

#define _HException_( msg, type) HException( (msg), (type), __FILE__, __LINE__ )
#define _HException_Unknown( msg) _HException_( (msg), (HException::Unknown) )
#define _HException_Normal( msg) _HException_( (msg), (HException::Normal) )

class HException : std::exception
{
public:
    enum NType
    {
        Unknown = -1,
        Normal = 0,
        IO,
        GRAPH,
    };

protected:
    std::string reason_;
    NType type_;
    std::string wherefile_;
    int whereline_;

public:
    HException(const std::string reason, NType type = Unknown, std::string file = "", int line = 0) NOEXCEPT;
    virtual ~HException() NOEXCEPT;

    const std::string &reason() NOEXCEPT { return reason_; }
    NType type() NOEXCEPT { return type_; }
    const std::string &wherefile() NOEXCEPT { return wherefile_; }
    int whereline() NOEXCEPT { return whereline_; }

private:
    HException() NOEXCEPT {}
};

#endif // HEXCEPTION_H
