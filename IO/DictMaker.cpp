#include "DictMaker.h"
#include "EXTERNALS/tinyxml2/tinyxml2.h"
#include "IO/FileUtils.h"

namespace IO
{
    class XmlSaxHander : public tinyxml2::XMLVisitor
    {
    public:
        XmlSaxHander()
            : saxParserImp_(nullptr){
        }

        virtual bool visitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute );
        virtual bool visitExit(const tinyxml2::XMLElement& element );
        virtual bool visit(const tinyxml2::XMLText& text );
        virtual bool visit(const tinyxml2::XMLUnknown&){ return true; }

        void setSAXParserImp(SAXParser* parser) {
            saxParserImp_ = parser;
        }

    private:
        SAXParser *saxParserImp_;
    };

    bool XmlSaxHander::visitEnter( const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute ) {
        std::vector<const char*> attsVector;
        for( const tinyxml2::XMLAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next() ) {
            attsVector.push_back(attrib->Name());
            attsVector.push_back(attrib->Value());
        }

        attsVector.push_back(nullptr);

        SAXParser::startElement(saxParserImp_, element.Value(), &attsVector[0]);
        return true;
    }

    bool XmlSaxHander::visitExit( const tinyxml2::XMLElement& element ) {
        SAXParser::endElement(saxParserImp_, element.Value());
        return true;
    }

    bool XmlSaxHander::visit( const tinyxml2::XMLText& text ) {
        SAXParser::textHandler(saxParserImp_, text.Value(), static_cast<int>(strlen(text.Value())));
        return true;
    }

    SAXParser::SAXParser() {
        delegator_ = nullptr;
    }

    SAXParser::~SAXParser(void) {
    }

    bool SAXParser::parse(const HBYTE* xmlData, uint64 dataLength) {
        tinyxml2::XMLDocument tinyDoc;
        tinyDoc.Parse((const char *)xmlData, dataLength);
        XmlSaxHander printer;
        printer.setSAXParserImp(this);
        return tinyDoc.Accept( &printer );
    }

    bool SAXParser::parse(const std::string& filename) {
        bool ret = false;
        HData data = FileUtils::getInstance().getDataFromFile(filename);
        if (!data.isNull()) {
            ret = parse((const HBYTE*)data.getBytes(), data.getSize());
        }

        return ret;
    }

    void SAXParser::startElement(void *ctx, const char *name, const char **atts) {
        ((SAXParser*)(ctx))->delegator_->startElement(ctx, name, atts);
    }

    void SAXParser::endElement(void *ctx, const char *name) {
        ((SAXParser*)(ctx))->delegator_->endElement(ctx, name);
    }

    void SAXParser::textHandler(void *ctx, const char *name, int len) {
        ((SAXParser*)(ctx))->delegator_->textHandler(ctx, name, len);
    }

    void SAXParser::setDelegator(SAXDelegator* delegator) {
        delegator_ = delegator;
    }

    DictMaker::DictMaker()
        : resultType_(SAX_RESULT_NONE) {
    }

    DictMaker::~DictMaker() {
    }

    ValueMap DictMaker::dictionaryWithContentsOfFile(const std::string& fileName) {
        resultType_ = SAX_RESULT_DICT;
        SAXParser parser;
        parser.setDelegator(this);
        parser.parse(fileName);
        return rootDict_;
    }

    ValueMap DictMaker::dictionaryWithDataOfFile(const HBYTE* filedata, int filesize) {
        resultType_ = SAX_RESULT_DICT;
        SAXParser parser;
        parser.setDelegator(this);
        parser.parse(filedata, filesize);
        return rootDict_;
    }

    ValueVector DictMaker::arrayWithContentsOfFile(const std::string& fileName) {
        resultType_ = SAX_RESULT_ARRAY;
        SAXParser parser;
        parser.setDelegator(this);
        parser.parse(fileName);
        return rootArray_;
    }

    void DictMaker::startElement(void *, const char *name, const char **) {
        const std::string sName(name);
        if( sName == "dict" ) {
            if(resultType_ == SAX_RESULT_DICT && rootDict_.empty()) {
                curDict_ = &rootDict_;
            }

            state_ = SAX_DICT;

            SAXState preState = SAX_NONE;
            if (! stateStack_.empty()) {
                preState = stateStack_.top();
            }

            if (SAX_ARRAY == preState) {
                // add a new dictionary into the array
                curArray_->push_back(HValue(ValueMap()));
                curDict_ = &(curArray_->rbegin())->asValueMap();
            }
            else if (SAX_DICT == preState) {
                // add a new dictionary into the pre dictionary
                ValueMap* preDict = dictStack_.top();
                (*preDict)[curKey_] = HValue(ValueMap());
                curDict_ = &(*preDict)[curKey_].asValueMap();
            }

            // record the dict state
            stateStack_.push(state_);
            dictStack_.push(curDict_);
        }
        else if(sName == "key") {
            state_ = SAX_KEY;
        }
        else if(sName == "integer") {
            state_ = SAX_INT;
        }
        else if(sName == "real") {
            state_ = SAX_REAL;
        }
        else if(sName == "string") {
            state_ = SAX_STRING;
        }
        else if (sName == "array") {
            state_ = SAX_ARRAY;

            if (resultType_ == SAX_RESULT_ARRAY && rootArray_.empty()) {
                curArray_ = &rootArray_;
            }
            SAXState preState = SAX_NONE;
            if (! stateStack_.empty()) {
                preState = stateStack_.top();
            }

            if (preState == SAX_DICT) {
                (*curDict_)[curKey_] = HValue(ValueVector());
                curArray_ = &(*curDict_)[curKey_].asValueVector();
            }
            else if (preState == SAX_ARRAY) {
                ValueVector* preArray = arrayStack_.top();
                preArray->push_back(HValue(ValueVector()));
                curArray_ = &(curArray_->rbegin())->asValueVector();
            }
            // record the array state
            stateStack_.push(state_);
            arrayStack_.push(curArray_);
        }
        else {
            state_ = SAX_NONE;
        }
    }

    void DictMaker::endElement(void *, const char *name) {
        SAXState curState = stateStack_.empty() ? SAX_DICT : stateStack_.top();
        const std::string sName((char*)name);
        if( sName == "dict" ) {
            stateStack_.pop();
            dictStack_.pop();
            if ( !dictStack_.empty()) {
                curDict_ = dictStack_.top();
            }
        }
        else if (sName == "array") {
            stateStack_.pop();
            arrayStack_.pop();
            if (! arrayStack_.empty())
            {
                curArray_ = arrayStack_.top();
            }
        }
        else if (sName == "true") {
            if (SAX_ARRAY == curState) {
                curArray_->push_back(HValue(true));
            }
            else if (SAX_DICT == curState)
            {
                (*curDict_)[curKey_] = HValue(true);
            }
        }
        else if (sName == "false") {
            if (SAX_ARRAY == curState) {
                curArray_->push_back(HValue(false));
            }
            else if (SAX_DICT == curState)
            {
                (*curDict_)[curKey_] = HValue(false);
            }
        }
        else if (sName == "string" || sName == "integer" || sName == "real") {
            if (SAX_ARRAY == curState) {
                if (sName == "string")
                    curArray_->push_back(HValue(curValue_));
                else if (sName == "integer")
                    curArray_->push_back(HValue(atoi(curValue_.c_str())));
                else
                    curArray_->push_back(HValue(atof(curValue_.c_str())));
            }
            else if (SAX_DICT == curState) {
                if (sName == "string")
                    (*curDict_)[curKey_] = HValue(curValue_);
                else if (sName == "integer")
                    (*curDict_)[curKey_] = HValue(atoi(curValue_.c_str()));
                else
                    (*curDict_)[curKey_] = HValue(atof(curValue_.c_str()));
            }

            curValue_.clear();
        }

        state_ = SAX_NONE;
    }

    void DictMaker::textHandler(void *, const char *ch, int len) {
        if (state_ == SAX_NONE) {
            return;
        }

        SAXState curState = stateStack_.empty() ? SAX_DICT : stateStack_.top();
        const std::string text = std::string((char*)ch,len);

        switch(state_) {
        case SAX_KEY:
            curKey_ = text;
            break;
        case SAX_INT:
        case SAX_REAL:
        case SAX_STRING: {
                if (curState == SAX_DICT)
                    throw _HException_Normal("key not found : <integer/real>");

                curValue_.append(text);
            }
            break;
        default:
            break;
        }
    }
}
