#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <string>
#include "GRAPH/Color.h"
#include "GRAPH/Types.h"

namespace GRAPH
{
    class Unity3DTexture;

    class RGBAProtocol
    {
    public:
        virtual ~RGBAProtocol() {}

        virtual void setColor(const Color3B& color) = 0;
        virtual const Color3B& getColor() const = 0;
        virtual const Color3B& getDisplayedColor() const = 0;
        virtual uint8 getDisplayedOpacity() const = 0;
        virtual uint8 getOpacity() const = 0;
        virtual void setOpacity(uint8 opacity) = 0;
        virtual void setOpacityModifyRGB(bool value) = 0;
        virtual bool isOpacityModifyRGB() const = 0;
        virtual bool isCascadeColorEnabled() const = 0;
        virtual void setCascadeColorEnabled(bool cascadeColorEnabled) = 0;
        virtual void updateDisplayedColor(const Color3B& color) = 0;
        virtual bool isCascadeOpacityEnabled() const = 0;
        virtual void setCascadeOpacityEnabled(bool cascadeOpacityEnabled) = 0;
        virtual void updateDisplayedOpacity(uint8 opacity) = 0;
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

        virtual Unity3DTexture* getTexture() const = 0;
        virtual void setTexture(Unity3DTexture *texture) = 0;
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
