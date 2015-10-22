#ifndef SPRITE_H
#define SPRITE_H

#include <string>
#include "GRAPH/Node.h"
#include "GRAPH/Protocols.h"
#include "GRAPH/UNITY3D/RenderCommand.h"

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
    class GLTextureAtlas;

    class Sprite : public Node, public TextureProtocol
    {
    public:
        static const int INDEX_NOT_INITIALIZED = -1;

        static Sprite* create();
        static Sprite* create(const std::string& filename);
        static Sprite* create(const PolygonInfo& info);
        static Sprite* create(const std::string& filename, const MATH::Rectf& rect);
        static Sprite* createWithTexture(GLTexture *texture);
        static Sprite* createWithTexture(GLTexture *texture, const MATH::Rectf& rect, bool rotated=false);
        static Sprite* createWithSpriteFrame(SpriteFrame *spriteFrame);
        static Sprite* createWithSpriteFrameName(const std::string& spriteFrameName);

        virtual void updateTransform() override;

        virtual SpriteBatchNode* getBatchNode() const;
        virtual void setBatchNode(SpriteBatchNode *spriteBatchNode);

        virtual void setTexture(const std::string &filename );
        virtual void setTexture(GLTexture *texture) override;
        virtual GLTexture* getTexture() const override;

        virtual void setTextureRect(const MATH::Rectf& rect);
        virtual void setTextureRect(const MATH::Rectf& rect, bool rotated, const MATH::Sizef& untrimmedSize);
        virtual void setVertexRect(const MATH::Rectf& rect);
        virtual void setSpriteFrame(const std::string &spriteFrameName);
        virtual void setSpriteFrame(SpriteFrame* newFrame);

        virtual bool isFrameDisplayed(SpriteFrame *frame) const;

        virtual SpriteFrame* getSpriteFrame() const;

        virtual bool isDirty() const { return dirty_; }
        virtual void setDirty(bool dirty) { dirty_ = dirty; }

        inline V3F_C4B_T2F_Quad getQuad() const { return quad_; }

        inline bool isTextureRectRotated() const { return rectRotated_; }

        inline uint64 getAtlasIndex() const { return atlasIndex_; }
        inline void setAtlasIndex(uint64 atlasIndex) { atlasIndex_ = atlasIndex; }

        inline const MATH::Rectf& getTextureRect() const { return rect_; }

        inline GLTextureAtlas* getTextureAtlas() const { return textureAtlas_; }
        inline void setTextureAtlas(GLTextureAtlas *textureAtlas) { textureAtlas_ = textureAtlas; }

        inline const MATH::Vector2f& getOffsetPosition() const { return offsetPosition_; }

        bool isFlippedX() const;
        void setFlippedX(bool flippedX);

        bool isFlippedY() const;
        void setFlippedY(bool flippedY);

        inline void setBlendFunc(const BlendFunc &blendFunc) override { blendFunc_ = blendFunc; }
        inline const BlendFunc& getBlendFunc() const override { return blendFunc_; }

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
        virtual void setVisible(bool bVisible) override;
        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;
        virtual void setOpacityModifyRGB(bool modify) override;
        virtual bool isOpacityModifyRGB() const override;

    public:
        Sprite();
        virtual ~Sprite();

        virtual bool init() override;
        virtual bool initWithTexture(GLTexture *texture);
        virtual bool initWithPolygon(const PolygonInfo& info);
        virtual bool initWithTexture(GLTexture *texture, const MATH::Rectf& rect);
        virtual bool initWithTexture(GLTexture *texture, const MATH::Rectf& rect, bool rotated);
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

        GLTextureAtlas*       textureAtlas_;
        uint64             atlasIndex_;
        SpriteBatchNode*    batchNode_;
        bool                dirty_;
        bool                recursiveDirty_;
        bool                shouldBeHidden_;
        MATH::Matrix4       transformToBatch_;
        BlendFunc        blendFunc_;
        GLTexture*       texture_;
        SpriteFrame*     spriteFrame_;
        TrianglesCommand trianglesCommand_;
        MATH::Rectf rect_;
        bool   rectRotated_;
        MATH::Vector2f offsetPosition_;
        MATH::Vector2f unflippedOffsetPositionFromCenter_;
        V3F_C4B_T2F_Quad quad_;
        PolygonInfo  polyInfo_;
        bool opacityModifyRGB_;
        bool flippedX_;
        bool flippedY_;

        DISALLOW_COPY_AND_ASSIGN(Sprite)
    };

    class SpriteBatchNode : public Node, public TextureProtocol
    {
        static const int DEFAULT_CAPACITY = 29;

    public:
        static SpriteBatchNode* createWithTexture(GLTexture* tex, uint64 capacity = DEFAULT_CAPACITY);
        static SpriteBatchNode* create(const std::string& fileImage, uint64 capacity = DEFAULT_CAPACITY);

        inline GLTextureAtlas* getTextureAtlas() { return textureAtlas_; }
        void setTextureAtlas(GLTextureAtlas* textureAtlas);

        inline const std::vector<Sprite*>& getDescendants() const { return descendants_; }

        void increaseAtlasCapacity();
        void removeChildAtIndex(uint64 index, bool doCleanup);
        void appendChild(Sprite* sprite);
        void removeSpriteFromAtlas(Sprite *sprite);

        uint64 rebuildIndexInOrder(Sprite *parent, uint64 index);
        uint64 highestAtlasIndexInChild(Sprite *sprite);
        uint64 lowestAtlasIndexInChild(Sprite *sprite);
        uint64 atlasIndexForChild(Sprite *sprite, int z);
        void reorderBatch(bool reorder);

        virtual GLTexture* getTexture() const override;
        virtual void setTexture(GLTexture *texture) override;

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

        void insertQuadFromSprite(Sprite *sprite, uint64 index);
        SpriteBatchNode * addSpriteWithoutQuad(Sprite *child, int z, int aTag);

    public:
        SpriteBatchNode();
        virtual ~SpriteBatchNode();


        bool initWithTexture(GLTexture *tex, uint64 capacity = DEFAULT_CAPACITY);
        bool initWithFile(const std::string& fileImage, uint64 capacity = DEFAULT_CAPACITY);
        bool init() override;

    protected:
        void updateQuadFromSprite(Sprite *sprite, uint64 index);

        void updateAtlasIndex(Sprite* sprite, uint64* curIndex);
        void swap(uint64 oldIndex, uint64 newIndex);
        void updateBlendFunc();

        GLTextureAtlas *textureAtlas_;
        BlendFunc blendFunc_;
        BatchCommand batchCommand_;
        std::vector<Sprite*> descendants_;
    };
}

#endif // SPRITE_H
