#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include <string>
#include <map>
#include <unordered_map>

#include "BASE/HObject.h"
#include "GRAPH/BASE/GLCommon.h"
#include "MATH/Vector.h"
#include "MATH/Rectangle.h"
#include "MATH/Size.h"
#include "GRAPH/BASE/Types.h"
#include "GRAPH/BASE/Fonts.h"
#include "IMAGE/SmartImage.h"

namespace GRAPH
{
    typedef struct _MipmapInfo
    {
        unsigned char* address;
        int len;
        _MipmapInfo():address(NULL),len(0){}
    } MipmapInfo;

    namespace ui
    {
        class Scale9Sprite;
    }

    class GLProgram;
    class SpriteFrame;

    class Texture2D : public HObject
    {
    public:
        /**
         Extension to set the Min / Mag filter
         */
        typedef struct _TexParams {
            GLuint    minFilter;
            GLuint    magFilter;
            GLuint    wrapS;
            GLuint    wrapT;
        }TexParams;

    public:
        /** sets the default pixel format for UIImagescontains alpha channel.

         @param format
         If the UIImage contains alpha channel, then the options are:
         - generate 32-bit textures: IMAGE::PixelFormat::RGBA8888 (default one)
         - generate 24-bit textures: IMAGE::PixelFormat::RGB888
         - generate 16-bit textures: IMAGE::PixelFormat::RGBA4444
         - generate 16-bit textures: IMAGE::PixelFormat::RGB5A1
         - generate 16-bit textures: IMAGE::PixelFormat::RGB565
         - generate 8-bit textures: IMAGE::PixelFormat::A8 (only use it if you use just 1 color)

         How does it work ?
         - If the image is an RGBA (with Alpha) then the default pixel format will be used (it can be a 8-bit, 16-bit or 32-bit texture)
         - If the image is an RGB (without Alpha) then: If the default pixel format is RGBA8888 then a RGBA8888 (32-bit) will be used. Otherwise a RGB565 (16-bit texture) will be used.

         This parameter is not valid for PVR / PVR.CCZ images.

         @since v0.8
         */
        static void setDefaultAlphaPixelFormat(IMAGE::PixelFormat format);

        /** Returns the alpha pixel format.
         @since v0.8
         */
        static IMAGE::PixelFormat getDefaultAlphaPixelFormat();

    public:
        /**
         * @js ctor
         */
        Texture2D();
        /**
         * @js NA
         * @lua NA
         */
        virtual ~Texture2D();

        /** Release only the gl texture.
         * @js NA
         * @lua NA
         */
        void releaseGLTexture();

        /** Initializes with a texture2d with data.

         @param data Specifies a pointer to the image data in memory.
         @param dataLen The image data length.
         @param pixelFormat The image pixelFormat.
         @param pixelsWide The image width.
         @param pixelsHigh The image height.
         @param contentSize The image content size.
         * @js NA
         * @lua NA
         */
        bool initWithData(const void *data, ssize_t dataLen, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh, const MATH::Sizef& contentSize);

        /** Initializes with mipmaps.

         @param mipmaps Specifies a pointer to the image data in memory.
         @param mipmapsNum The mipmaps number.
         @param pixelFormat The image pixelFormat.
         @param pixelsWide The image width.
         @param pixelsHigh The image height.
         */
        bool initWithMipmaps(MipmapInfo* mipmaps, int mipmapsNum, IMAGE::PixelFormat pixelFormat, int pixelsWide, int pixelsHigh);

        /** Update with texture data.

         @param data Specifies a pointer to the image data in memory.
         @param offsetX Specifies a texel offset in the x direction within the texture array.
         @param offsetY Specifies a texel offset in the y direction within the texture array.
         @param width Specifies the width of the texture subimage.
         @param height Specifies the height of the texture subimage.
         */
        bool updateWithData(const void *data,int offsetX,int offsetY,int width,int height);
        /**
        Drawing extensions to make it easy to draw basic quads using a Texture2D object.
        These functions require GL_TEXTURE_2D and both GL_VERTEX_ARRAY and GL_TEXTURE_COORD_ARRAY client states to be enabled.
        */
        /** Draws a texture at a given point. */
        void drawAtPoint(const MATH::Vector2f& point);
        /** Draws a texture inside a rect.*/
        void drawInRect(const MATH::Rectf& rect);

        /**
        Extensions to make it easy to create a Texture2D object from an image file.
        */
        /**
        Initializes a texture from a UIImage object.

        We will use the format you specified with setDefaultAlphaPixelFormat to convert the image for texture.
        NOTE: It will not convert the pvr image file.
        @param image An UIImage object.
        */
        bool initWithImage(IMAGE::SmartImage * image);

        /**
        Initializes a texture from a UIImage object.

        We will use the format you passed to the function to convert the image format to the texture format.
        If you pass PixelFormat::Automatic, we will auto detect the image render type and use that type for texture to render.
        @param image An UIImage object.
        @param format Texture pixel formats.
        **/
        bool initWithImage(IMAGE::SmartImage * image, IMAGE::PixelFormat format);

        /** Initializes a texture from a string with dimensions, alignment, font name and font size.

         @param text A null terminated string.
         @param fontName The font name.
         @param fontSize The font size.
         @param dimensions The font dimension.
         @param hAlignment The font horizontal text alignment type.
         @param vAlignment The font vertical text alignment type.
         */
        bool initWithString(const char *text,  const std::string &fontName, float fontSize, const MATH::Sizef& dimensions = MATH::Sizef(0, 0), TextHAlignment hAlignment = TextHAlignment::CENTER, TextVAlignment vAlignment = TextVAlignment::TOP);

        /** Initializes a texture from a string using a text definition.

         @param text A null terminated string.
         @param textDefinition A FontDefinition object contains font attributes.
         */
        bool initWithString(const char *text, const FontDefinition& textDefinition);

        /** Sets the min filter, mag filter, wrap s and wrap t texture parameters.
        If the texture size is NPOT (non power of 2), then in can only use GL_CLAMP_TO_EDGE in GL_TEXTURE_WRAP_{S,T}.

        @warning Calling this method could allocate additional texture memory.

        @since v0.8
        * @code
        * When this function bound into js or lua,the input parameter will be changed
        * In js: var setBlendFunc(var arg1, var arg2, var arg3, var arg4)
        * In lua: local setBlendFunc(local arg1, local arg2, local arg3, local arg4)
        * @endcode
        */
        void setTexParameters(const TexParams& texParams);

        /** Sets antialias texture parameters:
        - GL_TEXTURE_MIN_FILTER = GL_LINEAR
        - GL_TEXTURE_MAG_FILTER = GL_LINEAR

        @warning Calling this method could allocate additional texture memory.

        @since v0.8
        */
        void setAntiAliasTexParameters();

        /** Sets alias texture parameters:
        - GL_TEXTURE_MIN_FILTER = GL_NEAREST
        - GL_TEXTURE_MAG_FILTER = GL_NEAREST

        @warning Calling this method could allocate additional texture memory.

        @since v0.8
        */
        void setAliasTexParameters();


        /** Generates mipmap images for the texture.
        It only works if the texture size is POT (power of 2).
        @since v0.99.0
        */
        void generateMipmap();

        const char* getStringForFormat() const;

        /** Returns the bits-per-pixel of the in-memory OpenGL texture
        @since v1.0
        */
        unsigned int getBitsPerPixelForFormat() const;

        /** Helper functions that returns bits per pixels for a given format.
         @since v2.0
         */
        unsigned int getBitsPerPixelForFormat(IMAGE::PixelFormat format) const;

        /** Get content size. */
        const MATH::Sizef& getContentSizeInPixels();

        /** Whether or not the texture has their Alpha premultiplied. */
        bool hasPremultipliedAlpha() const;

        /** Whether or not the texture has mip maps.*/
        bool hasMipmaps() const;

        /** Gets the pixel format of the texture. */
        IMAGE::PixelFormat getPixelFormat() const;

        /** Gets the width of the texture in pixels. */
        int getPixelsWide() const;

        /** Gets the height of the texture in pixels. */
        int getPixelsHigh() const;

        /** Gets the texture name. */
        GLuint getName() const;

        /** Gets max S. */
        GLfloat getMaxS() const;
        /** Sets max S. */
        void setMaxS(GLfloat maxS);

        /** Gets max T. */
        GLfloat getMaxT() const;
        /** Sets max T. */
        void setMaxT(GLfloat maxT);

        /** Get the texture content size.*/
        MATH::Sizef getContentSize() const;

        /** Set a shader program to the texture.

         It's used by drawAtPoint and drawInRect
         */
        void setGLProgram(GLProgram* program);

        /** Get a shader program from the texture.*/
        GLProgram* getGLProgram() const;

        void removeSpriteFrameCapInset(SpriteFrame* spriteFrame);

    private:
        /**
         * Whether the texture contains a 9-patch capInset info or not.
         *
         * @return True is Texture contains a 9-patch info, false otherwise.
         */
        bool isContain9PatchInfo()const;

    protected:
        /** pixel format of the texture */
        IMAGE::PixelFormat _pixelFormat;

        /** width in pixels */
        int _pixelsWide;

        /** height in pixels */
        int _pixelsHigh;

        /** texture name */
        GLuint _name;

        /** texture max S */
        GLfloat _maxS;

        /** texture max T */
        GLfloat _maxT;

        /** content size */
        MATH::Sizef _contentSize;

        /** whether or not the texture has their Alpha premultiplied */
        bool _hasPremultipliedAlpha;

        /** whether or not the texture has mip maps*/
        bool _hasMipmaps;

        /** shader program used by drawAtPoint and drawInRect */
        GLProgram* _shaderProgram;

        static const IMAGE::PixelFormatInfoMap _pixelFormatInfoTables;

        bool _antialiasEnabled;
        friend class SpriteFrameCache;
        friend class TextureCache;
        friend class ui::Scale9Sprite;
    };
}

#endif // TEXTURE2D_H

