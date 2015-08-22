#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <string>
#include "GRAPH/RENDERER/Texture2D.h"

namespace GRAPH
{
    class ApplicationProtocol
    {
    public:
        virtual ~ApplicationProtocol(){
        }

        virtual void initGLContextAttrs() = 0;
        virtual bool applicationDidFinishLaunching() = 0;
        virtual void applicationDidEnterBackground() = 0;
        virtual void applicationWillEnterForeground() = 0;
    };

    class RGBAProtocol
    {
    public:
        virtual ~RGBAProtocol() {}

        virtual void setColor(const Color3B& color) = 0;
        virtual const Color3B& getColor() const = 0;
        virtual const Color3B& getDisplayedColor() const = 0;
        virtual GLubyte getDisplayedOpacity() const = 0;
        virtual GLubyte getOpacity() const = 0;
        virtual void setOpacity(GLubyte opacity) = 0;
        virtual void setOpacityModifyRGB(bool value) = 0;
        virtual bool isOpacityModifyRGB() const = 0;
        virtual bool isCascadeColorEnabled() const = 0;
        virtual void setCascadeColorEnabled(bool cascadeColorEnabled) = 0;
        virtual void updateDisplayedColor(const Color3B& color) = 0;
        virtual bool isCascadeOpacityEnabled() const = 0;
        virtual void setCascadeOpacityEnabled(bool cascadeOpacityEnabled) = 0;
        virtual void updateDisplayedOpacity(GLubyte opacity) = 0;
    };

    class BlendProtocol
    {
    public:
        virtual ~BlendProtocol() {}

        virtual void setBlendFunc(const BlendFunc &blendFunc) = 0;
        virtual const BlendFunc &getBlendFunc() const = 0;
    };

    class TextureProtocol : public BlendProtocol
    {
    public:
        virtual ~TextureProtocol() {}

        virtual Texture2D* getTexture() const = 0;
        virtual void setTexture(Texture2D *texture) = 0;
    };

    class LabelProtocol
    {
    public:
        virtual ~LabelProtocol() {}

        virtual void setString(const std::string &label) = 0;
        virtual const std::string& getString() const = 0;
    };

    class DirectorDelegate
    {
    public:
        virtual ~DirectorDelegate() {}

        virtual void updateProjection() = 0;
    };
}

#endif // PROTOCOLS_H
