#include "Thin3D.h"
#include "IMAGE/ZimLoad.h"
using IMAGE::ZIM_ETC1;
using IMAGE::ZIM_RGBA8888;
using IMAGE::ZIM_LUMINANCE;
using IMAGE::LoadZIM;
using IMAGE::LoadZIMPtr;
using IMAGE::ZIM_FORMAT_MASK;
#include "IMAGE/PNGLoad.h"
using IMAGE::PNGLoadPtr;
#include "EXTERNALS/jpge/jpgd.h"
#include "IO/FileUtil.h"
using IO::ReadLocalFile;
#include "UTILS/STRING/String.h"
using UTILS::STRING::StringFromFormat;

namespace THIN3D
{
    static const char * const glsl_fsTexCol =
    "#ifdef GL_ES\n"
    "precision lowp float;\n"
    "#endif\n"
    "varying vec4 oColor0;\n"
    "varying vec2 oTexCoord0;\n"
    "uniform sampler2D Sampler0;\n"
    "void main() { gl_FragColor = texture2D(Sampler0, oTexCoord0) * oColor0; }\n";

    static const char * const hlslFsTexCol =
    "struct PS_INPUT { float4 color : COLOR0; float2 uv : TEXCOORD0; };\n"
    "sampler2D Sampler0 : register(s0);\n"
    "float4 main(PS_INPUT input) : COLOR0 {\n"
    "  return input.color * tex2D(Sampler0, input.uv);\n"
    "}\n";

    static const char * const glsl_fsCol =
    "#ifdef GL_ES\n"
    "precision lowp float;\n"
    "#endif\n"
    "varying vec4 oColor0;\n"
    "void main() { gl_FragColor = oColor0; }\n";

    static const char * const hlslFsCol =
    "struct PS_INPUT { float4 color : COLOR0; };\n"
    "float4 main(PS_INPUT input) : COLOR0 {\n"
    "  return input.color;\n"
    "}\n";

    static const char * const glsl_vsCol =
    "attribute vec3 Position;\n"
    "attribute vec4 Color0;\n"
    "varying vec4 oColor0;\n"
    "uniform mat4 WorldViewProj;\n"
    "void main() {\n"
    "  gl_Position = WorldViewProj * vec4(Position, 1.0);\n"
    "  oColor0 = Color0;\n"
    "}";

    static const char * const hlslVsCol =
    "struct VS_INPUT { float3 Position : POSITION; float4 Color0 : COLOR0; };\n"
    "struct VS_OUTPUT { float4 Position : POSITION; float4 Color0 : COLOR0; };\n"
    "float4x4 WorldViewProj;\n"
    "VS_OUTPUT main(VS_INPUT input) {\n"
    "  VS_OUTPUT output;\n"
    "  output.Position = mul(float4(input.Position, 1.0), WorldViewProj);\n"
    "  output.Color0 = input.Color0;\n"
    "  return output;\n"
    "}\n";

    static const char * const glsl_vsTexCol =
    "attribute vec3 Position;\n"
    "attribute vec4 Color0;\n"
    "attribute vec2 TexCoord0;\n"
    "varying vec4 oColor0;\n"
    "varying vec2 oTexCoord0;\n"
    "uniform mat4 WorldViewProj;\n"
    "void main() {\n"
    "  gl_Position = WorldViewProj * vec4(Position, 1.0);\n"
    "  oColor0 = Color0;\n"
    "  oTexCoord0 = TexCoord0;\n"
    "}\n";

    static const char * const hlslVsTexCol =
    "struct VS_INPUT { float3 Position : POSITION; float2 Texcoord0 : TEXCOORD0; float4 Color0 : COLOR0; };\n"
    "struct VS_OUTPUT { float4 Position : POSITION; float2 Texcoord0 : TEXCOORD0; float4 Color0 : COLOR0; };\n"
    "float4x4 WorldViewProj;\n"
    "VS_OUTPUT main(VS_INPUT input) {\n"
    "  VS_OUTPUT output;\n"
    "  output.Position = mul(float4(input.Position, 1.0), WorldViewProj);\n"
    "  output.Texcoord0 = input.Texcoord0;\n"
    "  output.Color0 = input.Color0;\n"
    "  return output;\n"
    "}\n";

    void Thin3DContext::createPresets() {
        // Build prebuilt objects
        T3DBlendStateDesc off = { false };
        T3DBlendStateDesc additive = { true, T3DBlendEquation::ADD, T3DBlendFactor::ONE, T3DBlendFactor::ONE, T3DBlendEquation::ADD, T3DBlendFactor::ONE, T3DBlendFactor::ZERO };
        T3DBlendStateDesc standard_alpha = { true, T3DBlendEquation::ADD, T3DBlendFactor::SRC_ALPHA, T3DBlendFactor::ONE_MINUS_SRC_ALPHA, T3DBlendEquation::ADD, T3DBlendFactor::ONE, T3DBlendFactor::ZERO };
        T3DBlendStateDesc premul_alpha = { true, T3DBlendEquation::ADD, T3DBlendFactor::ONE, T3DBlendFactor::ONE_MINUS_SRC_ALPHA, T3DBlendEquation::ADD, T3DBlendFactor::ONE, T3DBlendFactor::ZERO };

        bsPresets_[BS_OFF] = createBlendState(off);
        bsPresets_[BS_ADDITIVE] = createBlendState(additive);
        bsPresets_[BS_STANDARD_ALPHA] = createBlendState(standard_alpha);
        bsPresets_[BS_PREMUL_ALPHA] = createBlendState(premul_alpha);

        vsPresets_[VS_TEXTURE_COLOR_2D] = createVertexShader(glsl_vsTexCol, hlslVsTexCol);
        vsPresets_[VS_COLOR_2D] = createVertexShader(glsl_vsCol, hlslVsCol);

        fsPresets_[FS_TEXTURE_COLOR_2D] = createFragmentShader(glsl_fsTexCol, hlslFsTexCol);
        fsPresets_[FS_COLOR_2D] = createFragmentShader(glsl_fsCol, hlslFsCol);

        ssPresets_[SS_TEXTURE_COLOR_2D] = createShaderSet(vsPresets_[VS_TEXTURE_COLOR_2D], fsPresets_[FS_TEXTURE_COLOR_2D]);
        ssPresets_[SS_COLOR_2D] = createShaderSet(vsPresets_[VS_COLOR_2D], fsPresets_[FS_COLOR_2D]);
    }

    Thin3DContext::~Thin3DContext() {
        for (int i = 0; i < VS_MAX_PRESET; i++) {
            if (vsPresets_[i]) {
                vsPresets_[i]->release();
            }
        }
        for (int i = 0; i < FS_MAX_PRESET; i++) {
            if (fsPresets_[i]) {
                fsPresets_[i]->release();
            }
        }
        for (int i = 0; i < BS_MAX_PRESET; i++) {
            if (bsPresets_[i]) {
                bsPresets_[i]->release();
            }
        }
        for (int i = 0; i < SS_MAX_PRESET; i++) {
            if (ssPresets_[i]) {
                ssPresets_[i]->release();
            }
        }
    }

    static T3DImageFormat ZimToT3DFormat(int zim) {
        switch (zim) {
        case ZIM_ETC1: return T3DImageFormat::ETC1;
        case ZIM_LUMINANCE: return T3DImageFormat::LUMINANCE;
        case ZIM_RGBA8888:
        default: return T3DImageFormat::RGBA8888;
        }
    }

    static T3DImageType DetectImageFileType(const uint8 *data, Size size) {
        UNUSED(size);
        if (!memcmp(data, "ZIMG", 4)) {
            return ZIM;
        }
        else if (!memcmp(data, "\x89\x50\x4E\x47", 4)) {
            return PNG;
        }
        else if (!memcmp(data, "\xff\xd8\xff\xe0", 4)) {
            return JPEG;
        }

        return TYPE_UNKNOWN;
    }

    static void LoadTextureLevels(const uint8 *data, Size size, T3DImageType type, int width[16], int height[16], int *num_levels, T3DImageFormat *fmt, uint8 *image[16], int *zim_flags) {
        if (type == DETECT) {
            type = DetectImageFileType(data, size);
        }
        if (type == TYPE_UNKNOWN) {
            throw _NException_Normal("File has unknown format");
        }

        *num_levels = 0;
        *zim_flags = 0;

        switch (type) {
        case ZIM: {
                *num_levels = LoadZIMPtr((const uint8 *)data, size, width, height, zim_flags, image);
                *fmt = ZimToT3DFormat(*zim_flags & ZIM_FORMAT_MASK);
            }
            break;
        case PNG: {
                PNGLoadPtr((const unsigned char *)data, size, &width[0], &height[0], &image[0]);
                *num_levels = 1;
                *fmt = RGBA8888;
            }
            break;
        case JPEG: {
                int actual_components = 0;
                unsigned char *jpegBuf = jpgd::decompress_jpeg_image_from_memory(data, (int)size, &width[0], &height[0], &actual_components, 4);
                if (jpegBuf) {
                    *num_levels = 1;
                    *fmt = RGBA8888;
                    image[0] = (uint8 *)jpegBuf;
                }
            }
            break;
        default:
            throw _NException_Normal("File has unknown format");
        }
    }

    void Thin3DTexture::loadFromFileData(const uint8 *data, Size dataSize, T3DImageType type) {
        int width[16], height[16];
        uint8 *image[16] = { NULLPTR };

        int num_levels;
        int zim_flags;
        T3DImageFormat fmt;

        LoadTextureLevels(data, dataSize, type, width, height, &num_levels, &fmt, image, &zim_flags);

        create(LINEAR2D, fmt, width[0], height[0], 1, num_levels);
        for (int i = 0; i < num_levels; i++) {
            if (image[i]) {
                setImageData(0, 0, 0, width[i], height[i], 1, i, width[i] * 4, image[i]);
                free(image[i]);
            }
            else {
                throw _NException_Normal(StringFromFormat("Missing image level %i", i));
            }
        }

        finalize(zim_flags);
    }

    void Thin3DTexture::loadFromFile(const std::string &filename, T3DImageType type) {
        filename_ = "";
        Size fileSize;
        uint8 *buffer = ReadLocalFile(filename.c_str(), &fileSize);
        loadFromFileData(buffer, fileSize, type);
        filename_ = filename;
        delete[] buffer;
    }

    Thin3DTexture *Thin3DContext::createTextureFromFile(const char *filename, T3DImageType type) {
        Thin3DTexture *tex = createTexture();
        tex->loadFromFile(filename, type);
        return tex;
    }

    // TODO: Remove the code duplication between this and LoadFromFileData
    Thin3DTexture *Thin3DContext::createTextureFromFileData(const uint8 *data, int size, T3DImageType type) {
        int width[16], height[16];
        int num_levels = 0;
        int zim_flags = 0;
        T3DImageFormat fmt;
        uint8 *image[16] = { NULLPTR };

       LoadTextureLevels(data, size, type, width, height, &num_levels, &fmt, image, &zim_flags);

        Thin3DTexture *tex = createTexture(LINEAR2D, fmt, width[0], height[0], 1, num_levels);
        for (int i = 0; i < num_levels; i++) {
            tex->setImageData(0, 0, 0, width[i], height[i], 1, i, width[i] * 4, image[i]);
            free(image[i]);
        }

        tex->finalize(zim_flags);
        return tex;
    }
}
