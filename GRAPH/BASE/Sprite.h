#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include "GRAPH/BASE/Node.h"
#include "GRAPH/BASE/Protocols.h"
#include "GRAPH/RENDERER/TextureAtlas.h"
#include "GRAPH/RENDERER/RenderCommand.h"
#include "MATH/Vector.h"
#include "MATH/Matrix.h"
#include "MATH/Rectangle.h"
#include "MATH/Size.h"

namespace GRAPH
{
    class PolygonInfo
    {
    public:
        PolygonInfo():
        isVertsOwner(true),
        rect(MATH::RectfZERO),
        filename("")
        {
            triangles.verts = nullptr;
            triangles.indices = nullptr;
            triangles.vertCount = 0;
            triangles.indexCount = 0;
        }

        PolygonInfo(const PolygonInfo& other);
        PolygonInfo& operator= (const PolygonInfo &other);
        ~PolygonInfo();

        void setQuad(V3F_C4B_T2F_Quad *quad);

        const unsigned int getVertCount() const;

        const unsigned int getTriaglesCount() const;

        const float getArea() const;

        MATH::Rectf rect;
        std::string filename;
        TrianglesCommand::Triangles triangles;

    protected:
        bool isVertsOwner;

    private:
        void releaseVertsAndIndices();
    };

    class SpriteBatchNode;

    class Sprite : public Node, public TextureProtocol
    {
    public:
         /** Sprite invalid index on the SpriteBatchNode. */
        static const int INDEX_NOT_INITIALIZED = -1;

        static Sprite* create();
        static Sprite* create(const std::string& filename);
        static Sprite* create(const PolygonInfo& info);
        static Sprite* create(const std::string& filename, const MATH::Rectf& rect);
        static Sprite* createWithTexture(Texture2D *texture);
        static Sprite* createWithTexture(Texture2D *texture, const MATH::Rectf& rect, bool rotated=false);

        virtual void updateTransform() override;

        virtual SpriteBatchNode* getBatchNode() const;
        virtual void setBatchNode(SpriteBatchNode *spriteBatchNode);

        virtual void setTexture(const std::string &filename );
        virtual void setTexture(Texture2D *texture) override;
        virtual Texture2D* getTexture() const override;

        virtual void setTextureRect(const MATH::Rectf& rect);
        virtual void setTextureRect(const MATH::Rectf& rect, bool rotated, const MATH::Sizef& untrimmedSize);

        virtual void setVertexRect(const MATH::Rectf& rect);

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
        virtual bool initWithFile(const std::string& filename);
        virtual bool initWithFile(const std::string& filename, const MATH::Rectf& rect);

        void debugDraw(bool on);

        PolygonInfo getPolygonInfo() const;
        void setPolygonInfo(const PolygonInfo& info);

    protected:
        void updateColor() override;
        virtual void setTextureCoords(MATH::Rectf rect);
        virtual void updateBlendFunc();
        virtual void setReorderChildDirtyRecursively();
        virtual void setDirtyRecursively(bool value);

        TextureAtlas*       _textureAtlas;      /// SpriteBatchNode texture atlas (weak reference)
        ssize_t             _atlasIndex;        /// Absolute (real) Index on the SpriteSheet
        SpriteBatchNode*    _batchNode;         /// Used batch node (weak reference)

        bool                _dirty;             /// Whether the sprite needs to be updated
        bool                _recursiveDirty;    /// Whether all of the sprite's children needs to be updated
        bool                _shouldBeHidden;    /// should not be drawn because one of the ancestors is not visible
        MATH::Matrix4              _transformToBatch;

        BlendFunc        _blendFunc;            /// It's required for TextureProtocol inheritance
        Texture2D*       _texture;              /// Texture2D object that is used to render the sprite
        TrianglesCommand _trianglesCommand;     ///

        MATH::Rectf _rect;                             /// Retangle of Texture2D
        bool   _rectRotated;                    /// Whether the texture is rotated

        // Offset Position (used by Zwoptex)
        MATH::Vector2f _offsetPosition;
        MATH::Vector2f _unflippedOffsetPositionFromCenter;

        // vertex coords, texture coords and color info
        V3F_C4B_T2F_Quad _quad;
        PolygonInfo  _polyInfo;

        // opacity and RGB protocol
        bool _opacityModifyRGB;

        // image is flipped
        bool _flippedX;                         /// Whether the sprite is flipped horizontally or not
        bool _flippedY;                         /// Whether the sprite is flipped vertically or not

        bool _insideBounds;                     /// whether or not the sprite was inside bounds the previous frame
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
        inline void setTextureAtlas(TextureAtlas* textureAtlas)
        {
            if (textureAtlas != _textureAtlas)
            {
                SAFE_RETAIN(textureAtlas);
                SAFE_RELEASE(_textureAtlas);
                _textureAtlas = textureAtlas;
            }
        }

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
        BatchCommand _batchCommand;     // render command

        // all descendants: children, grand children, etc...
        // There is not need to retain/release these objects, since they are already retained by _children
        // So, using std::vector<Sprite*> is slightly faster than using cocos2d::Array for this particular case
        std::vector<Sprite*> _descendants;
    };
}

#endif // SPRITE_H
