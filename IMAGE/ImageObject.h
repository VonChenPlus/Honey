#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include "BASE/HObject.h"
#include "IMAGE/ImageDefine.h"

namespace IMAGE
{
    class ImageObject : public HObject
    {
    public:
        ImageObject();
        virtual ~ImageObject();

        enum class Format
        {
            JPG,
            PNG,
            ETC,
            RAW_DATA,
            UNKNOWN
        };

        bool initWithImageFile(const std::string& path);
        bool initWithImageData(const unsigned char *data, uint64 dataLen);
        bool initWithRawData(const unsigned char *data, uint64 dataLen, int width, int height, int bitsPerComponent, bool preMulti = false);
        bool initWithImageFileThreadSafe(const std::string& fullpath);

        // Getters
        inline unsigned char *   getData()               { return data_; }
        inline uint64           getDataLen()            { return dataLen_; }
        inline Format            getFileType()           {return fileType_; }
        inline ImageFormat       getRenderFormat()       { return renderFormat_; }
        inline int               getWidth()              { return width_; }
        inline int               getHeight()             { return height_; }
        inline bool              hasPremultipliedAlpha() { return hasPremultipliedAlpha_; }

    protected:
        bool initWithJpgData(const unsigned char *data, uint64 dataLen);
        bool initWithPngData(const unsigned char *data, uint64 dataLen);
        bool initWithETCData(const unsigned char *data, uint64 dataLen);

        void premultipliedAlpha();

    protected:
        // noncopyable
        ImageObject(const ImageObject&    rImg);
        ImageObject & operator=(const ImageObject&);

        Format detectFormat(const unsigned char * data, uint64 dataLen);
        bool isPng(const unsigned char * data, uint64 dataLen);
        bool isJpg(const unsigned char * data, uint64 dataLen);
        bool isEtc(const unsigned char * data, uint64 dataLen);

    protected:
        unsigned char *data_;
        uint64 dataLen_;
        int width_;
        int height_;
        Format fileType_;
        ImageFormat renderFormat_;
        // false if we cann't auto detect the image is premultiplied or not.
        bool hasPremultipliedAlpha_;
        std::string filePath_;
    };
}

#endif // IMAGEOBJECT_H
