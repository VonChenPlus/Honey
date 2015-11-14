#ifndef UISCALE9SPRITE_H
#define UISCALE9SPRITE_H

#include "GRAPH/Node.h"
#include "GRAPH/Protocols.h"
#include "GRAPH/Sprite.h"

namespace GRAPH
{
    namespace UI
    {
        class Scale9Sprite : public Node , public BlendProtocol
        {
        public:
            Scale9Sprite();
            virtual ~Scale9Sprite();

            enum class State
            {
                NORMAL,
                GRAY
            };

        public:
            static Scale9Sprite* create();

            static Scale9Sprite* create(const std::string& file, const MATH::Rectf& rect,  const MATH::Rectf& capInsets);
            static Scale9Sprite* create(const MATH::Rectf& capInsets, const std::string& file);
            static Scale9Sprite* create(const std::string& file, const MATH::Rectf& rect);
            static Scale9Sprite* create(const std::string& file);

            virtual bool initWithFile(const std::string& file, const MATH::Rectf& rect,  const MATH::Rectf& capInsets);
            virtual bool initWithFile(const std::string& file, const MATH::Rectf& rect);
            virtual bool initWithFile(const MATH::Rectf& capInsets, const std::string& file);
            virtual bool initWithFile(const std::string& file);

            virtual bool initWithSpriteFrame(SpriteFrame* spriteFrame, const MATH::Rectf& capInsets);
            virtual bool initWithSpriteFrame(SpriteFrame* spriteFrame);
            virtual bool initWithSpriteFrameName(const std::string& spriteFrameName, const MATH::Rectf& capInsets);
            virtual bool initWithSpriteFrameName(const std::string& spriteFrameName);

            //override function
            virtual bool init() override;
            virtual bool init(Sprite* sprite, const MATH::Rectf& rect, bool rotated, const MATH::Rectf& capInsets);
            virtual bool init(Sprite* sprite, const MATH::Rectf& rect, const MATH::Rectf& capInsets);
            virtual bool init(Sprite* sprite,
                              const MATH::Rectf& rect,
                              bool rotated,
                              const MATH::Vector2f &offset,
                              const MATH::Sizef &originalSize,
                              const MATH::Rectf& capInsets);

            virtual void setBlendFunc(const BlendFunc &blendFunc) override;
            virtual const BlendFunc &getBlendFunc() const override;

            Scale9Sprite* resizableSpriteWithCapInsets(const MATH::Rectf& capInsets) const;
            virtual bool updateWithSprite(Sprite* sprite,
                                          const MATH::Rectf& rect,
                                          bool rotated,
                                          const MATH::Rectf& capInsets);

            virtual bool updateWithSprite(Sprite* sprite,
                                          const MATH::Rectf& rect,
                                          bool rotated,
                                          const MATH::Vector2f &offset,
                                          const MATH::Sizef &originalSize,
                                          const MATH::Rectf& capInsets);


            // overrides
            virtual void setContentSize(const MATH::Sizef & size) override;
            virtual void setAnchorPoint(const MATH::Vector2f& anchorPoint) override;

            void setState(State state);

            MATH::Sizef getOriginalSize() const;

            void setPreferredSize(const MATH::Sizef& size);

            MATH::Sizef getPreferredSize() const;

            void setCapInsets(const MATH::Rectf& rect);
            MATH::Rectf getCapInsets()const;

            void setInsetLeft(float leftInset);
            float getInsetLeft()const;

            void setInsetTop(float topInset);
            float getInsetTop()const;

            void setInsetRight(float rightInset);
            float getInsetRight()const;

            void setInsetBottom(float bottomInset);
            float getInsetBottom()const;

            void setScale9Enabled(bool enabled);
            bool isScale9Enabled()const;

            virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;
            virtual void cleanup() override;

            virtual void onEnter() override;
            virtual void onEnterTransitionDidFinish() override;
            virtual void onExit() override;
            virtual void onExitTransitionDidStart() override;

            virtual void updateDisplayedOpacity(uint8 parentOpacity) override;
            virtual void updateDisplayedColor(const Color3B& parentColor) override;
            virtual void disableCascadeColor() override;
            virtual void disableCascadeOpacity() override;

            Sprite* getSprite()const;

            virtual void setFlippedX(bool flippedX);
            virtual bool isFlippedX()const;
            virtual void setFlippedY(bool flippedY);
            virtual bool isFlippedY()const;

            //override the setScale function of Node
            virtual void setScaleX(float scaleX) override;
            virtual void setScaleY(float scaleY) override;
            virtual void setScale(float scale) override;
            virtual void setScale(float scaleX, float scaleY) override;
            using Node::setScaleZ;
            virtual float getScaleX() const override;
            virtual float getScaleY() const override;
            virtual float getScale() const override;
            using Node::getScaleZ;
            virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

        protected:
            void updateCapInset();
            void updatePositions();
            void createSlicedSprites();
            void cleanupSlicedSprites();
            void adjustScale9ImagePosition();
            void applyBlendFunc();
            void updateBlendFunc(Unity3DTexture *texture);

            virtual void sortAllProtectedChildren();

            bool _spritesGenerated;
            MATH::Rectf _spriteRect;
            bool   _spriteFrameRotated;
            MATH::Rectf _capInsetsInternal;
            bool _positionsAreDirty;

            Sprite* _scale9Image; //the original sprite
            Sprite* _topLeftSprite;
            Sprite* _topSprite;
            Sprite* _topRightSprite;
            Sprite* _leftSprite;
            Sprite* _centerSprite;
            Sprite* _rightSprite;
            Sprite* _bottomLeftSprite;
            Sprite* _bottomSprite;
            Sprite* _bottomRightSprite;

            bool _scale9Enabled;
            BlendFunc _blendFunc;

            MATH::Sizef _topLeftSize;
            MATH::Sizef _centerSize;
            MATH::Sizef _bottomRightSize;
            MATH::Vector2f _centerOffset;

            MATH::Sizef _originalSize;
            MATH::Vector2f _offset;
            MATH::Sizef _preferredSize;
            MATH::Rectf _capInsets;
            float _insetLeft;
            float _insetTop;
            float _insetRight;
            float _insetBottom;

            void addProtectedChild(Node* child);

            HObjectVector<Node*> _protectedChildren;        ///holds the 9 sprites
            bool _reorderProtectedChildDirty;

            bool _flippedX;
            bool _flippedY;
            bool _isPatch9;
        };
    }
}

#endif // UISCALE9SPRITE_H
