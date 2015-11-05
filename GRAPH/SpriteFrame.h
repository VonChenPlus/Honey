#ifndef SPRITEFRAME_H
#define SPRITEFRAME_H

#include <string>
#include <set>
#include "BASE/HValue.h"
#include "GRAPH/Node.h"

namespace GRAPH
{
    class Unity3DTexture;

    class SpriteFrame : public HObject
    {
    public:
        static SpriteFrame* create(const std::string& filename, const MATH::Rectf& rect);
        static SpriteFrame* create(const std::string& filename, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize);
        static SpriteFrame* createWithTexture(Unity3DTexture* pobTexture, const MATH::Rectf& rect);
        static SpriteFrame* createWithTexture(Unity3DTexture* pobTexture, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize);

        inline bool isRotated() const { return rotated_; }
        inline void setRotated(bool rotated) { rotated_ = rotated; }

        inline const MATH::Rectf& getRect() const { return rect_; }
        void setRect(const MATH::Rectf& rect);

        inline const MATH::Sizef& getOriginalSize() const { return originalSize_; }
        inline void setOriginalSize(const MATH::Sizef& sizeInPixels) { originalSize_ = sizeInPixels; }

        Unity3DTexture* getTexture();
        void setTexture(Unity3DTexture* pobTexture);

        const MATH::Vector2f& getOffset() const;
        void setOffset(const MATH::Vector2f& offsets);

    public:
        SpriteFrame();
        virtual ~SpriteFrame();

        bool initWithTexture(Unity3DTexture* pobTexture, const MATH::Rectf& rect);
        bool initWithTextureFilename(const std::string& filename, const MATH::Rectf& rect);
        bool initWithTexture(Unity3DTexture* pobTexture, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize);
        bool initWithTextureFilename(const std::string& filename, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize);

    protected:
        MATH::Vector2f offset_;
        MATH::Sizef originalSize_;
        bool   rotated_;
        MATH::Rectf rect_;
        Unity3DTexture *texture_;
        std::string  textureFilename_;
    };

    class SpriteFrameCache : public HObject
    {
    public:
        static SpriteFrameCache& getInstance();

        virtual ~SpriteFrameCache();

        bool init();

        void addSpriteFramesWithFile(const std::string& plist);
        void addSpriteFramesWithFile(const std::string& plist, const std::string& textureFileName);
        void addSpriteFramesWithFile(const std::string&plist, Unity3DTexture *texture);
        void addSpriteFramesWithFileContent(const std::string& plist_content, Unity3DTexture *texture);
        void addSpriteFrame(SpriteFrame *frame, const std::string& frameName);

        bool isSpriteFramesWithFileLoaded(const std::string& plist) const;

        void removeSpriteFrames();
        void removeUnusedSpriteFrames();
        void removeSpriteFrameByName(const std::string& name);
        void removeSpriteFramesFromFile(const std::string& plist);
        void removeSpriteFramesFromFileContent(const std::string& plist_content);
        void removeSpriteFramesFromTexture(Unity3DTexture* texture);

        SpriteFrame* getSpriteFrameByName(const std::string& name);

    protected:
        SpriteFrameCache(){ init(); }

        void addSpriteFramesWithDictionary(ValueMap& dictionary, Unity3DTexture *texture);
        void removeSpriteFramesFromDictionary(ValueMap& dictionary);

    private:
        HObjectMap<std::string, SpriteFrame*> spriteFrames_;
        ValueMap spriteFramesAliases_;
        std::set<std::string>* loadedFileNames_;
    };
}

#endif // SPRITEFRAME_H
