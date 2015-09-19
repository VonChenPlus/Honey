#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include "GRAPH/Node.h"
#include "GRAPH/Protocols.h"
#include "GRAPH/RENDERER/RenderCommand.h"

namespace GRAPH
{
    class PolygonInfo
    {
    public:
        PolygonInfo();
        PolygonInfo(const PolygonInfo& other);
        PolygonInfo& operator= (const PolygonInfo &other);
        ~PolygonInfo();

        void setQuad(V3F_C4B_T2F_Quad *quad);

        const unsigned int getVertCount() const;
        const unsigned int getTriaglesCount() const;
        const float getArea() const;

        MATH::Rectf rect;
        TrianglesCommand::Triangles triangles;

    protected:
        bool isVertsOwner;

    private:
        void releaseVertsAndIndices();
    };

    class SpriteBatchNode;
    class SpriteFrame;
    class TextureAtlas;

    class Sprite : public Node, public TextureProtocol
    {
    public:
        static const int INDEX_NOT_INITIALIZED = -1;

        static Sprite* create();
        static Sprite* create(const std::string& filename);
        static Sprite* create(const PolygonInfo& info);
        static Sprite* create(const std::string& filename, const MATH::Rectf& rect);
        static Sprite* createWithTexture(Texture2D *texture);
        static Sprite* createWithTexture(Texture2D *texture, const MATH::Rectf& rect, bool rotated=false);
        static Sprite* createWithSpriteFrame(SpriteFrame *spriteFrame);
        static Sprite* createWithSpriteFrameName(const std::string& spriteFrameName);

        virtual void updateTransform() override;

        virtual SpriteBatchNode* getBatchNode() const;
        virtual void setBatchNode(SpriteBatchNode *spriteBatchNode);

        virtual void setTexture(const std::string &filename );
        virtual void setTexture(Texture2D *texture) override;
        virtual Texture2D* getTexture() const override;

        virtual void setTextureRect(const MATH::Rectf& rect);
        virtual void setTextureRect(const MATH::Rectf& rect, bool rotated, const MATH::Sizef& untrimmedSize);

        virtual void setVertexRect(const MATH::Rectf& rect);

        virtual void setSpriteFrame(const std::string &spriteFrameName);
        virtual void setSpriteFrame(SpriteFrame* newFrame);

        virtual bool isFrameDisplayed(SpriteFrame *frame) const;

        virtual SpriteFrame* getSpriteFrame() const;

        virtual bool isDirty() const { return _dirty; }

        virtual void setDirty(bool dirty) { _dirty = dirty; }

        inline V3F_C4B_T2F_Quad getQuad() const { return _quad; }

        inline bool isTextureRectRotated() const { return _rectRotated; }

        inline ssize_t getAtlasIndex() const { return _atlasIndex; }
        inline void setAtlasIndex(ssize_t atlasIndex) { _atlasIndex = atlasIndex; }

        inline const MATH::Rectf& getTextureRect() const { return _rect; }

        inline TextureAtlas* getTextureAtlas() const { return _textureAtlas; }
        inline void setTextureAtlas(TextureAtlas *textureAtlas) { _textureAtlas = textureAtlas; }

        inline const MATH::Vector2f& getOffsetPosition() const { return _offsetPosition; }

        bool isFlippedX() const;
        void setFlippedX(bool flippedX);

        bool isFlippedY() const;
        void setFlippedY(bool flippedY);

        inline void setBlendFunc(const BlendFunc &blendFunc) override { _blendFunc = blendFunc; }
        inline const BlendFunc& getBlendFunc() const override { return _blendFunc; }

        virtual void setScaleX(float scaleX) override;
        virtual void setScaleY(float scaleY) override;
        virtual void setScale(float scaleX, float scaleY) override;
        virtual void setPosition(const MATH::Vector2f& pos) override;
        virtual void setPosition(float x, float y) override;
        virtual void setRotation(float rotation) override;
        virtual void setRotationSkewX(float rotationX) override;
        virtual void setRotationSkewY(float rotationY) override;
        virtual void setSkewX(float sx) override;
        virtual void setSkewY(float sy) override;
        virtual void removeChild(Node* child, bool cleanup) override;
        virtual void removeAllChildrenWithCleanup(bool cleanup) override;
        virtual void reorderChild(Node *child, int zOrder) override;
        using Node::addChild;
        virtual void addChild(Node *child, int zOrder, int tag) override;
        virtual void addChild(Node *child, int zOrder, const std::string &name) override;
        virtual void sortAllChildren() override;
        virtual void setScale(float scale) override;
        virtual void setPositionZ(float positionZ) override;
        virtual void setAnchorPoint(const MATH::Vector2f& anchor) override;
        virtual void ignoreAnchorPointForPosition(bool value) override;
        virtual void setVisible(bool bVisible) override;
        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;
        virtual void setOpacityModifyRGB(bool modify) override;
        virtual bool isOpacityModifyRGB() const override;

    public:
        Sprite();
        virtual ~Sprite();

        virtual bool init() override;
        virtual bool initWithTexture(Texture2D *texture);
        virtual bool initWithPolygon(const PolygonInfo& info);
        virtual bool initWithTexture(Texture2D *texture, const MATH::Rectf& rect);
        virtual bool initWithTexture(Texture2D *texture, const MATH::Rectf& rect, bool rotated);
        virtual bool initWithSpriteFrame(SpriteFrame *spriteFrame);
        virtual bool initWithSpriteFrameName(const std::string& spriteFrameName);
        virtual bool initWithFile(const std::string& filename);
        virtual bool initWithFile(const std::string& filename, const MATH::Rectf& rect);

        PolygonInfo getPolygonInfo() const;
        void setPolygonInfo(const PolygonInfo& info);

    protected:
        void updateColor() override;
        virtual void setTextureCoords(MATH::Rectf rect);
        virtual void updateBlendFunc();
        virtual void setReorderChildDirtyRecursively();
        virtual void setDirtyRecursively(bool value);

        TextureAtlas*       _textureAtlas;
        ssize_t             _atlasIndex;
        SpriteBatchNode*    _batchNode;

        bool                _dirty;
        bool                _recursiveDirty;
        bool                _shouldBeHidden;
        MATH::Matrix4       _transformToBatch;

        BlendFunc        _blendFunc;
        Texture2D*       _texture;
        SpriteFrame*     _spriteFrame;
        TrianglesCommand _trianglesCommand;

        MATH::Rectf _rect;
        bool   _rectRotated;

        MATH::Vector2f _offsetPosition;
        MATH::Vector2f _unflippedOffsetPositionFromCenter;

        V3F_C4B_T2F_Quad _quad;
        PolygonInfo  _polyInfo;

        bool _opacityModifyRGB;

        bool _flippedX;
        bool _flippedY;

        bool _insideBounds;
    private:
        DISALLOW_COPY_AND_ASSIGN(Sprite)
    };

    class SpriteBatchNode : public Node, public TextureProtocol
    {
        static const int DEFAULT_CAPACITY = 29;

    public:
        static SpriteBatchNode* createWithTexture(Texture2D* tex, ssize_t capacity = DEFAULT_CAPACITY);
        static SpriteBatchNode* create(const std::string& fileImage, ssize_t capacity = DEFAULT_CAPACITY);

        inline TextureAtlas* getTextureAtlas() { return _textureAtlas; }
        void setTextureAtlas(TextureAtlas* textureAtlas);

        inline const std::vector<Sprite*>& getDescendants() const { return _descendants; }

        void increaseAtlasCapacity();
        void removeChildAtIndex(ssize_t index, bool doCleanup);
        void appendChild(Sprite* sprite);
        void removeSpriteFromAtlas(Sprite *sprite);

        ssize_t rebuildIndexInOrder(Sprite *parent, ssize_t index);
        ssize_t highestAtlasIndexInChild(Sprite *sprite);
        ssize_t lowestAtlasIndexInChild(Sprite *sprite);
        ssize_t atlasIndexForChild(Sprite *sprite, int z);
        void reorderBatch(bool reorder);

        virtual Texture2D* getTexture() const override;
        virtual void setTexture(Texture2D *texture) override;

        virtual void setBlendFunc(const BlendFunc &blendFunc) override;
        virtual const BlendFunc& getBlendFunc() const override;

        virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;

        virtual void addChild(Node * child, int zOrder, int tag) override;
        virtual void addChild(Node * child, int zOrder, const std::string &name) override;
        virtual void reorderChild(Node *child, int zOrder) override;
        virtual void removeChild(Node *child, bool cleanup) override;

        virtual void removeAllChildrenWithCleanup(bool cleanup) override;
        virtual void sortAllChildren() override;

        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

        void insertQuadFromSprite(Sprite *sprite, ssize_t index);
        SpriteBatchNode * addSpriteWithoutQuad(Sprite *child, int z, int aTag);

    public:
        SpriteBatchNode();
        virtual ~SpriteBatchNode();


        bool initWithTexture(Texture2D *tex, ssize_t capacity = DEFAULT_CAPACITY);
        bool initWithFile(const std::string& fileImage, ssize_t capacity = DEFAULT_CAPACITY);
        bool init() override;

    protected:
        void updateQuadFromSprite(Sprite *sprite, ssize_t index);

        void updateAtlasIndex(Sprite* sprite, ssize_t* curIndex);
        void swap(ssize_t oldIndex, ssize_t newIndex);
        void updateBlendFunc();

        TextureAtlas *_textureAtlas;
        BlendFunc _blendFunc;
        BatchCommand _batchCommand;
        std::vector<Sprite*> _descendants;
    };
}

#endif // SPRITE_H
