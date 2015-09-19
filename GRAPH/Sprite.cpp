#include <algorithm>
#include "GRAPH/Sprite.h"
#include "GRAPH/SpriteFrame.h"
#include "GRAPH/Director.h"
#include "GRAPH/RENDERER/Texture2D.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/Renderer.h"

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

    void PolygonInfo::releaseVertsAndIndices()
    {
        if (isVertsOwner)
        {
            if (nullptr != triangles.verts)
            {
                SAFE_DELETE_ARRAY(triangles.verts);
            }

            if (nullptr != triangles.indices)
            {
                SAFE_DELETE_ARRAY(triangles.indices);
            }
        }
    }

    const unsigned int PolygonInfo::getVertCount() const
    {
        return (unsigned int) triangles.vertCount;
    }

    const unsigned int PolygonInfo::getTriaglesCount() const
    {
        return (unsigned int) triangles.indexCount / 3;
    }

    const float PolygonInfo::getArea() const
    {
        float area = 0;
        V3F_C4B_T2F *verts = triangles.verts;
        unsigned short *indices = triangles.indices;
        for (int i = 0; i < triangles.indexCount; i += 3)
        {
            auto A = verts[indices[i]].vertices;
            auto B = verts[indices[i + 1]].vertices;
            auto C = verts[indices[i + 2]].vertices;
            area += (A.x*(B.y - C.y) + B.x*(C.y - A.y) + C.x*(A.y - B.y)) / 2;
        }
        return area;
    }

    // MARK: create, init, dealloc
    Sprite* Sprite::createWithTexture(Texture2D *texture)
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithTexture(texture))
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::createWithTexture(Texture2D *texture, const MATH::Rectf& rect, bool rotated)
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithTexture(texture, rect, rotated))
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::createWithSpriteFrame(SpriteFrame *spriteFrame)
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::createWithSpriteFrameName(const std::string& spriteFrameName)
    {
        SpriteFrame *frame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);
        return createWithSpriteFrame(frame);
    }

    Sprite* Sprite::create(const std::string& filename)
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithFile(filename))
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::create(const PolygonInfo& info)
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if(sprite && sprite->initWithPolygon(info))
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::create(const std::string& filename, const MATH::Rectf& rect)
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->initWithFile(filename, rect))
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    Sprite* Sprite::create()
    {
        Sprite *sprite = new (std::nothrow) Sprite();
        if (sprite && sprite->init())
        {
            sprite->autorelease();
            return sprite;
        }
        SAFE_DELETE(sprite);
        return nullptr;
    }

    bool Sprite::init(void)
    {
        return initWithTexture(nullptr, MATH::RectfZERO );
    }

    bool Sprite::initWithTexture(Texture2D *texture)
    {
        MATH::Rectf rect = MATH::RectfZERO;
        rect.size = texture->getContentSize();

        return initWithTexture(texture, rect);
    }

    bool Sprite::initWithTexture(Texture2D *texture, const MATH::Rectf& rect)
    {
        return initWithTexture(texture, rect, false);
    }

    bool Sprite::initWithSpriteFrameName(const std::string& spriteFrameName)
    {
        SpriteFrame *frame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);
        return initWithSpriteFrame(frame);
    }

    bool Sprite::initWithSpriteFrame(SpriteFrame *spriteFrame)
    {
        bool bRet = initWithTexture(spriteFrame->getTexture(), spriteFrame->getRect());
        setSpriteFrame(spriteFrame);

        return bRet;
    }

    bool Sprite::initWithFile(const std::string& filename)
    {
        Texture2D *texture = Director::getInstance().getTextureCache()->addImage(filename);
        if (texture)
        {
            MATH::Rectf rect = MATH::RectfZERO;
            rect.size = texture->getContentSize();
            return initWithTexture(texture, rect);
        }

        // don't release here.
        // when load texture failed, it's better to get a "transparent" sprite then a crashed program
        // this->release();
        return false;
    }

    bool Sprite::initWithFile(const std::string &filename, const MATH::Rectf& rect)
    {
        Texture2D *texture = Director::getInstance().getTextureCache()->addImage(filename);
        if (texture)
        {
            return initWithTexture(texture, rect);
        }

        // don't release here.
        // when load texture failed, it's better to get a "transparent" sprite then a crashed program
        // this->release();
        return false;
    }

    bool Sprite::initWithPolygon(const PolygonInfo &info)
    {
        _polyInfo = info;
        setContentSize(_polyInfo.rect.size);
        return true;
    }

    // designated initializer
    bool Sprite::initWithTexture(Texture2D *texture, const MATH::Rectf& rect, bool rotated)
    {
        bool result;
        if (Node::init())
        {
            _batchNode = nullptr;

            _recursiveDirty = false;
            setDirty(false);

            _opacityModifyRGB = true;

            _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

            _flippedX = _flippedY = false;

            // default transform anchor: center
            setAnchorPoint(MATH::Vector2f(0.5f, 0.5f));

            // zwoptex default values
            _offsetPosition.setZero();

            // clean the Quad
            memset(&_quad, 0, sizeof(_quad));

            // Atlas: Color
            _quad.bl.colors = Color4B::WHITE;
            _quad.br.colors = Color4B::WHITE;
            _quad.tl.colors = Color4B::WHITE;
            _quad.tr.colors = Color4B::WHITE;

            // shader state
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));

            // update texture (calls updateBlendFunc)
            setTexture(texture);
            setTextureRect(rect, rotated, rect.size);

            _polyInfo.setQuad(&_quad);
            // by default use "Self Render".
            // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
            setBatchNode(nullptr);
            result = true;
        }
        else
        {
            result = false;
        }
        _recursiveDirty = true;
        setDirty(true);
        return result;
    }

    Sprite::Sprite(void)
    : _shouldBeHidden(false)
    , _texture(nullptr)
    , _spriteFrame(nullptr)
    , _insideBounds(true)
    {
    }

    Sprite::~Sprite(void)
    {
        SAFE_RELEASE(_spriteFrame);
        SAFE_RELEASE(_texture);
    }

    /*
     * Texture methods
     */

    /*
     * This array is the data of a white image with 2 by 2 dimension.
     * It's used for creating a default texture when sprite's texture is set to nullptr.
     * Supposing codes as follows:
     *
     *   auto sp = new (std::nothrow) Sprite();
     *   sp->init();  // Texture was set to nullptr, in order to make opacity and color to work correctly, we need to create a 2x2 white texture.
     *
     * The test is in "TestCpp/SpriteTest/Sprite without texture".
     */
    static unsigned char _2x2_white_image[] = {
        // RGBA8888
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    #define _2x2_WHITE_IMAGE_KEY  "/_2x2_white_image"

    // MARK: texture
    void Sprite::setTexture(const std::string &filename)
    {
        Texture2D *texture = Director::getInstance().getTextureCache()->addImage(filename);
        setTexture(texture);

        MATH::Rectf rect = MATH::RectfZERO;
        if (texture)
            rect.size = texture->getContentSize();
        setTextureRect(rect);
    }

    void Sprite::setTexture(Texture2D *texture)
    {
        if (texture == nullptr)
        {
            // Gets the texture by key firstly.
            texture = Director::getInstance().getTextureCache()->getTextureForKey(_2x2_WHITE_IMAGE_KEY);

            // If texture wasn't in cache, create it from RAW data.
            if (texture == nullptr)
            {
                IMAGE::TinyImage* image = new (std::nothrow) IMAGE::TinyImage();
                image->initWithRawData(_2x2_white_image, sizeof(_2x2_white_image), 2, 2, 8);
                texture = Director::getInstance().getTextureCache()->addImage(image, _2x2_WHITE_IMAGE_KEY);
                SAFE_RELEASE(image);
            }
        }

        if (_texture != texture)
        {
            SAFE_RETAIN(texture);
            SAFE_RELEASE(_texture);
            _texture = texture;
            updateBlendFunc();
        }
    }

    Texture2D* Sprite::getTexture() const
    {
        return _texture;
    }

    void Sprite::setTextureRect(const MATH::Rectf& rect)
    {
        setTextureRect(rect, false, rect.size);
    }

    void Sprite::setTextureRect(const MATH::Rectf& rect, bool rotated, const MATH::Sizef& untrimmedSize)
    {
        _rectRotated = rotated;

        setContentSize(untrimmedSize);
        setVertexRect(rect);
        setTextureCoords(rect);

        float relativeOffsetX = _unflippedOffsetPositionFromCenter.x;
        float relativeOffsetY = _unflippedOffsetPositionFromCenter.y;

        // issue #732
        if (_flippedX)
        {
            relativeOffsetX = -relativeOffsetX;
        }
        if (_flippedY)
        {
            relativeOffsetY = -relativeOffsetY;
        }

        _offsetPosition.x = relativeOffsetX + (_contentSize.width - _rect.size.width) / 2;
        _offsetPosition.y = relativeOffsetY + (_contentSize.height - _rect.size.height) / 2;

        // rendering using batch node
        if (_batchNode)
        {
            // update dirty_, don't update recursiveDirty_
            setDirty(true);
        }
        else
        {
            // self rendering

            // Atlas: Vertex
            float x1 = 0.0f + _offsetPosition.x;
            float y1 = 0.0f + _offsetPosition.y;
            float x2 = x1 + _rect.size.width;
            float y2 = y1 + _rect.size.height;

            // Don't update Z.
            _quad.bl.vertices.set(x1, y1, 0.0f);
            _quad.br.vertices.set(x2, y1, 0.0f);
            _quad.tl.vertices.set(x1, y2, 0.0f);
            _quad.tr.vertices.set(x2, y2, 0.0f);
        }
    }

    // override this method to generate "double scale" sprites
    void Sprite::setVertexRect(const MATH::Rectf& rect)
    {
        _rect = rect;
    }

    void Sprite::setSpriteFrame(const std::string &spriteFrameName)
    {
        SpriteFrame *spriteFrame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);

        setSpriteFrame(spriteFrame);
    }

    void Sprite::setSpriteFrame(SpriteFrame *spriteFrame)
    {
        // retain the sprite frame
        // do not removed by SpriteFrameCache::removeUnusedSpriteFrames
        if (_spriteFrame != spriteFrame)
        {
            SAFE_RELEASE(_spriteFrame);
            _spriteFrame = spriteFrame;
            spriteFrame->retain();
        }
        _unflippedOffsetPositionFromCenter = spriteFrame->getOffset();

        Texture2D *texture = spriteFrame->getTexture();
        // update texture before updating texture rect
        if (texture != _texture)
        {
            setTexture(texture);
        }

        // update rect
        _rectRotated = spriteFrame->isRotated();
        setTextureRect(spriteFrame->getRect(), _rectRotated, spriteFrame->getOriginalSize());
    }

    bool Sprite::isFrameDisplayed(SpriteFrame *frame) const
    {
        MATH::Rectf r = frame->getRect();

        return (r.equals(_rect) &&
                frame->getTexture()->getName() == _texture->getName() &&
                frame->getOffset().equals(_unflippedOffsetPositionFromCenter));
    }

    SpriteFrame* Sprite::getSpriteFrame() const
    {
        if(nullptr != this->_spriteFrame)
        {
            return this->_spriteFrame;
        }
        return SpriteFrame::createWithTexture(_texture,
                                               _rect,
                                               _rectRotated,
                                               _unflippedOffsetPositionFromCenter,
                                               _contentSize);
    }

    void Sprite::setTextureCoords(MATH::Rectf rect)
    {
        Texture2D *tex = _batchNode ? _textureAtlas->getTexture() : _texture;
        if (! tex)
        {
            return;
        }

        float atlasWidth = (float)tex->getPixelsWidth();
        float atlasHeight = (float)tex->getPixelsHight();

        float left, right, top, bottom;

        if (_rectRotated)
        {
            left    = rect.origin.x/atlasWidth;
            right   = (rect.origin.x+rect.size.height) / atlasWidth;
            top     = rect.origin.y/atlasHeight;
            bottom  = (rect.origin.y+rect.size.width) / atlasHeight;

            if (_flippedX)
            {
                std::swap(top, bottom);
            }

            if (_flippedY)
            {
                std::swap(left, right);
            }

            _quad.bl.texCoords.u = left;
            _quad.bl.texCoords.v = top;
            _quad.br.texCoords.u = left;
            _quad.br.texCoords.v = bottom;
            _quad.tl.texCoords.u = right;
            _quad.tl.texCoords.v = top;
            _quad.tr.texCoords.u = right;
            _quad.tr.texCoords.v = bottom;
        }
        else
        {
            left    = rect.origin.x/atlasWidth;
            right    = (rect.origin.x + rect.size.width) / atlasWidth;
            top        = rect.origin.y/atlasHeight;
            bottom    = (rect.origin.y + rect.size.height) / atlasHeight;

            if(_flippedX)
            {
                std::swap(left, right);
            }

            if(_flippedY)
            {
                std::swap(top, bottom);
            }

            _quad.bl.texCoords.u = left;
            _quad.bl.texCoords.v = bottom;
            _quad.br.texCoords.u = right;
            _quad.br.texCoords.v = bottom;
            _quad.tl.texCoords.u = left;
            _quad.tl.texCoords.v = top;
            _quad.tr.texCoords.u = right;
            _quad.tr.texCoords.v = top;
        }
    }

    // MARK: visit, draw, transform

    void Sprite::updateTransform(void)
    {
        // recalculate matrix only if it is dirty
        if( isDirty() ) {

            // If it is not visible, or one of its ancestors is not visible, then do nothing:
            if( !_visible || ( _parent && _parent != _batchNode && static_cast<Sprite*>(_parent)->_shouldBeHidden) )
            {
                _quad.br.vertices.setZero();
                _quad.tl.vertices.setZero();
                _quad.tr.vertices.setZero();
                _quad.bl.vertices.setZero();
                _shouldBeHidden = true;
            }
            else
            {
                _shouldBeHidden = false;

                if( ! _parent || _parent == _batchNode )
                {
                    _transformToBatch = getNodeToParentTransform();
                }
                else
                {
                    const MATH::Matrix4 &nodeToParent = getNodeToParentTransform();
                    MATH::Matrix4 &parentTransform = static_cast<Sprite*>(_parent)->_transformToBatch;
                    _transformToBatch = parentTransform * nodeToParent;
                }

                //
                // calculate the Quad based on the Affine Matrix
                //

                MATH::Sizef &size = _rect.size;

                float x1 = _offsetPosition.x;
                float y1 = _offsetPosition.y;

                float x2 = x1 + size.width;
                float y2 = y1 + size.height;
                float x = _transformToBatch.m[12];
                float y = _transformToBatch.m[13];

                float cr = _transformToBatch.m[0];
                float sr = _transformToBatch.m[1];
                float cr2 = _transformToBatch.m[5];
                float sr2 = -_transformToBatch.m[4];
                float ax = x1 * cr - y1 * sr2 + x;
                float ay = x1 * sr + y1 * cr2 + y;

                float bx = x2 * cr - y1 * sr2 + x;
                float by = x2 * sr + y1 * cr2 + y;

                float cx = x2 * cr - y2 * sr2 + x;
                float cy = x2 * sr + y2 * cr2 + y;

                float dx = x1 * cr - y2 * sr2 + x;
                float dy = x1 * sr + y2 * cr2 + y;

                _quad.bl.vertices.set(ax, ay, _positionZ);
                _quad.br.vertices.set(bx, by, _positionZ);
                _quad.tl.vertices.set(dx, dy, _positionZ);
                _quad.tr.vertices.set(cx, cy, _positionZ);
            }

            // MARMALADE CHANGE: ADDED CHECK FOR nullptr, TO PERMIT SPRITES WITH NO BATCH NODE / TEXTURE ATLAS
            if (_textureAtlas)
            {
                _textureAtlas->updateQuad(&_quad, _atlasIndex);
            }

            _recursiveDirty = false;
            setDirty(false);
        }

        Node::updateTransform();
    }

    // draw

    void Sprite::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
    {
        _trianglesCommand.init(_globalZOrder, _texture->getName(), getGLProgramState(), _blendFunc, _polyInfo.triangles, transform, flags);
        renderer->addCommand(&_trianglesCommand);
    }

    // MARK: visit, draw, transform

    void Sprite::addChild(Node *child, int zOrder, int tag)
    {
        if (_batchNode)
        {
            Sprite* childSprite = dynamic_cast<Sprite*>(child);
            //put it in descendants array of batch node
            _batchNode->appendChild(childSprite);

            if (!_reorderChildDirty)
            {
                setReorderChildDirtyRecursively();
            }
        }
        //CCNode already sets isReorderChildDirty_ so this needs to be after batchNode check
        Node::addChild(child, zOrder, tag);
    }

    void Sprite::addChild(Node *child, int zOrder, const std::string &name)
    {
        if (_batchNode)
        {
            Sprite* childSprite = dynamic_cast<Sprite*>(child);
            //put it in descendants array of batch node
            _batchNode->appendChild(childSprite);

            if (!_reorderChildDirty)
            {
                setReorderChildDirtyRecursively();
            }
        }
        //CCNode already sets isReorderChildDirty_ so this needs to be after batchNode check
        Node::addChild(child, zOrder, name);
    }

    void Sprite::reorderChild(Node *child, int zOrder)
    {
        if( _batchNode && ! _reorderChildDirty)
        {
            setReorderChildDirtyRecursively();
            _batchNode->reorderBatch(true);
        }

        Node::reorderChild(child, zOrder);
    }

    void Sprite::removeChild(Node *child, bool cleanup)
    {
        if (_batchNode)
        {
            _batchNode->removeSpriteFromAtlas((Sprite*)(child));
        }

        Node::removeChild(child, cleanup);
    }

    void Sprite::removeAllChildrenWithCleanup(bool cleanup)
    {
        if (_batchNode)
        {
            for(const auto &child : _children) {
                Sprite* sprite = dynamic_cast<Sprite*>(child);
                if (sprite)
                {
                    _batchNode->removeSpriteFromAtlas(sprite);
                }
            }
        }

        Node::removeAllChildrenWithCleanup(cleanup);
    }

    void Sprite::sortAllChildren()
    {
        if (_reorderChildDirty)
        {
            std::sort(std::begin(_children), std::end(_children), NodeComparisonLess);

            if ( _batchNode)
            {
                for(const auto &child : _children)
                    child->sortAllChildren();
            }

            _reorderChildDirty = false;
        }
    }

    //
    // Node property overloads
    // used only when parent is SpriteBatchNode
    //

    void Sprite::setReorderChildDirtyRecursively(void)
    {
        //only set parents flag the first time
        if ( ! _reorderChildDirty )
        {
            _reorderChildDirty = true;
            Node* node = static_cast<Node*>(_parent);
            while (node && node != _batchNode)
            {
                static_cast<Sprite*>(node)->setReorderChildDirtyRecursively();
                node=node->getParent();
            }
        }
    }

    void Sprite::setDirtyRecursively(bool bValue)
    {
        _recursiveDirty = bValue;
        setDirty(bValue);

        for(const auto &child: _children) {
            Sprite* sp = dynamic_cast<Sprite*>(child);
            if (sp)
            {
                sp->setDirtyRecursively(true);
            }
        }
    }

    // FIXME: HACK: optimization
    #define SET_DIRTY_RECURSIVELY() {                       \
                        if (! _recursiveDirty) {            \
                            _recursiveDirty = true;         \
                            setDirty(true);                 \
                            if (!_children.empty())         \
                                setDirtyRecursively(true);  \
                            }                               \
                        }

    void Sprite::setPosition(const MATH::Vector2f& pos)
    {
        Node::setPosition(pos);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setPosition(float x, float y)
    {
        Node::setPosition(x, y);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setRotation(float rotation)
    {
        Node::setRotation(rotation);

        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setRotationSkewX(float fRotationX)
    {
        Node::setRotationSkewX(fRotationX);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setRotationSkewY(float fRotationY)
    {
        Node::setRotationSkewY(fRotationY);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setSkewX(float sx)
    {
        Node::setSkewX(sx);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setSkewY(float sy)
    {
        Node::setSkewY(sy);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScaleX(float scaleX)
    {
        Node::setScaleX(scaleX);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScaleY(float scaleY)
    {
        Node::setScaleY(scaleY);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScale(float fScale)
    {
        Node::setScale(fScale);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setScale(float scaleX, float scaleY)
    {
        Node::setScale(scaleX, scaleY);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setPositionZ(float fVertexZ)
    {
        Node::setPositionZ(fVertexZ);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setAnchorPoint(const MATH::Vector2f& anchor)
    {
        Node::setAnchorPoint(anchor);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::ignoreAnchorPointForPosition(bool value)
    {
        Node::ignoreAnchorPointForPosition(value);
    }

    void Sprite::setVisible(bool bVisible)
    {
        Node::setVisible(bVisible);
        SET_DIRTY_RECURSIVELY();
    }

    void Sprite::setFlippedX(bool flippedX)
    {
        if (_flippedX != flippedX)
        {
            _flippedX = flippedX;
            setTextureRect(_rect, _rectRotated, _contentSize);
        }
    }

    bool Sprite::isFlippedX(void) const
    {
        return _flippedX;
    }

    void Sprite::setFlippedY(bool flippedY)
    {
        if (_flippedY != flippedY)
        {
            _flippedY = flippedY;
            setTextureRect(_rect, _rectRotated, _contentSize);
        }
    }

    bool Sprite::isFlippedY(void) const
    {
        return _flippedY;
    }

    //
    // MARK: RGBA protocol
    //

    void Sprite::updateColor(void)
    {
        Color4B color4( _displayedColor.red, _displayedColor.green, _displayedColor.blue, _displayedOpacity );

        // special opacity for premultiplied textures
        if (_opacityModifyRGB)
        {
            color4.red *= _displayedOpacity/255.0f;
            color4.green *= _displayedOpacity/255.0f;
            color4.blue *= _displayedOpacity/255.0f;
        }

        _quad.bl.colors = color4;
        _quad.br.colors = color4;
        _quad.tl.colors = color4;
        _quad.tr.colors = color4;

        // renders using batch node
        if (_batchNode)
        {
            if (_atlasIndex != INDEX_NOT_INITIALIZED)
            {
                _textureAtlas->updateQuad(&_quad, _atlasIndex);
            }
            else
            {
                // no need to set it recursively
                // update dirty_, don't update recursiveDirty_
                setDirty(true);
            }
        }

        // self render
        // do nothing
    }

    void Sprite::setOpacityModifyRGB(bool modify)
    {
        if (_opacityModifyRGB != modify)
        {
            _opacityModifyRGB = modify;
            updateColor();
        }
    }

    bool Sprite::isOpacityModifyRGB(void) const
    {
        return _opacityModifyRGB;
    }

    SpriteBatchNode* Sprite::getBatchNode() const
    {
        return _batchNode;
    }

    void Sprite::setBatchNode(SpriteBatchNode *spriteBatchNode)
    {
        _batchNode = spriteBatchNode; // weak reference

        // self render
        if( ! _batchNode ) {
            _atlasIndex = INDEX_NOT_INITIALIZED;
            setTextureAtlas(nullptr);
            _recursiveDirty = false;
            setDirty(false);

            float x1 = _offsetPosition.x;
            float y1 = _offsetPosition.y;
            float x2 = x1 + _rect.size.width;
            float y2 = y1 + _rect.size.height;
            _quad.bl.vertices.set( x1, y1, 0 );
            _quad.br.vertices.set(x2, y1, 0);
            _quad.tl.vertices.set(x1, y2, 0);
            _quad.tr.vertices.set(x2, y2, 0);

        } else {

            // using batch
            _transformToBatch = MATH::Matrix4::IDENTITY;
            setTextureAtlas(_batchNode->getTextureAtlas()); // weak ref
        }
    }

    // MARK: Texture protocol

    void Sprite::updateBlendFunc(void)
    {
        // it is possible to have an untextured spritec
        if (! _texture || ! _texture->hasPremultipliedAlpha())
        {
            _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
            setOpacityModifyRGB(false);
        }
        else
        {
            _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
            setOpacityModifyRGB(true);
        }
    }

    PolygonInfo Sprite::getPolygonInfo() const
    {
        return _polyInfo;
    }

    void Sprite::setPolygonInfo(const PolygonInfo& info)
    {
        _polyInfo = info;
    }

    SpriteBatchNode* SpriteBatchNode::createWithTexture(Texture2D* tex, ssize_t capacity/* = DEFAULT_CAPACITY*/)
    {
        SpriteBatchNode *batchNode = new (std::nothrow) SpriteBatchNode();
        batchNode->initWithTexture(tex, capacity);
        batchNode->autorelease();

        return batchNode;
    }

    /*
    * creation with File Image
    */

    SpriteBatchNode* SpriteBatchNode::create(const std::string& fileImage, ssize_t capacity/* = DEFAULT_CAPACITY*/)
    {
        SpriteBatchNode *batchNode = new (std::nothrow) SpriteBatchNode();
        batchNode->initWithFile(fileImage, capacity);
        batchNode->autorelease();

        return batchNode;
    }

    /*
    * init with Texture2D
    */
    bool SpriteBatchNode::initWithTexture(Texture2D *tex, ssize_t capacity/* = DEFAULT_CAPACITY*/)
    {
        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
        if (!tex->hasPremultipliedAlpha())
        {
            _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
        }
        _textureAtlas = new (std::nothrow) TextureAtlas();

        if (capacity == 0)
        {
            capacity = DEFAULT_CAPACITY;
        }

        _textureAtlas->initWithTexture(tex, capacity);

        updateBlendFunc();

        _children.reserve(capacity);

        _descendants.reserve(capacity);

        setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR));
        return true;
    }

    bool SpriteBatchNode::init()
    {
        Texture2D * texture = new (std::nothrow) Texture2D();
        texture->autorelease();
        return this->initWithTexture(texture, 0);
    }

    /*
    * init with FileImage
    */
    bool SpriteBatchNode::initWithFile(const std::string& fileImage, ssize_t capacity/* = DEFAULT_CAPACITY*/)
    {
        Texture2D *texture2D = Director::getInstance().getTextureCache()->addImage(fileImage);
        return initWithTexture(texture2D, capacity);
    }

    SpriteBatchNode::SpriteBatchNode()
        : _textureAtlas(nullptr)
    {
    }

    SpriteBatchNode::~SpriteBatchNode()
    {
        SAFE_RELEASE(_textureAtlas);
    }

    // override visit
    // don't call visit on it's children
    void SpriteBatchNode::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
    {
        // CAREFUL:
        // This visit is almost identical to CocosNode#visit
        // with the exception that it doesn't call visit on it's children
        //
        // The alternative is to have a void Sprite#visit, but
        // although this is less maintainable, is faster
        //
        if (!_visible)
        {
            return;
        }

        sortAllChildren();

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        Director::getInstance().pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        Director::getInstance().loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

        draw(renderer, _modelViewTransform, flags);

        Director::getInstance().popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    void SpriteBatchNode::addChild(Node *child, int zOrder, int tag)
    {
        Sprite *sprite = static_cast<Sprite*>(child);

        Node::addChild(child, zOrder, tag);

        appendChild(sprite);
    }

    void SpriteBatchNode::addChild(Node * child, int zOrder, const std::string &name)
    {
        Sprite *sprite = static_cast<Sprite*>(child);

        Node::addChild(child, zOrder, name);

        appendChild(sprite);
    }

    // override reorderChild
    void SpriteBatchNode::reorderChild(Node *child, int zOrder)
    {
        if (zOrder == child->getLocalZOrder())
        {
            return;
        }

        //set the z-order and sort later
        Node::reorderChild(child, zOrder);
    }

    // override remove child
    void SpriteBatchNode::removeChild(Node *child, bool cleanup)
    {
        Sprite *sprite = static_cast<Sprite*>(child);

        // explicit null handling
        if (sprite == nullptr)
        {
            return;
        }

        // cleanup before removing
        removeSpriteFromAtlas(sprite);

        Node::removeChild(sprite, cleanup);
    }

    void SpriteBatchNode::setTextureAtlas(TextureAtlas* textureAtlas) {
        if (textureAtlas != _textureAtlas) {
            SAFE_RETAIN(textureAtlas);
            SAFE_RELEASE(_textureAtlas);
            _textureAtlas = textureAtlas;
        }
    }

    void SpriteBatchNode::removeChildAtIndex(ssize_t index, bool doCleanup)
    {
        removeChild(_children.at(index), doCleanup);
    }

    void SpriteBatchNode::removeAllChildrenWithCleanup(bool doCleanup)
    {
        // Invalidate atlas index. issue #569
        // useSelfRender should be performed on all descendants. issue #1216
        for (const auto &sprite : _descendants) {
            sprite->setBatchNode(nullptr);
        }

        Node::removeAllChildrenWithCleanup(doCleanup);

        _descendants.clear();
        _textureAtlas->removeAllQuads();
    }

    //override sortAllChildren
    void SpriteBatchNode::sortAllChildren()
    {
        if (_reorderChildDirty)
        {
            std::sort(std::begin(_children), std::end(_children), NodeComparisonLess);

            //sorted now check all children
            if (!_children.empty())
            {
                //first sort all children recursively based on zOrder
                for (const auto &child : _children) {
                    child->sortAllChildren();
                }

                ssize_t index = 0;

                //fast dispatch, give every child a new atlasIndex based on their relative zOrder (keep parent -> child relations intact)
                // and at the same time reorder descendants and the quads to the right index
                for (const auto &child : _children) {
                    Sprite* sp = static_cast<Sprite*>(child);
                    updateAtlasIndex(sp, &index);
                }
            }

            _reorderChildDirty = false;
        }
    }

    void SpriteBatchNode::updateAtlasIndex(Sprite* sprite, ssize_t* curIndex)
    {
        auto& array = sprite->getChildren();
        auto count = array.size();

        ssize_t oldIndex = 0;

        if (count == 0)
        {
            oldIndex = sprite->getAtlasIndex();
            sprite->setAtlasIndex(*curIndex);
            if (oldIndex != *curIndex){
                swap(oldIndex, *curIndex);
            }
            (*curIndex)++;
        }
        else
        {
            bool needNewIndex = true;

            if (array.at(0)->getLocalZOrder() >= 0)
            {
                //all children are in front of the parent
                oldIndex = sprite->getAtlasIndex();
                sprite->setAtlasIndex(*curIndex);
                if (oldIndex != *curIndex)
                {
                    swap(oldIndex, *curIndex);
                }
                (*curIndex)++;

                needNewIndex = false;
            }

            for (const auto &child : array) {
                Sprite* sp = static_cast<Sprite*>(child);
                if (needNewIndex && sp->getLocalZOrder() >= 0)
                {
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

            if (needNewIndex)
            {//all children have a zOrder < 0)
                oldIndex = sprite->getAtlasIndex();
                sprite->setAtlasIndex(*curIndex);
                if (oldIndex != *curIndex) {
                    swap(oldIndex, *curIndex);
                }
                (*curIndex)++;
            }
        }
    }

    void SpriteBatchNode::swap(ssize_t oldIndex, ssize_t newIndex)
    {
        V3F_C4B_T2F_Quad* quads = _textureAtlas->getQuads();
        std::swap(quads[oldIndex], quads[newIndex]);

        //update the index of other swapped item

        auto oldIt = std::next(_descendants.begin(), oldIndex);
        auto newIt = std::next(_descendants.begin(), newIndex);

        (*newIt)->setAtlasIndex(oldIndex);
        //    (*oldIt)->setAtlasIndex(newIndex);

        std::swap(*oldIt, *newIt);
    }

    void SpriteBatchNode::reorderBatch(bool reorder)
    {
        _reorderChildDirty = reorder;
    }

    void SpriteBatchNode::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
    {
        // Optimization: Fast Dispatch
        if (_textureAtlas->getTotalQuads() == 0)
        {
            return;
        }

        for (const auto &child : _children)
        {
#if CC_USE_PHYSICS
            auto physicsBody = child->getPhysicsBody();
            if (physicsBody)
            {
                child->updateTransformFromPhysics(transform, flags);
            }
#endif
            child->updateTransform();
        }

        _batchCommand.init(_globalZOrder, getGLProgram(), _blendFunc, _textureAtlas, transform, flags);
        renderer->addCommand(&_batchCommand);
    }

    void SpriteBatchNode::increaseAtlasCapacity()
    {
        // if we're going beyond the current TextureAtlas's capacity,
        // all the previously initialized sprites will need to redo their texture coords
        // this is likely computationally expensive
        ssize_t quantity = (_textureAtlas->getCapacity() + 1) * 4 / 3;

        if (!_textureAtlas->resizeCapacity(quantity))
        {
        }
    }

    ssize_t SpriteBatchNode::rebuildIndexInOrder(Sprite *parent, ssize_t index)
    {
        auto& children = parent->getChildren();
        for (const auto &child : children) {
            Sprite* sp = static_cast<Sprite*>(child);
            if (sp && (sp->getLocalZOrder() < 0))
            {
                index = rebuildIndexInOrder(sp, index);
            }
        }

        // ignore self (batch node)
        if (parent != static_cast<HObject*>(this))
        {
            parent->setAtlasIndex(index);
            index++;
        }

        for (const auto &child : children) {
            Sprite* sp = static_cast<Sprite*>(child);
            if (sp && (sp->getLocalZOrder() >= 0))
            {
                index = rebuildIndexInOrder(sp, index);
            }
        }

        return index;
    }

    ssize_t SpriteBatchNode::highestAtlasIndexInChild(Sprite *sprite)
    {
        auto& children = sprite->getChildren();

        if (children.size() == 0)
        {
            return sprite->getAtlasIndex();
        }
        else
        {
            return highestAtlasIndexInChild(static_cast<Sprite*>(children.back()));
        }
    }

    ssize_t SpriteBatchNode::lowestAtlasIndexInChild(Sprite *sprite)
    {
        auto& children = sprite->getChildren();

        if (children.size() == 0)
        {
            return sprite->getAtlasIndex();
        }
        else
        {
            return lowestAtlasIndexInChild(static_cast<Sprite*>(children.at(0)));
        }
    }

    ssize_t SpriteBatchNode::atlasIndexForChild(Sprite *sprite, int nZ)
    {
        auto& siblings = sprite->getParent()->getChildren();
        auto childIndex = siblings.getIndex(sprite);

        // ignore parent Z if parent is spriteSheet
        bool ignoreParent = (SpriteBatchNode*) (sprite->getParent()) == this;
        Sprite *prev = nullptr;
        if (childIndex > 0 && childIndex != -1)
        {
            prev = static_cast<Sprite*>(siblings.at(childIndex - 1));
        }

        // first child of the sprite sheet
        if (ignoreParent)
        {
            if (childIndex == 0)
            {
                return 0;
            }

            return highestAtlasIndexInChild(prev) + 1;
        }

        // parent is a Sprite, so, it must be taken into account

        // first child of an Sprite ?
        if (childIndex == 0)
        {
            Sprite *p = static_cast<Sprite*>(sprite->getParent());

            // less than parent and brothers
            if (nZ < 0)
            {
                return p->getAtlasIndex();
            }
            else
            {
                return p->getAtlasIndex() + 1;
            }
        }
        else
        {
            // previous & sprite belong to the same branch
            if ((prev->getLocalZOrder() < 0 && nZ < 0) || (prev->getLocalZOrder() >= 0 && nZ >= 0))
            {
                return highestAtlasIndexInChild(prev) + 1;
            }

            // else (previous < 0 and sprite >= 0 )
            Sprite *p = static_cast<Sprite*>(sprite->getParent());
            return p->getAtlasIndex() + 1;
        }

        return 0;
    }

    // addChild helper, faster than insertChild
    void SpriteBatchNode::appendChild(Sprite* sprite)
    {
        _reorderChildDirty = true;
        sprite->setBatchNode(this);
        sprite->setDirty(true);

        if (_textureAtlas->getTotalQuads() == _textureAtlas->getCapacity()) {
            increaseAtlasCapacity();
        }

        _descendants.push_back(sprite);
        int index = static_cast<int>(_descendants.size() - 1);

        sprite->setAtlasIndex(index);

        V3F_C4B_T2F_Quad quad = sprite->getQuad();
        _textureAtlas->insertQuad(&quad, index);

        // add children recursively
        auto& children = sprite->getChildren();
        for (const auto &child : children) {
            appendChild(static_cast<Sprite*>(child));
        }
    }

    void SpriteBatchNode::removeSpriteFromAtlas(Sprite *sprite)
    {
        // remove from TextureAtlas
        _textureAtlas->removeQuadAtIndex(sprite->getAtlasIndex());

        // Cleanup sprite. It might be reused (issue #569)
        sprite->setBatchNode(nullptr);

        auto it = std::find(_descendants.begin(), _descendants.end(), sprite);
        if (it != _descendants.end())
        {
            auto next = std::next(it);

            Sprite *spr = nullptr;
            for (; next != _descendants.end(); ++next) {
                spr = *next;
                spr->setAtlasIndex(spr->getAtlasIndex() - 1);
            }

            _descendants.erase(it);
        }

        // remove children recursively
        auto& children = sprite->getChildren();
        for (const auto &obj : children) {
            Sprite* child = static_cast<Sprite*>(obj);
            if (child)
            {
                removeSpriteFromAtlas(child);
            }
        }
    }

    void SpriteBatchNode::updateBlendFunc()
    {
        if (!_textureAtlas->getTexture()->hasPremultipliedAlpha())
        {
            _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
            setOpacityModifyRGB(false);
        }
        else
        {
            _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
            setOpacityModifyRGB(true);
        }
    }

    // CocosNodeTexture protocol
    void SpriteBatchNode::setBlendFunc(const BlendFunc &blendFunc)
    {
        _blendFunc = blendFunc;
    }

    const BlendFunc& SpriteBatchNode::getBlendFunc() const
    {
        return _blendFunc;
    }

    Texture2D* SpriteBatchNode::getTexture() const
    {
        return _textureAtlas->getTexture();
    }

    void SpriteBatchNode::setTexture(Texture2D *texture)
    {
        _textureAtlas->setTexture(texture);
        updateBlendFunc();
    }

    // SpriteSheet Extension
    //implementation SpriteSheet (TMXTiledMapExtension)

    void SpriteBatchNode::insertQuadFromSprite(Sprite *sprite, ssize_t index)
    {
        // make needed room
        while (index >= _textureAtlas->getCapacity() || _textureAtlas->getCapacity() == _textureAtlas->getTotalQuads())
        {
            this->increaseAtlasCapacity();
        }
        //
        // update the quad directly. Don't add the sprite to the scene graph
        //
        sprite->setBatchNode(this);
        sprite->setAtlasIndex(index);

        V3F_C4B_T2F_Quad quad = sprite->getQuad();
        _textureAtlas->insertQuad(&quad, index);

        // FIXME:: updateTransform will update the textureAtlas too, using updateQuad.
        // FIXME:: so, it should be AFTER the insertQuad
        sprite->setDirty(true);
        sprite->updateTransform();
    }

    void SpriteBatchNode::updateQuadFromSprite(Sprite *sprite, ssize_t index)
    {
        // make needed room
        while (index >= _textureAtlas->getCapacity() || _textureAtlas->getCapacity() == _textureAtlas->getTotalQuads())
        {
            this->increaseAtlasCapacity();
        }

        //
        // update the quad directly. Don't add the sprite to the scene graph
        //
        sprite->setBatchNode(this);
        sprite->setAtlasIndex(index);

        sprite->setDirty(true);

        // UpdateTransform updates the textureAtlas quad
        sprite->updateTransform();
    }

    SpriteBatchNode * SpriteBatchNode::addSpriteWithoutQuad(Sprite*child, int z, int aTag)
    {
        // quad index is Z
        child->setAtlasIndex(z);

        // FIXME:: optimize with a binary search
        auto it = _descendants.begin();
        for (; it != _descendants.end(); ++it)
        {
            if ((*it)->getAtlasIndex() >= z)
                break;
        }

        _descendants.insert(it, child);

        // IMPORTANT: Call super, and not self. Avoid adding it to the texture atlas array
        Node::addChild(child, z, aTag);

        //#issue 1262 don't use lazy sorting, tiles are added as quads not as sprites, so sprites need to be added in order
        reorderBatch(false);

        return this;
    }
}
