#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

#include "BASE/HObject.h"
#include "BASE/HValue.h"
#include "GRAPH/BASE/GLCommon.h"

namespace GRAPH
{
    class Configuration : public HObject
    {
    public:

        /** Returns a shared instance of Configuration.
         *
         * @return An autoreleased Configuration object.
         */
        static Configuration *getInstance();

        /** Purge the shared instance of Configuration.
         */
        static void destroyInstance();

    public:
        /** Destructor
         * @js NA
         * @lua NA
         */
        virtual ~Configuration();

        /** OpenGL Max texture size.
         *
         * @return The OpenGL Max texture size.
         */
        int getMaxTextureSize() const;

        /** OpenGL Max Modelview Stack Depth.
         *
         * @return The OpenGL Max Modelview Stack Depth.
         */
        int getMaxModelviewStackDepth() const;

        /** Returns the maximum texture units.
         *
         * @return The maximum texture units.
         * @since v2.0.0
         */
        int getMaxTextureUnits() const;

        /** Whether or not the GPU supports NPOT (Non Power Of Two) textures.
         OpenGL ES 2.0 already supports NPOT (iOS).
         *
         * @return Is true if supports NPOT.
         * @since v0.99.2
         */
        bool supportsNPOT() const;

        /** Whether or not PVR Texture Compressed is supported.
         *
         * @return Is true if supports PVR Texture Compressed.
         */
        bool supportsPVRTC() const;

        /** Whether or not ETC Texture Compressed is supported.
         *
         *
         * @return Is true if supports ETC Texture Compressed.
         */
        bool supportsETC() const;

        /** Whether or not S3TC Texture Compressed is supported.
         *
         * @return Is true if supports S3TC Texture Compressed.
         */
        bool supportsS3TC() const;

        /** Whether or not ATITC Texture Compressed is supported.
         *
         * @return Is true if supports ATITC Texture Compressed.
         */
        bool supportsATITC() const;

        /** Whether or not BGRA8888 textures are supported.
         *
         * @return Is true if supports BGRA8888 textures.
         * @since v0.99.2
         */
        bool supportsBGRA8888() const;

        /** Whether or not glDiscardFramebufferEXT is supported.
         * @return Is true if supports glDiscardFramebufferEXT.
         * @since v0.99.2
         */
        bool supportsDiscardFramebuffer() const;

        /** Whether or not shareable VAOs are supported.
         *
         * @return Is true if supports shareable VAOs.
         * @since v2.0.0
         */
        bool supportsShareableVAO() const;

        /** Max support directional light in shader, for Sprite3D.
         *
         * @return Maximum supports directional light in shader.
         * @since v3.3
         */
        int getMaxSupportDirLightInShader() const;

        /** Max support point light in shader, for Sprite3D.
         *
         * @return Maximum supports point light in shader.
         * @since v3.3
         */
        int getMaxSupportPointLightInShader() const;

        /** Max support spot light in shader, for Sprite3D.
         *
         * @return Maximum supports spot light in shader.
         * @since v3.3
         */
        int getMaxSupportSpotLightInShader() const;

        /** Returns whether or not an OpenGL is supported.
         *
         * @param searchName A given search name.
         * @return Is true if an OpenGL is supported.
         */
        bool checkForGLExtension(const std::string &searchName) const;

        /** Initialize method.
         *
         * @return Is true if initialize success.
         */
        bool init();

        /** Returns the value of a given key as a double.
         *
         * @param key A given key.
         * @param defaultValue if not find the value, return the defaultValue.
         * @return
         */
        const HValue& getValue(const std::string& key, const HValue& defaultValue = HValue::Null) const;

        /** Sets a new key/value pair  in the configuration dictionary.
         *
         * @param key A given key.
         * @param value A given value.
         */
        void setValue(const std::string& key, const HValue& value);

        /** Gathers OpenGL / GPU information.
         */
        void gatherGPUInfo();

        /** Loads a config file. If the keys are already present, then they are going to be replaced. Otherwise the new keys are added.
         *
         * @param filename Config file name.
         */
        void loadConfigFile(const std::string& filename);

    private:
        Configuration(void);
        static Configuration    *s_sharedConfiguration;
        static std::string		s_configfile;

    protected:
        GLint           _maxTextureSize;
        GLint           _maxModelviewStackDepth;
        bool            _supportsPVRTC;
        bool            _supportsETC1;
        bool            _supportsS3TC;
        bool            _supportsATITC;
        bool            _supportsNPOT;
        bool            _supportsBGRA8888;
        bool            _supportsDiscardFramebuffer;
        bool            _supportsShareableVAO;
        GLint           _maxSamplesAllowed;
        GLint           _maxTextureUnits;
        char *          _glExtensions;
        int             _maxDirLightInShader; //max support directional light in shader
        int             _maxPointLightInShader; // max support point light in shader
        int             _maxSpotLightInShader; // max support spot light in shader

        ValueMap        _valueDict;
    };
}

#endif // __CCCONFIGURATION_H__
