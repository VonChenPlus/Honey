#include <string.h>
#include "IMAGE/ImageObject.h"
#include "BASE/HData.h"
#include "IO/FileUtils.h"
#include "IMAGE/PNGHandler.h"
#include "GRAPH/Color.h"
#include "EXTERNALS/rg_etc1/etc1.h"
#include "EXTERNALS/jpge/jpgd.h"

namespace IMAGE
{
    ImageObject::ImageObject()
        : data_(nullptr)
        , dataLen_(0)
        , width_(0)
        , height_(0)
        , fileType_(Format::UNKNOWN)
        , renderFormat_(ImageFormat::NONE)
        , hasPremultipliedAlpha_(true) {

    }

    ImageObject::~ImageObject() {
        SAFE_FREE(data_);
    }

    bool ImageObject::initWithImageFile(const std::string& path) {
        bool ret = false;
        filePath_ = IO::FileUtils::getInstance().fullPathForFilename(path);

        HData data = IO::FileUtils::getInstance().getDataFromFile(filePath_);

        if (!data.isNull()) {
            ret = initWithImageData((const unsigned char *)data.getBytes(), data.getSize());
        }

        return ret;
    }

    bool ImageObject::initWithImageFileThreadSafe(const std::string& fullpath) {
        bool ret = false;
        filePath_ = fullpath;

        HData data = IO::FileUtils::getInstance().getDataFromFile(fullpath);

        if (!data.isNull()) {
            ret = initWithImageData((const unsigned char *)data.getBytes(), data.getSize());
        }

        return ret;
    }

    bool ImageObject::initWithImageData(const unsigned char *data, uint64 dataLen) {
        bool ret = false;

        do {
            if (! data || dataLen <= 0) break;

            unsigned char* unpackedData = nullptr;
            uint64 unpackedLen = 0;

            unpackedData = const_cast<unsigned char*>(data);
            unpackedLen = dataLen;

            fileType_ = detectFormat(unpackedData, unpackedLen);

            switch (fileType_) {
            case Format::PNG:
                ret = initWithPngData(unpackedData, unpackedLen);
                break;
            case Format::JPG:
                ret = initWithJpgData(unpackedData, unpackedLen);
                break;
            case Format::ETC:
                ret = initWithETCData(unpackedData, unpackedLen);
                break;

            default:
                ;
            }

            if(unpackedData != data) {
                free(unpackedData);
            }
        } while (0);

        return ret;
    }

    bool ImageObject::isPng(const unsigned char *data, uint64 dataLen) {
        if (dataLen <= 8) {
            return false;
        }

        static unsigned char PNG_SIGNATURE[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};

        return memcmp(PNG_SIGNATURE, data, sizeof(PNG_SIGNATURE)) == 0;
    }

    bool ImageObject::isEtc(const unsigned char * data, uint64) {
        return etc1_pkm_is_valid((etc1_byte*)data) ? true : false;
    }

    bool ImageObject::isJpg(const unsigned char * data, uint64 dataLen) {
        if (dataLen <= 4) {
            return false;
        }

        static unsigned char JPG_SOI[] = {0xFF, 0xD8};

        return memcmp(data, JPG_SOI, 2) == 0;
    }

    ImageObject::Format ImageObject::detectFormat(const unsigned char * data, uint64 dataLen) {
        if (isPng(data, dataLen)) {
            return Format::PNG;
        }
        else if (isJpg(data, dataLen)) {
            return Format::JPG;
        }
        else if (isEtc(data, dataLen)) {
            return Format::ETC;
        }
        else {
            return Format::UNKNOWN;
        }
    }

    bool ImageObject::initWithJpgData(const unsigned char * data, uint64 dataLen) {
        int actual_components = 0;
        data_ = (unsigned char*)jpgd::decompress_jpeg_image_from_memory((const unsigned char*)data, (int)dataLen, &width_, &height_, &actual_components, 4);
        renderFormat_ = ImageFormat::RGB888;

        return true;
    }

    bool ImageObject::initWithPngData(const unsigned char *data, uint64 dataLen) {
        int color_type;
        PNGLoadPtr((const unsigned char*)data, dataLen, &width_, &height_, &color_type, (unsigned char**)&data_, (int *)&dataLen_);

        switch (color_type) {
        case PNG_COLOR_TYPE_GRAY:
            renderFormat_ = ImageFormat::I8;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            renderFormat_ = ImageFormat::AI88;
            break;
        case PNG_COLOR_TYPE_RGB:
            renderFormat_ = ImageFormat::RGB888;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            renderFormat_ = ImageFormat::RGBA8888;
            break;
        default:
            break;
        }

        // premultiplied alpha for RGBA8888
        if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
            premultipliedAlpha();
        }
        else {
            hasPremultipliedAlpha_ = false;
        }

        return true;
    }

    bool ImageObject::initWithETCData(const unsigned char * data, uint64) {
        const etc1_byte* header = static_cast<const etc1_byte*>(data);

        //check the data
        if (! etc1_pkm_is_valid(header)) {
            return  false;
        }

        width_ = etc1_pkm_get_width(header);
        height_ = etc1_pkm_get_height(header);

        if (0 == width_ || 0 == height_) {
            return false;
        }

         //if it is not gles or device do not support ETC, decode texture by software
        int bytePerPixel = 3;
        unsigned int stride = width_ * bytePerPixel;
        renderFormat_ = ImageFormat::RGB888;

        dataLen_ =  width_ * height_ * bytePerPixel;
        data_ = static_cast<unsigned char*>(malloc(dataLen_ * sizeof(unsigned char)));

        if (etc1_decode_image(static_cast<const unsigned char*>(data) + ETC_PKM_HEADER_SIZE, static_cast<etc1_byte*>(data_), width_, height_, bytePerPixel, stride) != 0) {
            dataLen_ = 0;
            if (data_ != nullptr) {
                free(data_);
            }
            return false;
        }

        return true;
    }

    bool ImageObject::initWithRawData(const unsigned char * data, uint64, int width, int height, int, bool preMulti) {
        bool ret = false;
        do {
            if(0 == width || 0 == height) break;

            height_   = height;
            width_    = width;
            hasPremultipliedAlpha_ = preMulti;
            renderFormat_ = ImageFormat::RGBA8888;

            // only RGBA8888 supported
            int bytesPerComponent = 4;
            dataLen_ = height * width * bytesPerComponent;
            data_ = static_cast<unsigned char*>(malloc(dataLen_ * sizeof(unsigned char)));
            if(! data_) break;
            memcpy(data_, data, dataLen_);

            ret = true;
        } while (0);

        return ret;
    }

    void ImageObject::premultipliedAlpha() {
        uint32* fourBytes = (unsigned int*)data_;
        for(int i = 0; i < width_ * height_; i++) {
            unsigned char* p = data_ + i * 4;
            fourBytes[i] = GRAPH::Color4B::PreMultiplyAlpha(GRAPH::Color4B(p[0], p[1], p[2], p[3]));
        }

        hasPremultipliedAlpha_ = true;
    }
}

