#include "GRAPH/BASE/Sprite.h"
#include <algorithm>
#include "GRAPH/BASE/Director.h"
#include "GRAPH/RENDERER/TextureCache.h"
#include "IMAGE/SmartImage.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/BASE/DrawNode.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "IO/FileUtils.h"
#include "GRAPH/BASE/Macros.h"
#include "UTILS/STRING/StringUtils.h"

namespace GRAPH
{
    static unsigned short quadIndices [] = { 0, 1, 2, 3, 2, 1 };
    const static float PRECISION = 10.0f;

    PolygonInfo::PolygonInfo(const PolygonInfo& other) :
        triangles(),
        isVertsOwner(true),
        rect()
    {
        filename = other.filename;
        isVertsOwner = other.isVertsOwner;
        rect = other.rect;
        triangles.verts = new V3F_C4B_T2F[other.triangles.vertCount];
        triangles.indices = new unsigned short[other.triangles.indexCount];
        triangles.vertCount = other.triangles.vertCount;
        triangles.indexCount = other.triangles.indexCount;
        memcpy(triangles.verts, other.triangles.verts, other.triangles.vertCount*sizeof(V3F_C4B_T2F));
        memcpy(triangles.indices, other.triangles.indices, other.triangles.indexCount*sizeof(unsigned short));
    };

    PolygonInfo& PolygonInfo::operator= (const PolygonInfo& other)
    {
        if (this != &other)
        {
            releaseVertsAndIndices();
            filename = other.filename;
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

    PolygonInfo::~PolygonInfo()
    {
        releaseVertsAndIndices();
    }

    void PolygonInfo::setQuad(V3F_C4B_T2F_Quad *quad)
    {
        releaseVertsAndIndices();
        isVertsOwner = false;
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
        SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
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
        SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
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
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
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
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
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
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(info.filename);
        bool res = false;
        if(initWithTexture(texture))
        {
            _polyInfo = info;
            setContentSize(_polyInfo.rect.size/Director::getInstance()->getContentScaleFactor());
            res = true;
        }
        return res;
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
    static unsigned char cc_2x2_white_image[] = {
        // RGBA8888
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    #define CC_2x2_WHITE_IMAGE_KEY  "/cc_2x2_white_image"

    // MARK: texture
    void Sprite::setTexture(const std::string &filename)
    {
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
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
            texture = Director::getInstance()->getTextureCache()->getTextureForKey(CC_2x2_WHITE_IMAGE_KEY);

            // If texture wasn't in cache, create it from RAW data.
            if (texture == nullptr)
            {
                IMAGE::SmartImage* image = new (std::nothrow) IMAGE::SmartImage();
                texture = Director::getInstance()->getTextureCache()->addImage(image, CC_2x2_WHITE_IMAGE_KEY);
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

    void Sprite::debugDraw(bool on)
    {
        DrawNode* draw = getChildByName<DrawNode*>("debugDraw");
        if(on)
        {
            if(!draw)
            {
                draw = DrawNode::create();
                draw->setName("debugDraw");
                addChild(draw);
            }
            draw->setVisible(true);
            draw->clear();
            //draw lines
            auto last = _polyInfo.triangles.indexCount/3;
            auto _indices = _polyInfo.triangles.indices;
            auto _verts = _polyInfo.triangles.verts;
            for(unsigned int i = 0; i < last; i++)
            {
                //draw 3 lines
                MATH::Vector3f from =_verts[_indices[i*3]].vertices;
                MATH::Vector3f to = _verts[_indices[i*3+1]].vertices;
                draw->drawLine(MATH::Vector2f(from.x, from.y), MATH::Vector2f(to.x,to.y), Color4F::GREEN);

                from =_verts[_indices[i*3+1]].vertices;
                to = _verts[_indices[i*3+2]].vertices;
                draw->drawLine(MATH::Vector2f(from.x, from.y), MATH::Vector2f(to.x,to.y), Color4F::GREEN);

                from =_verts[_indices[i*3+2]].vertices;
                to = _verts[_indices[i*3]].vertices;
                draw->drawLine(MATH::Vector2f(from.x, from.y), MATH::Vector2f(to.x,to.y), Color4F::GREEN);
            }
        }
        else
        {
            if(draw)
                draw->setVisible(false);
        }
    }


    // override this method to generate "double scale" sprites
    void Sprite::setVertexRect(const MATH::Rectf& rect)
    {
        _rect = rect;
    }

    void Sprite::setSpriteFrame(const std::string &spriteFrameName)
    {
        SpriteFrameCache *cache = SpriteFrameCache::getInstance();
        SpriteFrame *spriteFrame = cache->getSpriteFrameByName(spriteFrameName);

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
                                               CC_RECT_POINTS_TO_PIXELS(_rect),
                                               _rectRotated,
                                               CC_POINT_POINTS_TO_PIXELS(_unflippedOffsetPositionFromCenter),
                                               CC_SIZE_POINTS_TO_PIXELS(_contentSize));
    }

    void Sprite::setTextureCoords(MATH::Rectf rect)
    {
        rect = CC_RECT_POINTS_TO_PIXELS(rect);

        Texture2D *tex = _batchNode ? _textureAtlas->getTexture() : _texture;
        if (! tex)
        {
            return;
        }

        float atlasWidth = (float)tex->getPixelsWide();
        float atlasHeight = (float)tex->getPixelsHigh();

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
            std::sort(std::begin(_children), std::end(_children), nodeComparisonLess);

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
        Texture2D *texture2D = Director::getInstance()->getTextureCache()->addImage(fileImage);
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

        if (isVisitableByVisitingCamera())
        {
            // IMPORTANT:
            // To ease the migration to v3.0, we still support the Mat4 stack,
            // but it is deprecated and your code should not rely on it
            Director* director = Director::getInstance();
            director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

            draw(renderer, _modelViewTransform, flags);

            director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }
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
            std::sort(std::begin(_children), std::end(_children), nodeComparisonLess);

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

    SpriteFrame* SpriteFrame::create(const std::string& filename, const MATH::Rectf& rect)
    {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTextureFilename(filename, rect);
        spriteFrame->autorelease();

        return spriteFrame;
    }

    SpriteFrame* SpriteFrame::createWithTexture(Texture2D *texture, const MATH::Rectf& rect)
    {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTexture(texture, rect);
        spriteFrame->autorelease();

        return spriteFrame;
    }

    SpriteFrame* SpriteFrame::createWithTexture(Texture2D* texture, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize)
    {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTexture(texture, rect, rotated, offset, originalSize);
        spriteFrame->autorelease();

        return spriteFrame;
    }

    SpriteFrame* SpriteFrame::create(const std::string& filename, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize)
    {
        SpriteFrame *spriteFrame = new (std::nothrow) SpriteFrame();
        spriteFrame->initWithTextureFilename(filename, rect, rotated, offset, originalSize);
        spriteFrame->autorelease();

        return spriteFrame;
    }

    SpriteFrame::SpriteFrame()
    : _rotated(false)
    , _texture(nullptr)
    {

    }

    bool SpriteFrame::initWithTexture(Texture2D* texture, const MATH::Rectf& rect)
    {
        MATH::Rectf rectInPixels = CC_RECT_POINTS_TO_PIXELS(rect);
        return initWithTexture(texture, rectInPixels, false, MATH::Vec2fZERO, rectInPixels.size);
    }

    bool SpriteFrame::initWithTextureFilename(const std::string& filename, const MATH::Rectf& rect)
    {
        MATH::Rectf rectInPixels = CC_RECT_POINTS_TO_PIXELS( rect );
        return initWithTextureFilename(filename, rectInPixels, false, MATH::Vec2fZERO, rectInPixels.size);
    }

    bool SpriteFrame::initWithTexture(Texture2D* texture, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize)
    {
        _texture = texture;

        if (texture)
        {
            texture->retain();
        }

        _rectInPixels = rect;
        _rect = CC_RECT_PIXELS_TO_POINTS(rect);
        _offsetInPixels = offset;
        _offset = CC_POINT_PIXELS_TO_POINTS( _offsetInPixels );
        _originalSizeInPixels = originalSize;
        _originalSize = CC_SIZE_PIXELS_TO_POINTS( _originalSizeInPixels );
        _rotated = rotated;

        return true;
    }

    bool SpriteFrame::initWithTextureFilename(const std::string& filename, const MATH::Rectf& rect, bool rotated, const MATH::Vector2f& offset, const MATH::Sizef& originalSize)
    {
        _texture = nullptr;
        _textureFilename = filename;
        _rectInPixels = rect;
        _rect = CC_RECT_PIXELS_TO_POINTS( rect );
        _offsetInPixels = offset;
        _offset = CC_POINT_PIXELS_TO_POINTS( _offsetInPixels );
        _originalSizeInPixels = originalSize;
        _originalSize = CC_SIZE_PIXELS_TO_POINTS( _originalSizeInPixels );
        _rotated = rotated;

        return true;
    }

    SpriteFrame::~SpriteFrame()
    {
        SAFE_RELEASE(_texture);
    }

    void SpriteFrame::setRect(const MATH::Rectf& rect)
    {
        _rect = rect;
        _rectInPixels = CC_RECT_POINTS_TO_PIXELS(_rect);
    }

    void SpriteFrame::setRectInPixels(const MATH::Rectf& rectInPixels)
    {
        _rectInPixels = rectInPixels;
        _rect = CC_RECT_PIXELS_TO_POINTS(rectInPixels);
    }

    const MATH::Vector2f& SpriteFrame::getOffset() const
    {
        return _offset;
    }

    void SpriteFrame::setOffset(const MATH::Vector2f& offsets)
    {
        _offset = offsets;
        _offsetInPixels = CC_POINT_POINTS_TO_PIXELS( _offset );
    }

    const MATH::Vector2f& SpriteFrame::getOffsetInPixels() const
    {
        return _offsetInPixels;
    }

    void SpriteFrame::setOffsetInPixels(const MATH::Vector2f& offsetInPixels)
    {
        _offsetInPixels = offsetInPixels;
        _offset = CC_POINT_PIXELS_TO_POINTS( _offsetInPixels );
    }

    void SpriteFrame::setTexture(Texture2D * texture)
    {
        if( _texture != texture ) {
            SAFE_RELEASE(_texture);
            SAFE_RETAIN(texture);
            _texture = texture;
        }
    }

    Texture2D* SpriteFrame::getTexture()
    {
        if( _texture ) {
            return _texture;
        }

        if( _textureFilename.length() > 0 ) {
            return Director::getInstance()->getTextureCache()->addImage(_textureFilename.c_str());
        }
        // no texture or texture filename
        return nullptr;
    }

    static SpriteFrameCache *_sharedSpriteFrameCache = nullptr;

    SpriteFrameCache* SpriteFrameCache::getInstance()
    {
        if (! _sharedSpriteFrameCache)
        {
            _sharedSpriteFrameCache = new (std::nothrow) SpriteFrameCache();
            _sharedSpriteFrameCache->init();
        }

        return _sharedSpriteFrameCache;
    }

    void SpriteFrameCache::destroyInstance()
    {
        SAFE_RELEASE_NULL(_sharedSpriteFrameCache);
    }

    bool SpriteFrameCache::init()
    {
        _spriteFrames.reserve(20);
        _spriteFramesAliases.reserve(20);
        _loadedFileNames = new std::set<std::string>();
        return true;
    }

    SpriteFrameCache::~SpriteFrameCache()
    {
        SAFE_DELETE(_loadedFileNames);
    }

    void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist, Texture2D *texture)
    {
        if (_loadedFileNames->find(plist) != _loadedFileNames->end())
        {
            return; // We already added it
        }

        std::string fullPath = IO::FileUtils::getInstance().fullPathForFilename(plist);
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(fullPath);

        addSpriteFramesWithDictionary(dict, texture);
        _loadedFileNames->insert(plist);
    }

    void SpriteFrameCache::addSpriteFramesWithFileContent(const std::string& plist_content, Texture2D *texture)
    {
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromData((const HBYTE *)plist_content.c_str(), static_cast<int>(plist_content.size()));
        addSpriteFramesWithDictionary(dict, texture);
    }

    void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist, const std::string& textureFileName)
    {
        Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(textureFileName);

        if (texture)
        {
            addSpriteFramesWithFile(plist, texture);
        }
    }

    void SpriteFrameCache::addSpriteFramesWithFile(const std::string& plist)
    {
        std::string fullPath = IO::FileUtils::getInstance().fullPathForFilename(plist);
        if (fullPath.size() == 0)
        {
            return;
        }

        if (_loadedFileNames->find(plist) == _loadedFileNames->end())
        {

            ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(fullPath);

            std::string texturePath("");

            if (dict.find("metadata") != dict.end())
            {
                ValueMap& metadataDict = dict["metadata"].asValueMap();
                // try to read  texture file name from meta data
                texturePath = metadataDict["textureFileName"].asString();
            }

            if (!texturePath.empty())
            {
                // build texture path relative to plist file
                texturePath = IO::FileUtils::getInstance().fullPathFromRelativeFile(texturePath.c_str(), plist);
            }
            else
            {
                // build texture path by replacing file extension
                texturePath = plist;

                // remove .xxx
                size_t startPos = texturePath.find_last_of(".");
                texturePath = texturePath.erase(startPos);

                // append .png
                texturePath = texturePath.append(".png");
            }

            Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(texturePath.c_str());

            if (texture)
            {
                addSpriteFramesWithDictionary(dict, texture);
                _loadedFileNames->insert(plist);
            }
        }
    }

    void SpriteFrameCache::addSpriteFramesWithDictionary(ValueMap& dictionary, Texture2D* texture)
    {
        ValueMap& framesDict = dictionary["frames"].asValueMap();
        int format = 0;

        // get the format
        if (dictionary.find("metadata") != dictionary.end())
        {
            ValueMap& metadataDict = dictionary["metadata"].asValueMap();
            format = metadataDict["format"].asInt();
        }

        auto textureFileName = Director::getInstance()->getTextureCache()->getTextureFilePath(texture);
        auto image = new IMAGE::SmartImage();
        image->initWithImageFile(textureFileName);

        for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
        {
            ValueMap& frameDict = iter->second.asValueMap();
            std::string spriteFrameName = iter->first;
            SpriteFrame* spriteFrame = _spriteFrames.at(spriteFrameName);
            if (spriteFrame)
            {
                continue;
            }

            if (format == 0)
            {
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
            else if (format == 1 || format == 2)
            {
                MATH::Rectf frame = UTILS::STRING::RectFromString(frameDict["frame"].asString());
                bool rotated = false;

                // rotation
                if (format == 2)
                {
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
            else if (format == 3)
            {
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
                    _spriteFramesAliases[oneAlias] = HValue(spriteFrameName);
                }

                // create frame
                spriteFrame = SpriteFrame::createWithTexture(texture,
                    MATH::Rectf(textureRect.origin.x, textureRect.origin.y, spriteSize.width, spriteSize.height),
                    textureRotated,
                    spriteOffset,
                    spriteSourceSize);
            }

            // add sprite frame
            _spriteFrames.insert(spriteFrameName, spriteFrame);
        }
        SAFE_DELETE(image);
    }

    bool SpriteFrameCache::isSpriteFramesWithFileLoaded(const std::string& plist) const
    {
        bool result = false;

        if (_loadedFileNames->find(plist) != _loadedFileNames->end())
        {
            result = true;
        }

        return result;
    }

    void SpriteFrameCache::addSpriteFrame(SpriteFrame* frame, const std::string& frameName)
    {
        _spriteFrames.insert(frameName, frame);
    }

    void SpriteFrameCache::removeSpriteFrames()
    {
        _spriteFrames.clear();
        _spriteFramesAliases.clear();
        _loadedFileNames->clear();
    }

    void SpriteFrameCache::removeUnusedSpriteFrames()
    {
        bool removed = false;
        std::vector<std::string> toRemoveFrames;

        for (auto iter = _spriteFrames.begin(); iter != _spriteFrames.end(); ++iter)
        {
            SpriteFrame* spriteFrame = iter->second;
            if( spriteFrame->getReferenceCount() == 1 )
            {
                toRemoveFrames.push_back(iter->first);
                removed = true;
            }
        }

        _spriteFrames.erase(toRemoveFrames);

        // FIXME:. Since we don't know the .plist file that originated the frame, we must remove all .plist from the cache
        if( removed )
        {
            _loadedFileNames->clear();
        }
    }


    void SpriteFrameCache::removeSpriteFrameByName(const std::string& name)
    {
        // explicit nil handling
        if( !(name.size()>0) )
            return;

        // Is this an alias ?
        std::string key = _spriteFramesAliases[name].asString();

        if (!key.empty())
        {
            _spriteFrames.erase(key);
            _spriteFramesAliases.erase(key);
        }
        else
        {
            _spriteFrames.erase(name);
        }

        // FIXME:. Since we don't know the .plist file that originated the frame, we must remove all .plist from the cache
        _loadedFileNames->clear();
    }

    void SpriteFrameCache::removeSpriteFramesFromFile(const std::string& plist)
    {
        std::string fullPath = IO::FileUtils::getInstance().fullPathForFilename(plist);
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromFile(fullPath);
        if (dict.empty())
        {
            return;
        }
        removeSpriteFramesFromDictionary(dict);

        // remove it from the cache
        std::set<std::string>::iterator ret = _loadedFileNames->find(plist);
        if (ret != _loadedFileNames->end())
        {
            _loadedFileNames->erase(ret);
        }
    }

    void SpriteFrameCache::removeSpriteFramesFromFileContent(const std::string& plist_content)
    {
        ValueMap dict = IO::FileUtils::getInstance().getValueMapFromData((const HBYTE *)plist_content.data(), static_cast<int>(plist_content.size()));
        if (dict.empty())
        {
            return;
        }
        removeSpriteFramesFromDictionary(dict);
    }

    void SpriteFrameCache::removeSpriteFramesFromDictionary(ValueMap& dictionary)
    {
        ValueMap framesDict = dictionary["frames"].asValueMap();
        std::vector<std::string> keysToRemove;

        for (auto iter = framesDict.cbegin(); iter != framesDict.cend(); ++iter)
        {
            if (_spriteFrames.at(iter->first))
            {
                keysToRemove.push_back(iter->first);
            }
        }

        _spriteFrames.erase(keysToRemove);
    }

    void SpriteFrameCache::removeSpriteFramesFromTexture(Texture2D* texture)
    {
        std::vector<std::string> keysToRemove;

        for (auto iter = _spriteFrames.cbegin(); iter != _spriteFrames.cend(); ++iter)
        {
            std::string key = iter->first;
            SpriteFrame* frame = _spriteFrames.at(key);
            if (frame && (frame->getTexture() == texture))
            {
                keysToRemove.push_back(key);
            }
        }

        _spriteFrames.erase(keysToRemove);
    }

    SpriteFrame* SpriteFrameCache::getSpriteFrameByName(const std::string& name)
    {
        SpriteFrame* frame = _spriteFrames.at(name);
        if (!frame)
        {
            // try alias dictionary
            std::string key = _spriteFramesAliases[name].asString();
            if (!key.empty())
            {
                frame = _spriteFrames.at(key);
            }
        }
        return frame;
    }
}
