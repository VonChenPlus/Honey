#include "Texture.h"

#include <cmath>

#include "SMARTGRAPH/GFX/GLState.h"
#include "IMAGE/TinyZim.h"
using IMAGE::ZIM_CLAMP;
using IMAGE::ZIM_HAS_MIPS;
using IMAGE::ZIM_GEN_MIPS;
using IMAGE::LoadZIM;
using IMAGE::ZIM_MAX_MIP_LEVELS;
using IMAGE::ZIM_FORMAT_MASK;
using IMAGE::ZIM_RGBA8888;
using IMAGE::ZIM_RGBA4444;
using IMAGE::ZIM_RGB565;
using IMAGE::ZIM_ETC1;
#include "SMARTGRAPH/GFX/GLDebug.h"
#include "BASE/Honey.h"
#include "IMAGE/TinyPNG.h"
using IMAGE::PNGLoad;
using IMAGE::PNGLoadPtr;
#include "EXTERNALS/jpge/jpgd.h"
#include "EXTERNALS/rg_etc1/rg_etc1.h"
#include "SMARTGRAPH/GFX/GLExtensions.h"
#include "UTILS/STRING/NString.h"
using UTILS::STRING::StringFromFormat;

namespace GLOBAL
{
    extern GFX::GLExtensions &glExtensions();
}

namespace GFX
{
    Texture::Texture() : id_(0) {
        CheckGLExtensions();
        register_gl_resource_holder(this);
    }

    Texture::~Texture() {
        unregister_gl_resource_holder(this);
        destroy();
    }

    void Texture::destroy() {
        if (id_) {
            glDeleteTextures(1, &id_);
            id_ = 0;
        }
    }

    void Texture::glLost() {
        if (!filename_.empty())
        {
            load(filename_.c_str());
            //ILOG("Reloaded lost texture %s", filename_.c_str());
        }
        else
        {
            //WLOG("Texture %p cannot be restored - has no filename", this);
            destroy();
        }
    }

    static void SetTextureParameters(int zim_flags) {
        GLenum wrap = GL_REPEAT;
        if (zim_flags & ZIM_CLAMP)
            wrap = GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
        GL_CHECK();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if ((zim_flags & (ZIM_HAS_MIPS | ZIM_GEN_MIPS)))
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        GL_CHECK();
    }

    static uint8 *GenerateTexture(const char *filename, int &bpp, int &w, int &h, bool &clamp) {
        UNUSED(clamp);
        char name_and_params[256];
        // security check :)
        if (strlen(filename) > 200)
            return 0;
        sscanf(filename, "gen:%i:%i:%s", &w, &h, name_and_params);

        uint8 *data;
        if (!strcmp(name_and_params, "vignette")) {
            bpp = 1;
            data = new uint8[w*h];
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; x++) {
                    float dx = (float)(x - w/2) / (w/2);
                    float dy = (float)(y - h/2) / (h/2);
                    float dist = sqrtf(dx * dx + dy * dy);
                    dist /= 1.414f;
                    float val = 1.0 - powf(dist, 1.4f);
                    data[y*w + x] = val * 255;
                }
            }
        }
        else if (!strcmp(name_and_params, "circle")) {
            bpp = 1;
            // TODO
            data = new uint8[w*h];
            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; x++) {
                    float dx = (float)(x - w/2) / (w/2);
                    float dy = (float)(y - h/2) / (h/2);
                    float dist = sqrtf(dx * dx + dy * dy);
                    dist /= 1.414f;
                    float val = 1.0 - powf(dist, 1.4f);
                    data[y*w + x] = val * 255;
                }
            }
        }
        else {
            data = NULLPTR;
        }

        return data;
    }

    void Texture::load(const char *filename) {
        // hook for generated textures
        if (!memcmp(filename, "gen:", 4)) {
            int bpp, w, h;
            bool clamp = false;
            uint8 *data = GenerateTexture(filename, bpp, w, h, clamp);
            if (!data)
                throw _NException_("GenerateTexture failed", NException::GFX);
            glGenTextures(1, &id_);
            glBindTexture(GL_TEXTURE_2D, id_);
            if (bpp == 1) {
                glTexImage2D(GL_TEXTURE_2D, 0, 1, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
            }
            else {
                throw _NException_Normal("unsupported image format!");
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            delete [] data;
        }

        filename_ = filename;

        // Currently here are a bunch of project-specific workarounds.
        // They shouldn't really hurt anything else very much though.

        Size len = strlen(filename);
        char fn[1024];
        strncpy(fn, filename, sizeof(fn));
        fn[1023] = 0;
        bool zim = false;
        if (!strcmp("dds", &filename[len-3])) {
            strcpy(&fn[len-3], "zim");
            zim = true;
        }
        if (!strcmp("6TX", &filename[len-3]) || !strcmp("6tx", &filename[len-3])) {
            //ILOG("Detected 6TX %s", filename);
            strcpy(&fn[len-3], "zim");
            zim = true;
        }
        for (int i = 0; i < (int)strlen(fn); i++) {
            if (fn[i] == '\\') fn[i] = '/';
        }

        if (fn[0] == 'm') fn[0] = 'M';
        const char *name = fn;
        if (zim && 0==memcmp(name, "Media/textures/", strlen("Media/textures"))) name += strlen("Media/textures/");
        len = strlen(name);
        if (!strcmp("png", &name[len-3]) || !strcmp("PNG", &name[len-3])) {
            loadPNG(fn);
        }
        else if (!strcmp("zim", &name[len-3])) {
            loadZIM(name);
        }
        else if (!strcmp("jpg", &name[len-3]) || !strcmp("JPG", &name[len-3]) ||
                !strcmp("jpeg", &name[len-4]) || !strcmp("JPEG", &name[len-4])) {
            loadJPEG(fn);
        }
        else if (!name || !strlen(name)) {
            throw _NException_(StringFromFormat("Failed to identify image file %s by extension", name), NException::GFX);
        }
        else {
            throw _NException_("Cannot load a texture with an empty filename", NException::GFX);
        }
    }

    void Texture::loadPNG(const char *filename, bool genMips) {
        unsigned char *image_data;
        PNGLoad(filename, &width_, &height_, &image_data);
        GL_CHECK();
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        SetTextureParameters(genMips ? ZIM_GEN_MIPS : ZIM_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        if (genMips) {
            if (GLOBAL::glExtensions().FBO_ARB) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                glGenerateMipmapEXT(GL_TEXTURE_2D);
            }
        }
        GL_CHECK();
        free(image_data);
    }

    void Texture::loadJPEG(const char *filename, bool genMips) {
        //ILOG("Loading jpeg %s", filename);
        unsigned char *image_data;
        int actual_comps;
        image_data = jpgd::decompress_jpeg_image_from_file(filename, &width_, &height_, &actual_comps, 4);
        if (!image_data) {
            throw _NException_("jpeg: image data returned was 0", NException::IO);
        }

        GL_CHECK();
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        SetTextureParameters(genMips ? ZIM_GEN_MIPS : ZIM_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        if (genMips) {
            if (GLOBAL::glExtensions().FBO_ARB) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                glGenerateMipmapEXT(GL_TEXTURE_2D);
            }
        }
        GL_CHECK();
        free(image_data);
    }

    void Texture::loadPNG(const uint8 *data, Size size, bool genMips) {
        unsigned char *image_data;
        PNGLoadPtr(data, size, &width_, &height_, &image_data);
        GL_CHECK();
        // TODO: should check for power of 2 tex size and disallow genMips when not.
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        SetTextureParameters(genMips ? ZIM_GEN_MIPS : ZIM_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        if (genMips) {
            if (GLOBAL::glExtensions().FBO_ARB) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                glGenerateMipmapEXT(GL_TEXTURE_2D);
            }
        }
        GL_CHECK();
        free(image_data);
    }

    void Texture::loadXOR() {
        width_ = height_ = 256;
        unsigned char *buf = new unsigned char[width_*height_*4];
        for (int y = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
                buf[(y*width_ + x)*4 + 0] = x^y;
                buf[(y*width_ + x)*4 + 1] = x^y;
                buf[(y*width_ + x)*4 + 2] = x^y;
                buf[(y*width_ + x)*4 + 3] = 0xFF;
            }
        }
        GL_CHECK();
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        SetTextureParameters(ZIM_GEN_MIPS);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, buf);
        if(GLOBAL::glExtensions().FBO_ARB) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            glGenerateMipmapEXT(GL_TEXTURE_2D);
        }
        GL_CHECK();
        delete [] buf;
    }

    // Allocates using new[], doesn't free.
    static uint8 *ETC1ToRGBA(uint8 *etc1, int width, int height) {
        uint8 *rgba = new uint8[width * height * 4];
        memset(rgba, 0xFF, width * height * 4);
        for (int y = 0; y < height; y += 4) {
            for (int x = 0; x < width; x += 4) {
                rg_etc1::unpack_etc1_block(etc1 + ((y / 4) * width/4 + (x / 4)) * 8,
                    (uint32 *)rgba + (y * width + x), width, false);
            }
        }
        return rgba;
    }

    void Texture::loadZIM(const char *filename) {
        uint8 *image_data[ZIM_MAX_MIP_LEVELS];
        int width[ZIM_MAX_MIP_LEVELS];
        int height[ZIM_MAX_MIP_LEVELS];

        int flags;
        int num_levels = LoadZIM(filename, &width[0], &height[0], &flags, &image_data[0]);
        if (num_levels >= ZIM_MAX_MIP_LEVELS)
            throw _NException_Normal("num_levels >= ZIM_MAX_MIP_LEVELS");
        width_ = width[0];
        height_ = height[0];
        int data_type = GL_UNSIGNED_BYTE;
        int colors = GL_RGBA;
        int storage = GL_RGBA;
        bool compressed = false;
        switch (flags & ZIM_FORMAT_MASK) {
        case ZIM_RGBA8888:
            data_type = GL_UNSIGNED_BYTE;
            break;
        case ZIM_RGBA4444:
            data_type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case ZIM_RGB565:
            data_type = GL_UNSIGNED_SHORT_5_6_5;
            colors = GL_RGB;
            storage = GL_RGB;
            break;
        case ZIM_ETC1:
            compressed = true;
            break;
        }

        GL_CHECK();

        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        SetTextureParameters(flags);

        if (compressed) {
            for (int l = 0; l < num_levels; l++) {
                int data_w = width[l];
                int data_h = height[l];
                if (data_w < 4) data_w = 4;
                if (data_h < 4) data_h = 4;
                // TODO: OpenGL 4.3+ accepts ETC1 so we should not have to do this anymore on those cards.
                // Also, iOS does not have support for ETC1 compressed textures so we just decompress.
                // TODO: Use PVR texture compression on iOS.
                image_data[l] = ETC1ToRGBA(image_data[l], data_w, data_h);
                glTexImage2D(GL_TEXTURE_2D, l, GL_RGBA, width[l], height[l], 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, image_data[l]);
            }
            GL_CHECK();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, num_levels - 2);
        }
        else {
            for (int l = 0; l < num_levels; l++) {
                glTexImage2D(GL_TEXTURE_2D, l, storage, width[l], height[l], 0,
                    colors, data_type, image_data[l]);
            }
            if (num_levels == 1 && (flags & ZIM_GEN_MIPS)) {
                if(GLOBAL::glExtensions().FBO_ARB) {
                    glGenerateMipmap(GL_TEXTURE_2D);
                }
                else {
    #ifndef USING_GLES2
                    glGenerateMipmapEXT(GL_TEXTURE_2D);
    #endif
                }
            }
        }
        SetTextureParameters(flags);

        GL_CHECK();
        // Only free the top level, since the allocation is used for all of them.
        free(image_data[0]);
    }

    void Texture::bind(int stage) {
        GL_CHECK();
        if (stage != -1)
            glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_2D, id_);
        GL_CHECK();
    }

    void Texture::unBind(int stage) {
        GL_CHECK();
        if (stage != -1)
            glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_2D, 0);
        GL_CHECK();
    }
}

