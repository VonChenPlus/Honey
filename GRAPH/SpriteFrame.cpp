#include <algorithm>
#include "GRAPH/SpriteFrame.h"
#include "IO/FileUtils.h"
#include "UTILS/STRING/StringUtils.h"
#include "GRAPH/Director.h"
#include "GRAPH/UNITY3D/GLTexture.h"

namespace GRAPH
{
    SpriteFrame* SpriteFrame::create(const std::string& filename, const MATH::Rectf& rect) {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTextureFilename(filename, rect);
        spriteFrame->autorelease();
        return spriteFrame;
    }

    SpriteFrame* SpriteFrame::createWithTexture(GLTexture *texture, const MATH::Rectf& rect) {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTexture(texture, rect);
        spriteFrame->autorelease();
        return spriteFrame;
    }

    SpriteFrame* SpriteFrame::createWithTexture(GLTexture* texture, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize) {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTexture(texture, rect, rotated, offset, originalSize);
        spriteFrame->autorelease();
        return spriteFrame;
    }

    SpriteFrame* SpriteFrame::create(const std::string& filename, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize) {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTextureFilename(filename, rect, rotated, offset, originalSize);
        spriteFrame->autorelease();
        return spriteFrame;
    }

    SpriteFrame::SpriteFrame()
        : rotated_(false)
        , texture_(nullptr) {

    }

    bool SpriteFrame::initWithTexture(GLTexture* texture, const MATH::Rectf& rect) {
        return initWithTexture(texture, rect, false, MATH::Vec2fZERO, rect.size);
    }

    bool SpriteFrame::initWithTextureFilename(const std::string& filename, const MATH::Rectf& rect) {
        return initWithTextureFilename(filename, rect, false, MATH::Vec2fZERO, rect.size);
    }

    bool SpriteFrame::initWithTexture(GLTexture* texture, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize) {
        texture_ = texture;
        if (texture) {
            texture->retain();
        }

        rect_ = rect;
        offset_ =  offset ;
        originalSize_ = originalSize ;
        rotated_ = rotated;

        return true;
    }

    bool SpriteFrame::initWithTextureFilename(const std::string& filename, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize) {
        texture_ = nullptr;
        textureFilename_ = filename;
        rect_ = rect;
        offset_ = offset ;
        originalSize_ = originalSize ;
        rotated_ = rotated;
        return true;
    }

    SpriteFrame::~SpriteFrame() {
        SAFE_RELEASE(texture_);
    }

    void SpriteFrame::setRect(const MATH::Rectf& rect) {
        rect_ = rect;
    }

    const MATH::Vector2f& SpriteFrame::getOffset() const {
        return offset_;
    }

    void SpriteFrame::setOffset(const MATH::Vector2f& offsets) {
        offset_ = offsets;
    }

    void SpriteFrame::setTexture(GLTexture * texture) {
        if( texture_ != texture ) {
            SAFE_RELEASE(texture_);
            SAFE_RETAIN(texture);
            texture_ = texture;
        }
    }

    GLTexture* SpriteFrame::getTexture() {
        if( texture_ ) {
            return texture_;
        }

        if( textureFilename_.length() > 0 ) {
            return Director::getInstance().getTextureCache()->addImage(textureFilename_.c_str());
        }

        return nullptr;
    }

    SpriteFrameCache& SpriteFrameCache::getInstance() {
        static SpriteFrameCache instance;
        return instance;
    }

    bool SpriteFrameCache::init() {
        spriteFrames_.reserve(20);
        spriteFramesAliases_.reserve(20);
        loadedFileNames_ = new std::set<std::string>();
        return true;
    }

    SpriteFrameCache::~SpriteFrameCache() {
        SAFE_DELETE(loadedFileNames_);
    }

    void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist, GLTexture *texture) {
        if (loadedFileNames_->find(plist) != loadedFileNames_->end()) {
            return;
        }

        std::string fullPath = IO::FileUtils::getInstance().fullPathForFilename(plist);
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(fullPath);

        addSpriteFramesWithDictionary(dict, texture);
        loadedFileNames_->insert(plist);
    }

    void SpriteFrameCache::addSpriteFramesWithFileContent(const std::string& plist_content, GLTexture *texture) {
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromData((const HBYTE *)plist_content.c_str(), static_cast<int>(plist_content.size()));
        addSpriteFramesWithDictionary(dict, texture);
    }

    void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist, const std::string& textureFileName) {
        GLTexture *texture = Director::getInstance().getTextureCache()->addImage(textureFileName);

        if (texture) {
            addSpriteFramesWithFile(plist, texture);
        }
    }

    void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist) {
        std::string fullPath = IO::FileUtils::getInstance().fullPathForFilename(plist);
        if (fullPath.size() == 0) {
            return;
        }

        if (loadedFileNames_->find(plist) == loadedFileNames_->end()) {
            ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(fullPath);
            std::string texturePath("");
            if (dict.find("metadata") != dict.end()) {
                ValueMap& metadataDict = dict["metadata"].asValueMap();
                // try to read  texture file name from meta data
                texturePath = metadataDict["textureFileName"].asString();
            }

            if (!texturePath.empty()) {
                // build texture path relative to plist file
                texturePath = IO::FileUtils::getInstance().fullPathFromRelativeFile(texturePath.c_str(), plist);
            }
            else {
                // build texture path by replacing file extension
                texturePath = plist;
                // remove .xxx
                uint64 startPos = texturePath.find_last_of(".");
                texturePath = texturePath.erase(startPos);
                // append .png
                texturePath = texturePath.append(".png");
            }

            GLTexture *texture = Director::getInstance().getTextureCache()->addImage(texturePath.c_str());
            if (texture) {
                addSpriteFramesWithDictionary(dict, texture);
                loadedFileNames_->insert(plist);
            }
        }
    }

    void SpriteFrameCache::addSpriteFramesWithDictionary(ValueMap& dictionary, GLTexture* texture) {
        ValueMap& framesDict = dictionary["frames"].asValueMap();
        int format = 0;
        // get the format
        if (dictionary.find("metadata") != dictionary.end()) {
            ValueMap& metadataDict = dictionary["metadata"].asValueMap();
            format = metadataDict["format"].asInt();
        }

        auto textureFileName = Director::getInstance().getTextureCache()->getTextureFilePath(texture);
        auto image = new IMAGE::TinyImage();
        image->initWithImageFile(textureFileName);

        for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter) {
            ValueMap& frameDict = iter->second.asValueMap();
            std::string spriteFrameName = iter->first;
            SpriteFrame* spriteFrame = spriteFrames_.at(spriteFrameName);
            if (spriteFrame) {
                continue;
            }

            if (format == 0) {
                float x = frameDict["x"].asFloat();
                float y = frameDict["y"].asFloat();
                float w = frameDict["width"].asFloat();
                float h = frameDict["height"].asFloat();
                float ox = frameDict["offsetX"].asFloat();
                float oy = frameDict["offsetY"].asFloat();
                int ow = frameDict["originalWidth"].asInt();
                int oh = frameDict["originalHeight"].asInt();

                // abs ow/oh
                ow = abs(ow);
                oh = abs(oh);
                // create frame
                spriteFrame = SpriteFrame::createWithTexture(texture,
                    MATH::Rectf(x, y, w, h),
                    false,
                    MATH::Vector2f(ox, oy),
                    MATH::Sizef((float) ow, (float) oh)
                    );
            }
            else if (format == 1 || format == 2) {
                MATH::Rectf frame = UTILS::STRING::RectFromString(frameDict["frame"].asString());
                bool rotated = false;

                // rotation
                if (format == 2) {
                    rotated = frameDict["rotated"].asBool();
                }

                MATH::Vector2f offset = UTILS::STRING::PointFromString(frameDict["offset"].asString());
                MATH::Sizef sourceSize = UTILS::STRING::SizeFromString(frameDict["sourceSize"].asString());

                // create frame
                spriteFrame = SpriteFrame::createWithTexture(texture,
                    frame,
                    rotated,
                    offset,
                    sourceSize
                    );
            }
            else if (format == 3) {
                // get values
                MATH::Sizef spriteSize = UTILS::STRING::SizeFromString(frameDict["spriteSize"].asString());
                MATH::Vector2f spriteOffset = UTILS::STRING::PointFromString(frameDict["spriteOffset"].asString());
                MATH::Sizef spriteSourceSize = UTILS::STRING::SizeFromString(frameDict["spriteSourceSize"].asString());
                MATH::Rectf textureRect = UTILS::STRING::RectFromString(frameDict["textureRect"].asString());
                bool textureRotated = frameDict["textureRotated"].asBool();

                // get aliases
                ValueVector& aliases = frameDict["aliases"].asValueVector();

                for (const auto &value : aliases) {
                    std::string oneAlias = value.asString();
                    spriteFramesAliases_[oneAlias] = HValue(spriteFrameName);
                }

                // create frame
                spriteFrame = SpriteFrame::createWithTexture(texture,
                    MATH::Rectf(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
                    textureRotated,
                    spriteOffset,
                    spriteSourceSize);
            }

            // add sprite frame
            spriteFrames_.insert(spriteFrameName, spriteFrame);
        }
        SAFE_DELETE(image);
    }

    bool SpriteFrameCache::isSpriteFramesWithFileLoaded(const std::string& plist) const {
        bool result = false;

        if (loadedFileNames_->find(plist) != loadedFileNames_->end()) {
            result = true;
        }

        return result;
    }

    void SpriteFrameCache::addSpriteFrame(SpriteFrame* frame, const std::string& frameName) {
        spriteFrames_.insert(frameName, frame);
    }

    void SpriteFrameCache::removeSpriteFrames() {
        spriteFrames_.clear();
        spriteFramesAliases_.clear();
        loadedFileNames_->clear();
    }

    void SpriteFrameCache::removeUnusedSpriteFrames() {
        bool removed = false;
        std::vector<std::string> toRemoveFrames;

        for (auto iter = spriteFrames_.begin(); iter != spriteFrames_.end(); ++iter) {
            SpriteFrame* spriteFrame = iter->second;
            if( spriteFrame->getReferenceCount() == 1 ) {
                toRemoveFrames.push_back(iter->first);
                removed = true;
            }
        }

        spriteFrames_.erase(toRemoveFrames);

        if( removed ) {
            loadedFileNames_->clear();
        }
    }

    void SpriteFrameCache::removeSpriteFrameByName(const std::string& name) {
        if( !(name.size()>0) )
            return;

        std::string key = spriteFramesAliases_[name].asString();

        if (!key.empty()) {
            spriteFrames_.erase(key);
            spriteFramesAliases_.erase(key);
        }
        else {
            spriteFrames_.erase(name);
        }

        loadedFileNames_->clear();
    }

    void SpriteFrameCache::removeSpriteFramesFromFile(const std::string& plist) {
        std::string fullPath = IO::FileUtils::getInstance().fullPathForFilename(plist);
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(fullPath);
        if (dict.empty()) {
            return;
        }
        removeSpriteFramesFromDictionary(dict);

        // remove it from the cache
        std::set<std::string>::iterator ret = loadedFileNames_->find(plist);
        if (ret != loadedFileNames_->end()) {
            loadedFileNames_->erase(ret);
        }
    }

    void SpriteFrameCache::removeSpriteFramesFromFileContent(const std::string& plist_content) {
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromData((const HBYTE *)plist_content.data(), static_cast<int>(plist_content.size()));
        if (dict.empty()) {
            return;
        }
        removeSpriteFramesFromDictionary(dict);
    }

    void SpriteFrameCache::removeSpriteFramesFromDictionary(ValueMap& dictionary) {
        ValueMap framesDict = dictionary["frames"].asValueMap();
        std::vector<std::string> keysToRemove;

        for (auto iter = framesDict.cbegin(); iter != framesDict.cend(); ++iter) {
            if (spriteFrames_.at(iter->first)) {
                keysToRemove.push_back(iter->first);
            }
        }

        spriteFrames_.erase(keysToRemove);
    }

    void SpriteFrameCache::removeSpriteFramesFromTexture(GLTexture* texture) {
        std::vector<std::string> keysToRemove;

        for (auto iter = spriteFrames_.cbegin(); iter != spriteFrames_.cend(); ++iter) {
            std::string key = iter->first;
            SpriteFrame* frame = spriteFrames_.at(key);
            if (frame && (frame->getTexture() == texture)) {
                keysToRemove.push_back(key);
            }
        }

        spriteFrames_.erase(keysToRemove);
    }

    SpriteFrame* SpriteFrameCache::getSpriteFrameByName(const std::string& name) {
        SpriteFrame* frame = spriteFrames_.at(name);
        if (!frame) {
            std::string key = spriteFramesAliases_[name].asString();
            if (!key.empty()) {
                frame = spriteFrames_.at(key);
            }
        }
        return frame;
    }
}
