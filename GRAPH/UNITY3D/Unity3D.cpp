#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/UNITY3D/Unity3DGL.h"
#include "IMAGE/ImageConvert.h"
#include "MATH/Size.h"

namespace GRAPH
{
    bool Unity3DTexture::initWithData(const void *data, uint64 dataLen, IMAGE::ImageFormat imageFormat, uint32 imageWidth, uint32 imageHeight) {
        U3DMipmap mipmap;
        mipmap.address = (unsigned char*) data;
        mipmap.length = static_cast<int>(dataLen);
        return initWithMipmaps(&mipmap, 1, imageFormat, imageWidth, imageHeight);
    }

    bool Unity3DTexture::initWithString(const char *string, U3DStringToTexture loader, void *loaderOwner) {
        if (loader == nullptr)
            throw _HException_Normal("StringToTexture is NULL!");

        IMAGE::ImageFormat pixelFormat = IMAGE::ImageFormat::DEFAULT;
        HBYTE* outTempData = nullptr;
        uint64 outTempDataLen = 0;

        uint32 imageWidth;
        uint32 imageHeight;
        bool hasPremultipliedAlpha = false;
        HData outData = loader(loaderOwner, string, imageWidth, imageHeight, hasPremultipliedAlpha);
        if (outData.isNull()) {
            return false;
        }

        MATH::Sizef  imageSize = MATH::Sizef((float) imageWidth, (float) imageHeight);
        pixelFormat = IMAGE::convertDataToFormat(outData.getBytes(), imageWidth*imageHeight * 4, IMAGE::ImageFormat::RGBA8888, pixelFormat, &outTempData, &outTempDataLen);

        bool ret = initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight);

        if (outTempData != nullptr && outTempData != outData.getBytes()) {
            free(outTempData);
        }
        premultipliedAlpha_ = hasPremultipliedAlpha;

        return ret;
    }

    bool Unity3DTexture::initWithImage(IMAGE::ImageObject *image) {
        return initWithImage(image, IMAGE::ImageFormat::DEFAULT);
    }

    bool Unity3DTexture::initWithImage(IMAGE::ImageObject *image, IMAGE::ImageFormat format) {
        if (image == nullptr) {
            return false;
        }

        int imageWidth = image->getWidth();
        int imageHeight = image->getHeight();

        unsigned char*   tempData = image->getData();
        MATH::Sizef      imageSize = MATH::Sizef((float) imageWidth, (float) imageHeight);
        IMAGE::ImageFormat      pixelFormat = ((IMAGE::ImageFormat::NONE == format) || (IMAGE::ImageFormat::AUTO == format)) ? image->getRenderFormat() : format;
        IMAGE::ImageFormat      renderFormat = image->getRenderFormat();
        uint64	         tempDataLen = image->getDataLen();


        if (imageFormatInfoMap().at(renderFormat).compressed) {
            initWithData(tempData, tempDataLen, image->getRenderFormat(), imageWidth, imageHeight);
            return true;
        }
        else {
            unsigned char* outTempData = nullptr;
            uint64 outTempDataLen = 0;

            pixelFormat = IMAGE::convertDataToFormat(tempData, tempDataLen, renderFormat, pixelFormat, &outTempData, &outTempDataLen);

            initWithData(outTempData, outTempDataLen, pixelFormat, imageWidth, imageHeight);

            if (outTempData != nullptr && outTempData != tempData) {
                free(outTempData);
            }

            premultipliedAlpha_ = image->hasPremultipliedAlpha();

            return true;
        }
    }

    Unity3DCreator::RenderEngine Unity3DCreator::EngineMode = OPENGL;

    Unity3DContext *Unity3DCreator::CreateContext() {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateContext();
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DDepthState *Unity3DCreator::CreateDepthState() {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateDepthState();
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DBuffer *Unity3DCreator::CreateBuffer(uint32 usageFlags) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateBuffer(usageFlags);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DShaderSet *Unity3DCreator::CreateShaderSet(Unity3DShader *vshader, Unity3DShader *fshader) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateShaderSet(vshader, fshader);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DVertexFormat *Unity3DCreator::CreateVertexFormat(const std::vector<Unity3DVertexComponent> &components, int stride) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateVertexFormat(components, stride);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }

    Unity3DTexture *Unity3DCreator::CreateTexture(U3DTextureType type, bool antialias) {
        switch (EngineMode)
        {
        case OPENGL:
            return Unity3DGLCreator::CreateTexture(type, antialias);
        default:
            throw _HException_Normal("Unsupport Engine Mode!");
            break;
        }
    }
}
