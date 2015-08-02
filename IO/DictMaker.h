#ifndef DICTMAKER_H
#define DICTMAKER_H

#include <stack>
#include "BASE/Honey.h"
#include "BASE/HValue.h"
#include "BASE/HData.h"

namespace IO
{
    class SAXDelegator
    {
    public:
        virtual ~SAXDelegator() {}


        virtual void startElement(void *ctx, const char *name, const char **atts) = 0;
        virtual void endElement(void *ctx, const char *name) = 0;
        virtual void textHandler(void *ctx, const char *s, int len) = 0;
    };

    class SAXParser
    {
    public:
        SAXParser();
        ~SAXParser(void);
        bool parse(const HBYTE* xmlData, size_t dataLength);
        bool parse(const std::string& filename);
        void setDelegator(SAXDelegator* delegator);
        static void startElement(void *ctx, const uint8 *name, const uint8 **atts);
        static void endElement(void *ctx, const uint8 *name);
        static void textHandler(void *ctx, const uint8 *name, int len);

    private:
        SAXDelegator*  delegator_;
    };

    typedef enum
    {
        SAX_NONE = 0,
        SAX_KEY,
        SAX_DICT,
        SAX_INT,
        SAX_REAL,
        SAX_STRING,
        SAX_ARRAY
    }SAXState;

    typedef enum
    {
        SAX_RESULT_NONE = 0,
        SAX_RESULT_DICT,
        SAX_RESULT_ARRAY
    }SAXResult;

    class DictMaker final : public SAXDelegator
    {
    public:
        SAXResult resultType_;
        ValueMap rootDict_;
        ValueVector rootArray_;

        std::string curKey_;   ///< parsed key
        std::string curValue_; // parsed value
        SAXState state_;

        ValueMap*  curDict_;
        ValueVector* curArray_;

        std::stack<ValueMap*> dictStack_;
        std::stack<ValueVector*> arrayStack_;
        std::stack<SAXState>  stateStack_;

    public:
        DictMaker();
        ~DictMaker();

        ValueMap dictionaryWithContentsOfFile(const std::string& fileName);
        ValueMap dictionaryWithDataOfFile(const char* filedata, int filesize);
        ValueVector arrayWithContentsOfFile(const std::string& fileName);

        void startElement(void *, const HBYTE *name, const HBYTE **);
        void endElement(void *, const char *name);

        void textHandler(void *, const char *ch, int len);
    };
}

#endif // DICTMAKER_H
