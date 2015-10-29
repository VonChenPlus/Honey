#include <algorithm>
#include "GRAPH/Sprite.h"
#include "GRAPH/SpriteFrame.h"
#include "GRAPH/Director.h"
#include "GRAPH/UNITY3D/GLTexture.h"
#include "GRAPH/UNITY3D/GLTextureAtlas.h"
#include "GRAPH/UNITY3D/Unity3DGLShader.h"
#include "GRAPH/UNITY3D/GLShaderState.h"
#include "GRAPH/UNITY3D/Renderer.h"

namespace GRAPH
{
    PolygonInfo::PolygonInfo()
        : isVertsOwner(true)
        , rect(MATH::RectfZERO) {
        triangles.verts = nullptr;
        triangles.indices = nullptr;
        triangles.vertCount = 0;
        triangles.indexCount = 0;
    }

    PolygonInfo::PolygonInfo(const PolygonInfo& other) :
        isVertsOwner(true) {
        isVertsOwner = other.isVertsOwner;
        rect = other.rect;
        triangles.verts = new V3F_C4B_T2F[other.triangles.vertCount];
        triangles.indices = new unsigned short[other.triangles.indexCount];
        triangles.vertCount = other.triangles.vertCount;
        triangles.indexCount = other.triangles.indexCount;
        memcpy(triangles.verts, other.triangles.verts, other.triangles.vertCount*sizeof(V3F_C4B_T2F));
        memcpy(triangles.indices, other.triangles.indices, other.triangles.indexCount*sizeof(unsigned short));
    }

    PolygonInfo& PolygonInfo::operator= (const PolygonInfo& other) {
        if (this != &other) {
            releaseVertsAndIndices();
            isVertsOwner = other.isVertsOwner;
            rect = other.rect;
            triangles.verts = new V3F_C4B_T2F[other.triangles.vertCount];
            triangles.indices = new unsigned short[other.triangles.indexCount];
            triangles.vertCount = other.triangles.vertCount;
            triangles.indexCount = other.triangles.indexCount;
            memcpy(triangles.verts, other.triangles.verts, other.triangles.vertCount*sizeof(V3F_C4B_T2F));
            memcpy(triangles.indices, other.triangles.indices, other.triangles.indexCount*sizeof(unsigned short));
        }

        return *this;
    }

    PolygonInfo::~PolygonInfo() {
        releaseVertsAndIndices();
    }

    void PolygonInfo::setQuad(V3F_C4B_T2F_Quad *quad){
        releaseVertsAndIndices();
        isVertsOwner = false;
        static unsigned short quadIndices [] = { 0, 1, 2, 3, 2, 1 };
        triangles.indices = quadIndices;
        triangles.vertCount = 4;
        triangles.indexCount = 6;
        triangles.verts = (V3F_C4B_T2F*) quad;
    }

    void PolygonInfo::releaseVertsAndIndices() {
        if (isVertsOwner) {
            if (nullptr != triangles.verts) {
                SAFE_DELETE_ARRAY(triangles.verts);
            }

            if (nullptr != triangles.indices) {
                SAFE_DELETE_ARRAY(triangles.indices);
            }
        }
    }

    const unsigned int PolygonInfo::getVertCount() const {
        return (unsigned int) triangles.vertCount;
    }

    const unsigned int PolygonInfo::getTriaglesCount() const {
        return (unsigned int) triangles.indexCount / 3;
    }

    const float PolygonInfo::getArea() const {
        float area = 0;
        V3F_C4B_T2F *verts = triangles.verts;
        unsigned short *indices = triangles.indices;
        for (int i = 0; i < triangles.indexCount; i += 3) {
            auto A = verts[indices[i]].vertices;
            auto B = verts[indices[i + 1]].vertices;
            auto C = verts[indices[i + 2]].vertices;
            area += (A.x*(B.y - C.y) + B.x*(C.y - A.y) + C.x*(A.y - B.y)) / 2;
        }
        return area;
    }

    Sprite* Sprite::createWithTexture(GLTexture *texture) {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithTexture(texture)) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::createWithTexture(GLTexture *texture, const MATH::Rectf& rect, bool rotated) {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithTexture(texture, rect, rotated)) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::createWithSpriteFrame(SpriteFrame *spriteFrame) {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame)) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::createWithSpriteFrameName(const std::string& spriteFrameName) {
        SpriteFrame *frame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);
        return createWithSpriteFrame(frame);
    }

    Sprite* Sprite::create(const std::string& filename) {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithFile(filename)) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::create(const PolygonInfo& info) {
        Sprite *sprite = new (std::nothrow) Sprite();
        if(sprite && sprite->initWithPolygon(info)) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::create(const std::string& filename, const MATH::Rectf& rect) {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithFile(filename, rect)) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::create() {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->init()) {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    bool Sprite::init(void) {
        return initWithTexture(nullptr, MATH::RectfZERO );
    }

    bool Sprite::initWithTexture(GLTexture *texture) {
        MATH::Rectf rect = MATH::RectfZERO;
        rect.size = texture->getContentSize();

        return initWithTexture(texture, rect);
    }

    bool Sprite::initWithTexture(GLTexture *texture, const MATH::Rectf& rect) {
        return initWithTexture(texture, rect, false);
    }

    bool Sprite::initWithSpriteFrameName(const std::string& spriteFrameName) {
        SpriteFrame *frame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);
        return initWithSpriteFrame(frame);
    }

    bool Sprite::initWithSpriteFrame(SpriteFrame *spriteFrame) {
        bool bRet = initWithTexture(spriteFrame->getTexture(), spriteFrame->getRect());
        setSpriteFrame(spriteFrame);

        return bRet;
    }

    bool Sprite::initWithFile(const std::string& filename) {
        GLTexture *texture = TextureCache::getInstance().addImage(filename);
        if (texture) {
            MATH::Rectf rect = MATH::RectfZERO;
            rect.size = texture->getContentSize();
            return initWithTexture(texture, rect);
        }

        return false;
    }

    bool Sprite::initWithFile(const std::string &filename, const MATH::Rectf& rect) {
        GLTexture *texture = TextureCache::getInstance().addImage(filename);
        if (texture) {
            return initWithTexture(texture, rect);
        }

        return false;
    }

    bool Sprite::initWithPolygon(const PolygonInfo &info) {
        polyInfo_ = info;
        setContentSize(polyInfo_.rect.size);
        return true;
    }

    bool Sprite::initWithTexture(GLTexture *texture, const MATH::Rectf& rect, bool rotated) {
        bool result;
        if (Node::init()) {
            batchNode_ = nullptr;
            recursiveDirty_ = false;
            setDirty(false);
            opacityModifyRGB_ = true;
            blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
            flippedX_ = flippedY_ = false;
            // default transform anchor: center
            setAnchorPoint(MATH::Vector2f(0.5f, 0.5f));
            // zwoptex default values
            offsetPosition_.setZero();
            // clean the Quad
            memset(&quad_, 0, sizeof(quad_));
            // Atlas: Color
            quad_.bl.colors = Color4B::WHITE;
            quad_.br.colors = Color4B::WHITE;
            quad_.tl.colors = Color4B::WHITE;
            quad_.tr.colors = Color4B::WHITE;
            // shader state
            setGLShaderState(GLShaderState::getOrCreateWithGLShaderName(Unity3DGLShaderSet::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
            // update texture (calls updateBlendFunc)
            setTexture(texture);
            setTextureRect(rect, rotated, rect.size);
            polyInfo_.setQuad(&quad_);
            // by default use "Self Render".
            // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
            setBatchNode(nullptr);
            result = true;
        }
        else {
            result = false;
        }
        recursiveDirty_ = true;
        setDirty(true);
        return result;
    }

    Sprite::Sprite(void)
        : shouldBeHidden_(false)
        , texture_(nullptr)
        , spriteFrame_(nullptr) {
    }

    Sprite::~Sprite(void) {
        SAFE_RELEASE(spriteFrame_);
        SAFE_RELEASE(texture_);
    }

    static unsigned char _2x2_white_image[] = {
        // RGBA8888
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    #define _2x2_WHITE_IMAGE_KEY  "/_2x2_white_image"

    void Sprite::setTexture(const std::string &filename) {
        GLTexture *texture = TextureCache::getInstance().addImage(filename);
        setTexture(texture);

        MATH::Rectf rect = MATH::RectfZERO;
        if (texture)
            rect.size = texture->getContentSize();
        setTextureRect(rect);
    }

    void Sprite::setTexture(GLTexture *texture) {
        if (texture == nullptr) {
            texture = TextureCache::getInstance().getTextureForKey(_2x2_WHITE_IMAGE_KEY);

            if (texture == nullptr) {
                IMAGE::ImageObject* image = new (std::nothrow) IMAGE::ImageObject();
                image->initWithRawData(_2x2_white_image, sizeof(_2x2_white_image), 2, 2, 8);
                texture = TextureCache::getInstance().addImage(image, _2x2_WHITE_IMAGE_KEY);
                SAFE_RELEASE(image);
            }
        }

        if (texture_ != texture) {
            SAFE_RETAIN(texture);
            SAFE_RELEASE(texture_);
            texture_ = texture;
            updateBlendFunc();
        }
    }

    GLTexture* Sprite::getTexture() const {
        return texture_;
    }

    void Sprite::setTextureRect(const MATH::Rectf& rect) {
        setTextureRect(rect, false, rect.size);
    }

    void Sprite::setTextureRect(const MATH::Rectf& rect, bool rotated, const MATH::Sizef& untrimmedSize) {
        rectRotated_ = rotated;

        setContentSize(untrimmedSize);
        setVertexRect(rect);
        setTextureCoords(rect);

        float relativeOffsetX = unflippedOffsetPositionFromCenter_.x;
        float relativeOffsetY = unflippedOffsetPositionFromCenter_.y;

        if (flippedX_) {
            relativeOffsetX = -relativeOffsetX;
        }
        if (flippedY_) {
            relativeOffsetY = -relativeOffsetY;
        }

        offsetPosition_.x = relativeOffsetX + (contentSize_.width - rect_.size.width) / 2;
        offsetPosition_.y = relativeOffsetY + (contentSize_.height - rect_.size.height) / 2;

        if (batchNode_) {
            setDirty(true);
        }
        else {
            float x1 = 0.0f + offsetPosition_.x;
            float y1 = 0.0f + offsetPosition_.y;
            float x2 = x1 + rect_.size.width;
            float y2 = y1 + rect_.size.height;

            quad_.bl.vertices.set(x1, y1, 0.0f);
            quad_.br.vertices.set(x2, y1, 0.0f);
            quad_.tl.vertices.set(x1, y2, 0.0f);
            quad_.tr.vertices.set(x2, y2, 0.0f);
        }
    }

    void Sprite::setVertexRect(const MATH::Rectf& rect) {
        rect_ = rect;
    }

    void Sprite::setSpriteFrame(const std::string &spriteFrameName) {
        SpriteFrame *spriteFrame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);
        setSpriteFrame(spriteFrame);
    }

    void Sprite::setSpriteFrame(SpriteFrame *spriteFrame) {
        if (spriteFrame_ != spriteFrame) {
            SAFE_RELEASE(spriteFrame_);
            spriteFrame_ = spriteFrame;
            spriteFrame->retain();
        }

        unflippedOffsetPositionFromCenter_ = spriteFrame->getOffset();

        GLTexture *texture = spriteFrame->getTexture();
        if (texture != texture_) {
            setTexture(texture);
        }

        rectRotated_ = spriteFrame->isRotated();
        setTextureRect(spriteFrame->getRect(), rectRotated_, spriteFrame->getOriginalSize());
    }

    bool Sprite::isFrameDisplayed(SpriteFrame *frame) const {
        MATH::Rectf rect = frame->getRect();

        return (rect.equals(rect_) &&
                frame->getTexture()->getName() == texture_->getName() &&
                frame->getOffset().equals(unflippedOffsetPositionFromCenter_));
    }

    SpriteFrame* Sprite::getSpriteFrame() const {
        if(nullptr != this->spriteFrame_) {
            return this->spriteFrame_;
        }

        return SpriteFrame::createWithTexture(texture_,
                                               rect_,
                                               rectRotated_,
                                               unflippedOffsetPositionFromCenter_,
                                               contentSize_);
    }

    void Sprite::setTextureCoords(MATH::Rectf rect) {
        GLTexture *tex = batchNode_ ? textureAtlas_->getTexture() : texture_;
        if (! tex) {
            return;
        }

        float atlasWidth = (float)tex->getPixelsWidth();
        float atlasHeight = (float)tex->getPixelsHight();

        float left, right, top, bottom;

        if (rectRotated_) {
            left    = rect.origin.x/atlasWidth;
            right   = (rect.origin.x+rect.size.height) / atlasWidth;
            top     = rect.origin.y/atlasHeight;
            bottom  = (rect.origin.y+rect.size.width) / atlasHeight;

            if (flippedX_) {
                std::swap(top, bottom);
            }

            if (flippedY_) {
                std::swap(left, right);
            }

            quad_.bl.texCoords.u = left;
            quad_.bl.texCoords.v = top;
            quad_.br.texCoords.u = left;
            quad_.br.texCoords.v = bottom;
            quad_.tl.texCoords.u = right;
            quad_.tl.texCoords.v = top;
            quad_.tr.texCoords.u = right;
            quad_.tr.texCoords.v = bottom;
        }
        else {
            left    = rect.origin.x/atlasWidth;
            right    = (rect.origin.x + rect.size.width) / atlasWidth;
            top        = rect.origin.y/atlasHeight;
            bottom    = (rect.origin.y + rect.size.height) / atlasHeight;

            if(flippedX_) {
                std::swap(left, right);
            }

            if(flippedY_) {
                std::swap(top, bottom);
            }

            quad_.bl.texCoords.u = left;
            quad_.bl.texCoords.v = bottom;
            quad_.br.texCoords.u = right;
            quad_.br.texCoords.v = bottom;
            quad_.tl.texCoords.u = left;
            quad_.tl.texCoords.v = top;
            quad_.tr.texCoords.u = right;
            quad_.tr.texCoords.v = top;
        }
    }

    void Sprite::updateTransform(void) {
        if( isDirty() ) {
            if( !visible_ || ( parent_ && parent_ != batchNode_ && static_cast<Sprite*>(parent_)->shouldBeHidden_) ) {
                quad_.br.vertices.setZero();
                quad_.tl.vertices.setZero();
                quad_.tr.vertices.setZero();
                quad_.bl.vertices.setZero();
                shouldBeHidden_ = true;
            }
            else {
                shouldBeHidden_ = false;

                if( ! parent_ || parent_ == batchNode_ ) {
                    transformToBatch_ = getNodeToParentTransform();
                }
                else {
                    const MATH::Matrix4 &nodeToParent = getNodeToParentTransform();
                    MATH::Matrix4 &parentTransform = static_cast<Sprite*>(parent_)->transformToBatch_;
                    transformToBatch_ = parentTransform * nodeToParent;
                }

                MATH::Sizef &size = rect_.size;

                float x1 = offsetPosition_.x;
                float y1 = offsetPosition_.y;

                float x2 = x1 + size.width;
                float y2 = y1 + size.height;
                float x = transformToBatch_.m[12];
                float y = transformToBatch_.m[13];

                float cr = transformToBatch_.m[0];
                float sr = transformToBatch_.m[1];
                float cr2 = transformToBatch_.m[5];
                float sr2 = -transformToBatch_.m[4];
                float ax = x1 * cr - y1 * sr2 + x;
                float ay = x1 * sr + y1 * cr2 + y;

                float bx = x2 * cr - y1 * sr2 + x;
                float by = x2 * sr + y1 * cr2 + y;

                float cx = x2 * cr - y2 * sr2 + x;
                float cy = x2 * sr + y2 * cr2 + y;

                float dx = x1 * cr - y2 * sr2 + x;
                float dy = x1 * sr + y2 * cr2 + y;

                quad_.bl.vertices.set(ax, ay, positionZ_);
                quad_.br.vertices.set(bx, by, positionZ_);
                quad_.tl.vertices.set(dx, dy, positionZ_);
                quad_.tr.vertices.set(cx, cy, positionZ_);
            }

            if (textureAtlas_) {
                textureAtlas_->updateQuad(&quad_, atlasIndex_);
            }

            recursiveDirty_ = false;
            setDirty(false);
        }

        Node::updateTransform();
    }

    void Sprite::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) {
        trianglesCommand_.init(globalZOrder_, texture_->getName(), getGLShaderState(), blendFunc_, polyInfo_.triangles, transform, flags);
        renderer->addCommand(&trianglesCommand_);
    }

    void Sprite::addChild(Node *child, int zOrder, int tag) {
        if (batchNode_) {
            Sprite* childSprite = dynamic_cast<Sprite*>(child);
            batchNode_->appendChild(childSprite);

            if (!reorderChildDirty_) {
                setReorderChildDirtyRecursively();
            }
        }
        Node::addChild(child, zOrder, tag);
    }

    void Sprite::addChild(Node *child, int zOrder, const std::string &name) {
        if (batchNode_) {
            Sprite* childSprite = dynamic_cast<Sprite*>(child);
            batchNode_->appendChild(childSprite);

            if (!reorderChildDirty_) {
                setReorderChildDirtyRecursively();
            }
        }
        Node::addChild(child, zOrder, name);
    }

    void Sprite::reorderChild(Node *child, int zOrder) {
        if( batchNode_ && ! reorderChildDirty_) {
            setReorderChildDirtyRecursively();
            batchNode_->reorderBatch(true);
        }

        Node::reorderChild(child, zOrder);
    }

    void Sprite::removeChild(Node *child, bool cleanup) {
        if (batchNode_) {
            batchNode_->removeSpriteFromAtlas((Sprite*)(child));
        }

        Node::removeChild(child, cleanup);
    }

    void Sprite::removeAllChildrenWithCleanup(bool cleanup) {
        if (batchNode_) {
            for(const auto &child : children_) {
                Sprite* sprite = dynamic_cast<Sprite*>(child);
                if (sprite) {
                    batchNode_->removeSpriteFromAtlas(sprite);
                }
            }
        }

        Node::removeAllChildrenWithCleanup(cleanup);
    }

    void Sprite::sortAllChildren() {
        if (reorderChildDirty_) {
            std::sort(std::begin(children_), std::end(children_), NodeComparisonLess);

            if ( batchNode_) {
                for(const auto &child : children_)
                    child->sortAllChildren();
            }

            reorderChildDirty_ = false;
        }
    }

    void Sprite::setReorderChildDirtyRecursively(void) {
        if ( ! reorderChildDirty_ ) {
            reorderChildDirty_ = true;
            Node* node = static_cast<Node*>(parent_);
            while (node && node != batchNode_) {
                static_cast<Sprite*>(node)->setReorderChildDirtyRecursively();
                node=node->getParent();
            }
        }
    }

    void Sprite::setDirtyRecursively(bool bValue) {
        recursiveDirty_ = bValue;
        setDirty(bValue);

        for(const auto &child: children_) {
            Sprite* sp = dynamic_cast<Sprite*>(child);
            if (sp) {
                sp->setDirtyRecursively(true);
            }
        }
    }

    #define SET_DIRTY_RECURSIVELY() {                       \
                        if (! recursiveDirty_) {            \
                            recursiveDirty_ = true;         \
                            setDirty(true);                 \
                            if (!children_.empty())         \
                                setDirtyRecursively(true);  \
                            }                               \
                        }

    void Sprite::setPosition(const MATH::Vector2f& pos) {
        Node::setPosition(pos);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setPosition(float x, float y) {
        Node::setPosition(x, y);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setRotation(float rotation) {
        Node::setRotation(rotation);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setRotationSkewX(float fRotationX) {
        Node::setRotationSkewX(fRotationX);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setRotationSkewY(float fRotationY) {
        Node::setRotationSkewY(fRotationY);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setSkewX(float sx) {
        Node::setSkewX(sx);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setSkewY(float sy) {
        Node::setSkewY(sy);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScaleX(float scaleX) {
        Node::setScaleX(scaleX);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScaleY(float scaleY) {
        Node::setScaleY(scaleY);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScale(float fScale) {
        Node::setScale(fScale);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScale(float scaleX, float scaleY) {
        Node::setScale(scaleX, scaleY);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setPositionZ(float fVertexZ) {
        Node::setPositionZ(fVertexZ);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setAnchorPoint(const MATH::Vector2f& anchor) {
        Node::setAnchorPoint(anchor);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setVisible(bool bVisible) {
        Node::setVisible(bVisible);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setFlippedX(bool flippedX) {
        if (flippedX_ != flippedX) {
            flippedX_ = flippedX;
            setTextureRect(rect_, rectRotated_, contentSize_);
        }
    }

    bool Sprite::isFlippedX(void) const {
        return flippedX_;
    }

    void Sprite::setFlippedY(bool flippedY) {
        if (flippedY_ != flippedY) {
            flippedY_ = flippedY;
            setTextureRect(rect_, rectRotated_, contentSize_);
        }
    }

    bool Sprite::isFlippedY(void) const {
        return flippedY_;
    }

    void Sprite::updateColor(void) {
        Color4B color4( displayedColor_.red, displayedColor_.green, displayedColor_.blue, displayedOpacity_ );
        if (opacityModifyRGB_) {
            color4.red *= displayedOpacity_/255.0f;
            color4.green *= displayedOpacity_/255.0f;
            color4.blue *= displayedOpacity_/255.0f;
        }

        quad_.bl.colors = color4;
        quad_.br.colors = color4;
        quad_.tl.colors = color4;
        quad_.tr.colors = color4;

        if (batchNode_) {
            if (atlasIndex_ != INDEX_NOT_INITIALIZED) {
                textureAtlas_->updateQuad(&quad_, atlasIndex_);
            }
            else {
                setDirty(true);
            }
        }
    }

    void Sprite::setOpacityModifyRGB(bool modify) {
        if (opacityModifyRGB_ != modify) {
            opacityModifyRGB_ = modify;
            updateColor();
        }
    }

    bool Sprite::isOpacityModifyRGB(void) const {
        return opacityModifyRGB_;
    }

    SpriteBatchNode* Sprite::getBatchNode() const {
        return batchNode_;
    }

    void Sprite::setBatchNode(SpriteBatchNode *spriteBatchNode) {
        batchNode_ = spriteBatchNode;

        if( ! batchNode_ ) {
            atlasIndex_ = INDEX_NOT_INITIALIZED;
            setTextureAtlas(nullptr);
            recursiveDirty_ = false;
            setDirty(false);

            float x1 = offsetPosition_.x;
            float y1 = offsetPosition_.y;
            float x2 = x1 + rect_.size.width;
            float y2 = y1 + rect_.size.height;
            quad_.bl.vertices.set( x1, y1, 0 );
            quad_.br.vertices.set(x2, y1, 0);
            quad_.tl.vertices.set(x1, y2, 0);
            quad_.tr.vertices.set(x2, y2, 0);

        } else {
            transformToBatch_ = MATH::Matrix4::IDENTITY;
            setTextureAtlas(batchNode_->getTextureAtlas()); // weak ref
        }
    }

    void Sprite::updateBlendFunc(void) {
        if (! texture_ || ! texture_->hasPremultipliedAlpha()) {
            blendFunc_ = BlendFunc::ALPHA_NON_PREMULTIPLIED;
            setOpacityModifyRGB(false);
        }
        else {
            blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
            setOpacityModifyRGB(true);
        }
    }

    PolygonInfo Sprite::getPolygonInfo() const {
        return polyInfo_;
    }

    void Sprite::setPolygonInfo(const PolygonInfo& info) {
        polyInfo_ = info;
    }

    SpriteBatchNode* SpriteBatchNode::createWithTexture(GLTexture* tex, uint64 capacity/* = DEFAULT_CAPACITY*/) {
        SpriteBatchNode *batchNode = new (std::nothrow) SpriteBatchNode();
        batchNode->initWithTexture(tex, capacity);
        batchNode->autorelease();

        return batchNode;
    }

    SpriteBatchNode* SpriteBatchNode::create(const std::string& fileImage, uint64 capacity/* = DEFAULT_CAPACITY*/) {
        SpriteBatchNode *batchNode = new (std::nothrow) SpriteBatchNode();
        batchNode->initWithFile(fileImage, capacity);
        batchNode->autorelease();
        return batchNode;
    }

    bool SpriteBatchNode::initWithTexture(GLTexture *tex, uint64 capacity/* = DEFAULT_CAPACITY*/) {
        blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
        if (!tex->hasPremultipliedAlpha()) {
            blendFunc_ = BlendFunc::ALPHA_NON_PREMULTIPLIED;
        }
        textureAtlas_ = new (std::nothrow) GLTextureAtlas();

        if (capacity == 0) {
            capacity = DEFAULT_CAPACITY;
        }

        textureAtlas_->initWithTexture(tex, capacity);

        updateBlendFunc();

        children_.reserve(capacity);

        descendants_.reserve(capacity);

        setGLShaderState(GLShaderState::getOrCreateWithGLShaderName(Unity3DGLShaderSet::SHADER_NAME_POSITION_TEXTURE_COLOR));
        return true;
    }

    bool SpriteBatchNode::init() {
        GLTexture * texture = new (std::nothrow) GLTexture();
        texture->autorelease();
        return this->initWithTexture(texture, 0);
    }

    bool SpriteBatchNode::initWithFile(const std::string& fileImage, uint64 capacity/* = DEFAULT_CAPACITY*/) {
        GLTexture *texture2D = TextureCache::getInstance().addImage(fileImage);
        return initWithTexture(texture2D, capacity);
    }

    SpriteBatchNode::SpriteBatchNode()
        : textureAtlas_(nullptr) {

    }

    SpriteBatchNode::~SpriteBatchNode() {
        SAFE_RELEASE(textureAtlas_);
    }

    void SpriteBatchNode::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) {
        if (!visible_) {
            return;
        }

        sortAllChildren();

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        Director::getInstance().pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        Director::getInstance().loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, modelViewTransform_);

        draw(renderer, modelViewTransform_, flags);

        Director::getInstance().popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    void SpriteBatchNode::addChild(Node *child, int zOrder, int tag) {
        Sprite *sprite = static_cast<Sprite*>(child);

        Node::addChild(child, zOrder, tag);

        appendChild(sprite);
    }

    void SpriteBatchNode::addChild(Node * child, int zOrder, const std::string &name) {
        Sprite *sprite = static_cast<Sprite*>(child);

        Node::addChild(child, zOrder, name);

        appendChild(sprite);
    }

    void SpriteBatchNode::reorderChild(Node *child, int zOrder) {
        if (zOrder == child->getLocalZOrder()) {
            return;
        }

        Node::reorderChild(child, zOrder);
    }

    void SpriteBatchNode::removeChild(Node *child, bool cleanup) {
        Sprite *sprite = static_cast<Sprite*>(child);

        if (sprite == nullptr) {
            return;
        }

        removeSpriteFromAtlas(sprite);

        Node::removeChild(sprite, cleanup);
    }

    void SpriteBatchNode::setTextureAtlas(GLTextureAtlas* textureAtlas) {
        if (textureAtlas != textureAtlas_) {
            SAFE_RETAIN(textureAtlas);
            SAFE_RELEASE(textureAtlas_);
            textureAtlas_ = textureAtlas;
        }
    }

    void SpriteBatchNode::removeChildAtIndex(uint64 index, bool doCleanup) {
        removeChild(children_.at(index), doCleanup);
    }

    void SpriteBatchNode::removeAllChildrenWithCleanup(bool doCleanup) {
        for (const auto &sprite : descendants_) {
            sprite->setBatchNode(nullptr);
        }

        Node::removeAllChildrenWithCleanup(doCleanup);

        descendants_.clear();
        textureAtlas_->removeAllQuads();
    }

    void SpriteBatchNode::sortAllChildren() {
        if (reorderChildDirty_) {
            std::sort(std::begin(children_), std::end(children_), NodeComparisonLess);

            if (!children_.empty()) {
                for (const auto &child : children_) {
                    child->sortAllChildren();
                }

                uint64 index = 0;
                for (const auto &child : children_) {
                    Sprite* sp = static_cast<Sprite*>(child);
                    updateAtlasIndex(sp, &index);
                }
            }

            reorderChildDirty_ = false;
        }
    }

    void SpriteBatchNode::updateAtlasIndex(Sprite* sprite, uint64* curIndex) {
        auto& array = sprite->getChildren();
        auto count = array.size();

        uint64 oldIndex = 0;

        if (count == 0) {
            oldIndex = sprite->getAtlasIndex();
            sprite->setAtlasIndex(*curIndex);
            if (oldIndex != *curIndex){
                swap(oldIndex, *curIndex);
            }
            (*curIndex)++;
        }
        else {
            bool needNewIndex = true;

            if (array.at(0)->getLocalZOrder() >= 0) {
                oldIndex = sprite->getAtlasIndex();
                sprite->setAtlasIndex(*curIndex);
                if (oldIndex != *curIndex) {
                    swap(oldIndex, *curIndex);
                }
                (*curIndex)++;

                needNewIndex = false;
            }

            for (const auto &child : array) {
                Sprite* sp = static_cast<Sprite*>(child);
                if (needNewIndex && sp->getLocalZOrder() >= 0) {
                    oldIndex = sprite->getAtlasIndex();
                    sprite->setAtlasIndex(*curIndex);
                    if (oldIndex != *curIndex) {
                        this->swap(oldIndex, *curIndex);
                    }
                    (*curIndex)++;
                    needNewIndex = false;
                }

                updateAtlasIndex(sp, curIndex);
            }

            if (needNewIndex) {
                oldIndex = sprite->getAtlasIndex();
                sprite->setAtlasIndex(*curIndex);
                if (oldIndex != *curIndex) {
                    swap(oldIndex, *curIndex);
                }
                (*curIndex)++;
            }
        }
    }

    void SpriteBatchNode::swap(uint64 oldIndex, uint64 newIndex) {
        V3F_C4B_T2F_Quad* quads = textureAtlas_->getQuads();
        std::swap(quads[oldIndex], quads[newIndex]);

        auto oldIt = std::next(descendants_.begin(), oldIndex);
        auto newIt = std::next(descendants_.begin(), newIndex);

        (*newIt)->setAtlasIndex(oldIndex);

        std::swap(*oldIt, *newIt);
    }

    void SpriteBatchNode::reorderBatch(bool reorder) {
        reorderChildDirty_ = reorder;
    }

    void SpriteBatchNode::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) {
        if (textureAtlas_->getTotalQuads() == 0) {
            return;
        }

        for (const auto &child : children_) {
            child->updateTransform();
        }

        batchCommand_.init(globalZOrder_, getU3DShader(), blendFunc_, textureAtlas_, transform, flags);
        renderer->addCommand(&batchCommand_);
    }

    void SpriteBatchNode::increaseAtlasCapacity() {
        uint64 quantity = (textureAtlas_->getCapacity() + 1) * 4 / 3;

        if (!textureAtlas_->resizeCapacity(quantity)) {
        }
    }

    uint64 SpriteBatchNode::rebuildIndexInOrder(Sprite *parent, uint64 index) {
        auto& children = parent->getChildren();
        for (const auto &child : children) {
            Sprite* sp = static_cast<Sprite*>(child);
            if (sp && (sp->getLocalZOrder() < 0)) {
                index = rebuildIndexInOrder(sp, index);
            }
        }

        if (parent != static_cast<HObject*>(this)) {
            parent->setAtlasIndex(index);
            index++;
        }

        for (const auto &child : children) {
            Sprite* sp = static_cast<Sprite*>(child);
            if (sp && (sp->getLocalZOrder() >= 0)) {
                index = rebuildIndexInOrder(sp, index);
            }
        }

        return index;
    }

    uint64 SpriteBatchNode::highestAtlasIndexInChild(Sprite *sprite) {
        auto& children = sprite->getChildren();

        if (children.size() == 0) {
            return sprite->getAtlasIndex();
        }
        else {
            return highestAtlasIndexInChild(static_cast<Sprite*>(children.back()));
        }
    }

    uint64 SpriteBatchNode::lowestAtlasIndexInChild(Sprite *sprite) {
        auto& children = sprite->getChildren();

        if (children.size() == 0) {
            return sprite->getAtlasIndex();
        }
        else {
            return lowestAtlasIndexInChild(static_cast<Sprite*>(children.at(0)));
        }
    }

    uint64 SpriteBatchNode::atlasIndexForChild(Sprite *sprite, int nZ) {
        auto& siblings = sprite->getParent()->getChildren();
        auto childIndex = siblings.getIndex(sprite);

        bool ignoreParent = (SpriteBatchNode*) (sprite->getParent()) == this;
        Sprite *prev = nullptr;
        if (childIndex > 0 && childIndex != -1) {
            prev = static_cast<Sprite*>(siblings.at(childIndex - 1));
        }

        if (ignoreParent) {
            if (childIndex == 0) {
                return 0;
            }

            return highestAtlasIndexInChild(prev) + 1;
        }

        if (childIndex == 0) {
            Sprite *p = static_cast<Sprite*>(sprite->getParent());
            if (nZ < 0) {
                return p->getAtlasIndex();
            }
            else {
                return p->getAtlasIndex() + 1;
            }
        }
        else {
            if ((prev->getLocalZOrder() < 0 && nZ < 0) || (prev->getLocalZOrder() >= 0 && nZ >= 0)) {
                return highestAtlasIndexInChild(prev) + 1;
            }

            Sprite *p = static_cast<Sprite*>(sprite->getParent());
            return p->getAtlasIndex() + 1;
        }

        return 0;
    }

    void SpriteBatchNode::appendChild(Sprite* sprite) {
        reorderChildDirty_ = true;
        sprite->setBatchNode(this);
        sprite->setDirty(true);

        if (textureAtlas_->getTotalQuads() == textureAtlas_->getCapacity()) {
            increaseAtlasCapacity();
        }

        descendants_.push_back(sprite);
        int index = static_cast<int>(descendants_.size() - 1);

        sprite->setAtlasIndex(index);

        V3F_C4B_T2F_Quad quad = sprite->getQuad();
        textureAtlas_->insertQuad(&quad, index);

        // add children recursively
        auto& children = sprite->getChildren();
        for (const auto &child : children) {
            appendChild(static_cast<Sprite*>(child));
        }
    }

    void SpriteBatchNode::removeSpriteFromAtlas(Sprite *sprite) {
        textureAtlas_->removeQuadAtIndex(sprite->getAtlasIndex());
        sprite->setBatchNode(nullptr);

        auto it = std::find(descendants_.begin(), descendants_.end(), sprite);
        if (it != descendants_.end()) {
            auto next = std::next(it);

            Sprite *spr = nullptr;
            for (; next != descendants_.end(); ++next) {
                spr = *next;
                spr->setAtlasIndex(spr->getAtlasIndex() - 1);
            }

            descendants_.erase(it);
        }

        auto& children = sprite->getChildren();
        for (const auto &obj : children) {
            Sprite* child = static_cast<Sprite*>(obj);
            if (child) {
                removeSpriteFromAtlas(child);
            }
        }
    }

    void SpriteBatchNode::updateBlendFunc() {
        if (!textureAtlas_->getTexture()->hasPremultipliedAlpha()) {
            blendFunc_ = BlendFunc::ALPHA_NON_PREMULTIPLIED;
            setOpacityModifyRGB(false);
        }
        else {
            blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
            setOpacityModifyRGB(true);
        }
    }

    void SpriteBatchNode::setBlendFunc(const BlendFunc &blendFunc) {
        blendFunc_ = blendFunc;
    }

    const BlendFunc& SpriteBatchNode::getBlendFunc() const {
        return blendFunc_;
    }

    GLTexture* SpriteBatchNode::getTexture() const {
        return textureAtlas_->getTexture();
    }

    void SpriteBatchNode::setTexture(GLTexture *texture) {
        textureAtlas_->setTexture(texture);
        updateBlendFunc();
    }

    void SpriteBatchNode::insertQuadFromSprite(Sprite *sprite, uint64 index) {
        while (index >= textureAtlas_->getCapacity() || textureAtlas_->getCapacity() == textureAtlas_->getTotalQuads()) {
            this->increaseAtlasCapacity();
        }

        sprite->setBatchNode(this);
        sprite->setAtlasIndex(index);

        V3F_C4B_T2F_Quad quad = sprite->getQuad();
        textureAtlas_->insertQuad(&quad, index);

        sprite->setDirty(true);
        sprite->updateTransform();
    }

    void SpriteBatchNode::updateQuadFromSprite(Sprite *sprite, uint64 index) {
        while (index >= textureAtlas_->getCapacity() || textureAtlas_->getCapacity() == textureAtlas_->getTotalQuads()) {
            this->increaseAtlasCapacity();
        }

        sprite->setBatchNode(this);
        sprite->setAtlasIndex(index);
        sprite->setDirty(true);
        sprite->updateTransform();
    }

    SpriteBatchNode *SpriteBatchNode::addSpriteWithoutQuad(Sprite*child, int z, int aTag) {
        child->setAtlasIndex(z);

        auto it = descendants_.begin();
        for (; it != descendants_.end(); ++it) {
            if ((*it)->getAtlasIndex() >= z)
                break;
        }

        descendants_.insert(it, child);

        Node::addChild(child, z, aTag);        
        reorderBatch(false);

        return this;
    }
}
