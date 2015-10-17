#include "GRAPH/UI/UIScale9Sprite.h"
#include "MATH/AffineTransform.h"
#include "GRAPH/Director.h"
#include "GRAPH/SpriteFrame.h"
#include "GRAPH/UNITY3D/GLShader.h"
#include "GRAPH/UNITY3D/GLShaderState.h"
#include "GRAPH/UNITY3D/GLTexture.h"

namespace GRAPH
{
    namespace UI
    {
        Scale9Sprite::Scale9Sprite()
            : _spritesGenerated(false)
            , _spriteFrameRotated(false)
            , _positionsAreDirty(true)
            , _scale9Image(nullptr)
            , _topLeftSprite(nullptr)
            , _topSprite(nullptr)
            , _topRightSprite(nullptr)
            , _leftSprite(nullptr)
            , _centerSprite(nullptr)
            , _rightSprite(nullptr)
            , _bottomLeftSprite(nullptr)
            , _bottomSprite(nullptr)
            , _bottomRightSprite(nullptr)
            , _scale9Enabled(true)
            , _insetLeft(0)
            , _insetTop(0)
            , _insetRight(0)
            , _insetBottom(0)
            ,_flippedX(false)
            ,_flippedY(false)
            ,_isPatch9(false)

        {
            this->setAnchorPoint(MATH::Vector2f(0.5,0.5));
        }

        Scale9Sprite::~Scale9Sprite()
        {
            this->cleanupSlicedSprites();
            SAFE_RELEASE(_scale9Image);
        }

        bool Scale9Sprite::initWithFile(const MATH::Rectf& capInsets, const std::string& file)
        {
            bool pReturn = this->initWithFile(file, MATH::RectfZERO, capInsets);
            return pReturn;
        }

        bool Scale9Sprite::initWithFile(const std::string& file)
        {
            bool pReturn = this->initWithFile(file, MATH::RectfZERO);
            return pReturn;
        }

        bool Scale9Sprite::initWithSpriteFrame(SpriteFrame* spriteFrame,
                                                   const MATH::Rectf& capInsets)
        {
            Sprite *sprite = Sprite::createWithSpriteFrame(spriteFrame);
            bool pReturn = this->init(sprite,
                                      spriteFrame->getRect(),
                                      spriteFrame->isRotated(),
                                      spriteFrame->getOffset(),
                                      spriteFrame->getOriginalSize(),
                                      capInsets);
            return pReturn;
        }

        bool Scale9Sprite::initWithSpriteFrame(SpriteFrame* spriteFrame)
        {
            bool pReturn = this->initWithSpriteFrame(spriteFrame, MATH::RectfZERO);
            return pReturn;
        }

        bool Scale9Sprite::initWithSpriteFrameName(const std::string& spriteFrameName,
                                                   const MATH::Rectf& capInsets)
        {
            SpriteFrame *frame = SpriteFrameCache::getInstance().getSpriteFrameByName(spriteFrameName);
            if (nullptr == frame) return false;
            bool pReturn = this->initWithSpriteFrame(frame, capInsets);
            return pReturn;
        }

        bool Scale9Sprite::initWithSpriteFrameName(const std::string& spriteFrameName)
        {
            bool pReturn = this->initWithSpriteFrameName(spriteFrameName, MATH::RectfZERO);
            return pReturn;
        }

        bool Scale9Sprite::init()
        {
            return this->init(NULL, MATH::RectfZERO, MATH::RectfZERO);
        }

        bool Scale9Sprite::init(Sprite* sprite, const MATH::Rectf& rect, const MATH::Rectf& capInsets)
        {
            return this->init(sprite, rect, false, capInsets);
        }

        bool Scale9Sprite::init(Sprite* sprite,
                                const MATH::Rectf& rect,
                                bool rotated,
                                const MATH::Rectf& capInsets)
        {
            return init(sprite, rect, rotated, MATH::Vec2fZERO, rect.size, capInsets);
        }

        bool Scale9Sprite::init(Sprite* sprite,
                                const MATH::Rectf& rect,
                                bool rotated,
                                const MATH::Vector2f &offset,
                                const MATH::Sizef &originalSize,
                                const MATH::Rectf& capInsets)
        {
            if(sprite)
            {
                this->updateWithSprite(sprite,
                                       rect,
                                       rotated,
                                       offset,
                                       originalSize,
                                       capInsets);
            }

            return true;
        }

        bool Scale9Sprite::initWithFile(const std::string& file,
                                        const MATH::Rectf& rect,
                                        const MATH::Rectf& capInsets)
        {
            Sprite *sprite = nullptr;
            sprite = Sprite::create(file);
            bool pReturn = this->init(sprite, rect, capInsets);
            return pReturn;
        }

        bool Scale9Sprite::initWithFile(const std::string& file, const MATH::Rectf& rect)
        {
            bool pReturn = this->initWithFile(file, rect, MATH::RectfZERO);
            return pReturn;
        }

        Scale9Sprite* Scale9Sprite::create()
        {
            Scale9Sprite *pReturn = new (std::nothrow) Scale9Sprite();
            if (pReturn && pReturn->init())
            {
                pReturn->autorelease();
                return pReturn;
            }
            SAFE_DELETE(pReturn);
            return NULL;
        }

        Scale9Sprite* Scale9Sprite::create(const std::string& file,
                                           const MATH::Rectf& rect,
                                           const MATH::Rectf& capInsets)
        {
            Scale9Sprite* pReturn = new (std::nothrow) Scale9Sprite();
            if ( pReturn && pReturn->initWithFile(file, rect, capInsets) )
            {
                pReturn->autorelease();
                return pReturn;
            }
            SAFE_DELETE(pReturn);
            return NULL;
        }


        Scale9Sprite* Scale9Sprite::create(const std::string& file, const MATH::Rectf& rect)
        {
            Scale9Sprite* pReturn = new (std::nothrow) Scale9Sprite();
            if ( pReturn && pReturn->initWithFile(file, rect) )
            {
                pReturn->autorelease();
                return pReturn;
            }
            SAFE_DELETE(pReturn);
            return NULL;
        }



        Scale9Sprite* Scale9Sprite::create(const MATH::Rectf& capInsets,
                                           const std::string& file)
        {
            Scale9Sprite* pReturn = new (std::nothrow) Scale9Sprite();
            if ( pReturn && pReturn->initWithFile(capInsets, file) )
            {
                pReturn->autorelease();
                return pReturn;
            }
            SAFE_DELETE(pReturn);
            return NULL;
        }


        Scale9Sprite* Scale9Sprite::create(const std::string& file)
        {
            Scale9Sprite* pReturn = new (std::nothrow) Scale9Sprite();
            if ( pReturn && pReturn->initWithFile(file) )
            {
                pReturn->autorelease();
                return pReturn;
            }
            SAFE_DELETE(pReturn);
            return NULL;
        }

        void Scale9Sprite::cleanupSlicedSprites()
        {
            if (_topLeftSprite && _topLeftSprite->isRunning())
            {
                _topLeftSprite->onExit();
            }
            if (_topSprite && _topSprite->isRunning())
            {
                _topSprite->onExit();
            }
            if (_topRightSprite && _topRightSprite->isRunning())
            {
                _topRightSprite->onExit();
            }

            if (_leftSprite && _leftSprite->isRunning())
            {
                _leftSprite->onExit();
            }

            if (_centerSprite && _centerSprite->isRunning())
            {
                _centerSprite->onExit();
            }

            if (_rightSprite && _rightSprite->isRunning())
            {
                _rightSprite->onExit();
            }

            if (_bottomLeftSprite && _bottomLeftSprite->isRunning())
            {
                _bottomLeftSprite->onExit();
            }

            if (_bottomRightSprite && _bottomRightSprite->isRunning())
            {
                _bottomRightSprite->onExit();
            }

            if (_bottomSprite && _bottomSprite->isRunning())
            {
                _bottomSprite->onExit();
            }

            SAFE_RELEASE_NULL(_topLeftSprite);
            SAFE_RELEASE_NULL(_topSprite);
            SAFE_RELEASE_NULL(_topRightSprite);
            SAFE_RELEASE_NULL(_leftSprite);
            SAFE_RELEASE_NULL(_centerSprite);
            SAFE_RELEASE_NULL(_rightSprite);
            SAFE_RELEASE_NULL(_bottomLeftSprite);
            SAFE_RELEASE_NULL(_bottomSprite);
            SAFE_RELEASE_NULL(_bottomRightSprite);
        }


        void Scale9Sprite::setBlendFunc(const BlendFunc &blendFunc)
        {
            _blendFunc = blendFunc;
            applyBlendFunc();
        }
        const BlendFunc &Scale9Sprite::getBlendFunc() const
        {
            return _blendFunc;
        }

        void Scale9Sprite::updateBlendFunc(GLTexture *texture)
        {

            // it is possible to have an untextured sprite
            if (! texture || ! texture->hasPremultipliedAlpha())
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

        void Scale9Sprite::applyBlendFunc()
        {
            if(_scale9Image)
                _scale9Image->setBlendFunc(_blendFunc);
            if(_topLeftSprite)
                _topLeftSprite->setBlendFunc(_blendFunc);
            if(_topSprite)
                _topSprite->setBlendFunc(_blendFunc);
            if(_topRightSprite)
                _topRightSprite->setBlendFunc(_blendFunc);
            if(_leftSprite)
                _leftSprite->setBlendFunc(_blendFunc);
            if(_centerSprite)
                _centerSprite->setBlendFunc(_blendFunc);
            if(_rightSprite)
                _rightSprite->setBlendFunc(_blendFunc);
            if(_bottomLeftSprite)
                _bottomLeftSprite->setBlendFunc(_blendFunc);
            if(_bottomSprite)
                _bottomSprite->setBlendFunc(_blendFunc);
            if(_bottomRightSprite)
                _bottomRightSprite->setBlendFunc(_blendFunc);
        }

        bool Scale9Sprite::updateWithSprite(Sprite* sprite,
                                            const MATH::Rectf& rect,
                                            bool rotated,
                                            const MATH::Rectf& capInsets)
        {
            return updateWithSprite(sprite, rect, rotated, MATH::Vec2fZERO, rect.size, capInsets);
        }

        static MATH::Rectf intersectRect(const MATH::Rectf &first, const MATH::Rectf &second)
        {
            MATH::Rectf ret;
            ret.origin.x = MATH::MATH_MAX(first.origin.x,second.origin.x);
            ret.origin.y = MATH::MATH_MAX(first.origin.y,second.origin.y);

            float rightRealPoint = MATH::MATH_MIN(first.origin.x + first.size.width,
                                            second.origin.x + second.size.width);
            float bottomRealPoint = MATH::MATH_MIN(first.origin.y + first.size.height,
                                             second.origin.y + second.size.height);

            ret.size.width = MATH::MATH_MAX(rightRealPoint - ret.origin.x, 0.0f);
            ret.size.height = MATH::MATH_MAX(bottomRealPoint - ret.origin.y, 0.0f);
            return ret;
        }

        bool Scale9Sprite::updateWithSprite(Sprite* sprite,
                                            const MATH::Rectf& textureRect,
                                            bool rotated,
                                            const MATH::Vector2f &offset,
                                            const MATH::Sizef &originalSize,
                                            const MATH::Rectf& capInsets)
        {
            GLubyte opacity = getOpacity();
            Color3B color = getColor();

            // Release old sprites
            this->cleanupSlicedSprites();
            _protectedChildren.clear();

            updateBlendFunc(sprite?sprite->getTexture():nullptr);

            if(nullptr != sprite)
            {
                if (nullptr == sprite->getSpriteFrame())
                {
                    return false;
                }

                if (nullptr == _scale9Image)
                {
                    _scale9Image = sprite;
                    _scale9Image->retain();
                }
                else
                {
                    _scale9Image->setSpriteFrame(sprite->getSpriteFrame());
                }
            }

            if (!_scale9Image)
            {
                return false;
            }

            SpriteFrame *spriteFrame = _scale9Image->getSpriteFrame();

            if (!spriteFrame)
            {
                return false;
            }

            MATH::Rectf rect(textureRect);
            MATH::Sizef size(originalSize);

            if(_capInsets.equals(MATH::RectfZERO))
                _capInsets = capInsets;

            // If there is no given rect
            if ( rect.equals(MATH::RectfZERO) )
            {
                // Get the texture size as original
                MATH::Sizef textureSize = _scale9Image->getTexture()->getContentSize();

                rect = MATH::Rectf(0, 0, textureSize.width, textureSize.height);
            }

            if( size.equals(MATH::SizefZERO) )
            {
                size = rect.size;
            }

            // Set the given rect's size as original size
            _spriteRect = rect;
            _offset = offset;
            _spriteFrameRotated = rotated;
            _originalSize = size;
            _preferredSize = size;
            if(!capInsets.equals(MATH::RectfZERO))
            {
                _capInsetsInternal = capInsets;
            }

            if (_scale9Enabled)
            {
                this->createSlicedSprites();
            }

            applyBlendFunc();
            if(this->_isPatch9)
            {
                size.width = size.width - 2;
                size.height = size.height - 2;
            }
            this->setContentSize(size);

            if (_spritesGenerated)
            {
                // Restore color and opacity
                this->setOpacity(opacity);
                this->setColor(color);
            }
            _spritesGenerated = true;

            return true;
        }

        void Scale9Sprite::createSlicedSprites()
        {
            float width = _originalSize.width;
            float height = _originalSize.height;

            MATH::Vector2f offsetPosition(ceilf(_offset.x + (_originalSize.width - _spriteRect.size.width) / 2),
                                ceilf(_offset.y + (_originalSize.height - _spriteRect.size.height) / 2));

            // If there is no specified center region
            if ( _capInsetsInternal.equals(MATH::RectfZERO) )
            {
                // log("... cap insets not specified : using default cap insets ...");
                _capInsetsInternal = MATH::Rectf(width /3, height /3, width /3, height /3);
            }

            MATH::Rectf originalRect=_spriteRect;
            if(_spriteFrameRotated)
                originalRect = MATH::Rectf(_spriteRect.origin.x - offsetPosition.y,
                                    _spriteRect.origin.y - offsetPosition.x,
                                    _originalSize.width, _originalSize.height);
            else
                originalRect = MATH::Rectf(_spriteRect.origin.x - offsetPosition.x,
                                    _spriteRect.origin.y - offsetPosition.y,
                                    _originalSize.width, _originalSize.height);

            float leftWidth = _capInsetsInternal.origin.x;
            float centerWidth = _capInsetsInternal.size.width;
            float rightWidth = originalRect.size.width - (leftWidth + centerWidth);

            float topHeight = _capInsetsInternal.origin.y;
            float centerHeight = _capInsetsInternal.size.height;
            float bottomHeight = originalRect.size.height - (topHeight + centerHeight);

            // calculate rects

            // ... top row
            float x = 0.0;
            float y = 0.0;
            //why do we need pixelRect?
            MATH::Rectf pixelRect = MATH::Rectf(offsetPosition.x, offsetPosition.y,
                                  _spriteRect.size.width, _spriteRect.size.height);

            // top left
            MATH::Rectf leftTopBoundsOriginal = MATH::Rectf(x, y, leftWidth, topHeight);
            MATH::Rectf leftTopBounds = leftTopBoundsOriginal;

            // top center
            x += leftWidth;
            MATH::Rectf centerTopBounds = MATH::Rectf(x, y, centerWidth, topHeight);

            // top right
            x += centerWidth;
            MATH::Rectf rightTopBounds = MATH::Rectf(x, y, rightWidth, topHeight);

            // ... center row
            x = 0.0;
            y = 0.0;
            y += topHeight;

            // center left
            MATH::Rectf leftCenterBounds = MATH::Rectf(x, y, leftWidth, centerHeight);

            // center center
            x += leftWidth;
            MATH::Rectf centerBoundsOriginal = MATH::Rectf(x, y, centerWidth, centerHeight);
            MATH::Rectf centerBounds = centerBoundsOriginal;

            // center right
            x += centerWidth;
            MATH::Rectf rightCenterBounds = MATH::Rectf(x, y, rightWidth, centerHeight);

            // ... bottom row
            x = 0.0;
            y = 0.0;
            y += topHeight;
            y += centerHeight;

            // bottom left
            MATH::Rectf leftBottomBounds = MATH::Rectf(x, y, leftWidth, bottomHeight);

            // bottom center
            x += leftWidth;
            MATH::Rectf centerBottomBounds = MATH::Rectf(x, y, centerWidth, bottomHeight);

            // bottom right
            x += centerWidth;
            MATH::Rectf rightBottomBoundsOriginal = MATH::Rectf(x, y, rightWidth, bottomHeight);
            MATH::Rectf rightBottomBounds = rightBottomBoundsOriginal;

            if((_capInsetsInternal.origin.x + _capInsetsInternal.size.width) <= _originalSize.width
               || (_capInsetsInternal.origin.y + _capInsetsInternal.size.height) <= _originalSize.height)
                //in general case it is error but for legacy support we will check it
            {
                leftTopBounds = intersectRect(leftTopBounds, pixelRect);
                centerTopBounds = intersectRect(centerTopBounds, pixelRect);
                rightTopBounds = intersectRect(rightTopBounds, pixelRect);
                leftCenterBounds = intersectRect(leftCenterBounds, pixelRect);
                centerBounds = intersectRect(centerBounds, pixelRect);
                rightCenterBounds = intersectRect(rightCenterBounds, pixelRect);
                leftBottomBounds = intersectRect(leftBottomBounds, pixelRect);
                centerBottomBounds = intersectRect(centerBottomBounds, pixelRect);
                rightBottomBounds = intersectRect(rightBottomBounds, pixelRect);
            }

            MATH::Rectf rotatedLeftTopBoundsOriginal = leftTopBoundsOriginal;
            MATH::Rectf rotatedCenterBoundsOriginal = centerBoundsOriginal;
            MATH::Rectf rotatedRightBottomBoundsOriginal = rightBottomBoundsOriginal;

            MATH::Rectf rotatedCenterBounds = centerBounds;
            MATH::Rectf rotatedRightBottomBounds = rightBottomBounds;
            MATH::Rectf rotatedLeftBottomBounds = leftBottomBounds;
            MATH::Rectf rotatedRightTopBounds = rightTopBounds;
            MATH::Rectf rotatedLeftTopBounds = leftTopBounds;
            MATH::Rectf rotatedRightCenterBounds = rightCenterBounds;
            MATH::Rectf rotatedLeftCenterBounds = leftCenterBounds;
            MATH::Rectf rotatedCenterBottomBounds = centerBottomBounds;
            MATH::Rectf rotatedCenterTopBounds = centerTopBounds;

            if (!_spriteFrameRotated)
            {

                MATH::AffineTransform t = MATH::AffineTransform::IDENTITY;
                t = AffineTransformTranslate(t, originalRect.origin.x, originalRect.origin.y);

                rotatedLeftTopBoundsOriginal = MATH::RectApplyAffineTransform(rotatedLeftTopBoundsOriginal, t);
                rotatedCenterBoundsOriginal = MATH::RectApplyAffineTransform(rotatedCenterBoundsOriginal, t);
                rotatedRightBottomBoundsOriginal = MATH::RectApplyAffineTransform(rotatedRightBottomBoundsOriginal, t);

                rotatedCenterBounds = MATH::RectApplyAffineTransform(rotatedCenterBounds, t);
                rotatedRightBottomBounds = MATH::RectApplyAffineTransform(rotatedRightBottomBounds, t);
                rotatedLeftBottomBounds = MATH::RectApplyAffineTransform(rotatedLeftBottomBounds, t);
                rotatedRightTopBounds = MATH::RectApplyAffineTransform(rotatedRightTopBounds, t);
                rotatedLeftTopBounds = MATH::RectApplyAffineTransform(rotatedLeftTopBounds, t);
                rotatedRightCenterBounds = MATH::RectApplyAffineTransform(rotatedRightCenterBounds, t);
                rotatedLeftCenterBounds = MATH::RectApplyAffineTransform(rotatedLeftCenterBounds, t);
                rotatedCenterBottomBounds = MATH::RectApplyAffineTransform(rotatedCenterBottomBounds, t);
                rotatedCenterTopBounds = MATH::RectApplyAffineTransform(rotatedCenterTopBounds, t);


            }
            else
            {
                // set up transformation of coordinates
                // to handle the case where the sprite is stored rotated
                // in the spritesheet
                // log("rotated");

                MATH::AffineTransform t = MATH::AffineTransform::IDENTITY;

                t = AffineTransformTranslate(t, originalRect.size.height+originalRect.origin.x, originalRect.origin.y);
                t = AffineTransformRotate(t, 1.57079633f);

                leftTopBoundsOriginal = MATH::RectApplyAffineTransform(leftTopBoundsOriginal, t);
                centerBoundsOriginal = MATH::RectApplyAffineTransform(centerBoundsOriginal, t);
                rightBottomBoundsOriginal = MATH::RectApplyAffineTransform(rightBottomBoundsOriginal, t);

                centerBounds = MATH::RectApplyAffineTransform(centerBounds, t);
                rightBottomBounds = MATH::RectApplyAffineTransform(rightBottomBounds, t);
                leftBottomBounds = MATH::RectApplyAffineTransform(leftBottomBounds, t);
                rightTopBounds = MATH::RectApplyAffineTransform(rightTopBounds, t);
                leftTopBounds = MATH::RectApplyAffineTransform(leftTopBounds, t);
                rightCenterBounds = MATH::RectApplyAffineTransform(rightCenterBounds, t);
                leftCenterBounds = MATH::RectApplyAffineTransform(leftCenterBounds, t);
                centerBottomBounds = MATH::RectApplyAffineTransform(centerBottomBounds, t);
                centerTopBounds = MATH::RectApplyAffineTransform(centerTopBounds, t);

                rotatedLeftTopBoundsOriginal.origin = leftTopBoundsOriginal.origin;
                rotatedCenterBoundsOriginal.origin = centerBoundsOriginal.origin;
                rotatedRightBottomBoundsOriginal.origin = rightBottomBoundsOriginal.origin;

                rotatedCenterBounds.origin = centerBounds.origin;
                rotatedRightBottomBounds.origin = rightBottomBounds.origin;
                rotatedLeftBottomBounds.origin = leftBottomBounds.origin;
                rotatedRightTopBounds.origin = rightTopBounds.origin;
                rotatedLeftTopBounds.origin = leftTopBounds.origin;
                rotatedRightCenterBounds.origin = rightCenterBounds.origin;
                rotatedLeftCenterBounds.origin = leftCenterBounds.origin;
                rotatedCenterBottomBounds.origin = centerBottomBounds.origin;
                rotatedCenterTopBounds.origin = centerTopBounds.origin;


            }

            _topLeftSize = rotatedLeftTopBoundsOriginal.size;
            _centerSize = rotatedCenterBoundsOriginal.size;
            _bottomRightSize = rotatedRightBottomBoundsOriginal.size;
            if(_isPatch9)
            {
                _topLeftSize.width = _topLeftSize.width - 1;
                _topLeftSize.height = _topLeftSize.height - 1;
                _bottomRightSize.width = _bottomRightSize.width - 1;
                _bottomRightSize.height = _bottomRightSize.height - 1;
            }

            if(_spriteFrameRotated)
            {
                float offsetX = (rotatedCenterBounds.origin.x + rotatedCenterBounds.size.height/2)
                    - (rotatedCenterBoundsOriginal.origin.x + rotatedCenterBoundsOriginal.size.height/2);
                float offsetY = (rotatedCenterBoundsOriginal.origin.y + rotatedCenterBoundsOriginal.size.width/2)
                    - (rotatedCenterBounds.origin.y + rotatedCenterBounds.size.width/2);
                _centerOffset.x = -offsetY;
                _centerOffset.y = offsetX;
            }
            else
            {
                float offsetX = (rotatedCenterBounds.origin.x + rotatedCenterBounds.size.width/2)
                    - (rotatedCenterBoundsOriginal.origin.x + rotatedCenterBoundsOriginal.size.width/2);
                float offsetY = (rotatedCenterBoundsOriginal.origin.y + rotatedCenterBoundsOriginal.size.height/2)
                    - (rotatedCenterBounds.origin.y + rotatedCenterBounds.size.height/2);
                _centerOffset.x = offsetX;
                _centerOffset.y = offsetY;
            }

            //shrink the image size when it is 9-patch
            if(_isPatch9)
            {
                float offset = 1.4f;
                //Top left
                if(!_spriteFrameRotated)
                {
                    rotatedLeftTopBounds.origin.x+=offset;
                    rotatedLeftTopBounds.origin.y+=offset;
                    rotatedLeftTopBounds.size.width-=offset;
                    rotatedLeftTopBounds.size.height-=offset;
                    //Center left
                    rotatedLeftCenterBounds.origin.x+=offset;
                    rotatedLeftCenterBounds.size.width-=offset;
                    //Bottom left
                    rotatedLeftBottomBounds.origin.x+=offset;
                    rotatedLeftBottomBounds.size.width-=offset;
                    rotatedLeftBottomBounds.size.height-=offset;
                    //Top center
                    rotatedCenterTopBounds.size.height-=offset;
                    rotatedCenterTopBounds.origin.y+=offset;
                    //Bottom center
                    rotatedCenterBottomBounds.size.height-=offset;
                    //Top right
                    rotatedRightTopBounds.size.width-=offset;
                    rotatedRightTopBounds.size.height-=offset;
                    rotatedRightTopBounds.origin.y+=offset;
                    //Center right
                    rotatedRightCenterBounds.size.width-=offset;
                    //Bottom right
                    rotatedRightBottomBounds.size.width-=offset;
                    rotatedRightBottomBounds.size.height-=offset;
                }
                else
                {
                    //Top left
                    rotatedLeftTopBounds.size.width-=offset;
                    rotatedLeftTopBounds.size.height-=offset;
                    rotatedLeftTopBounds.origin.y+=offset;
                    //Center left
                    rotatedLeftCenterBounds.origin.y+=offset;
                    rotatedLeftCenterBounds.size.width-=offset;
                    //Bottom left
                    rotatedLeftBottomBounds.origin.x+=offset;
                    rotatedLeftBottomBounds.origin.y+=offset;
                    rotatedLeftBottomBounds.size.width-=offset;
                    rotatedLeftBottomBounds.size.height-=offset;
                    //Top center
                    rotatedCenterTopBounds.size.height-=offset;
                    //Bottom center
                    rotatedCenterBottomBounds.size.height-=offset;
                    rotatedCenterBottomBounds.origin.x+=offset;
                    //Top right
                    rotatedRightTopBounds.size.width-=offset;
                    rotatedRightTopBounds.size.height-=offset;
                    //Center right
                    rotatedRightCenterBounds.size.width-=offset;
                    //Bottom right
                    rotatedRightBottomBounds.size.width-=offset;
                    rotatedRightBottomBounds.size.height-=offset;
                    rotatedRightBottomBounds.origin.x+=offset;
                }
            }

            // Centre
            if(rotatedCenterBounds.size.width > 0 && rotatedCenterBounds.size.height > 0 )
            {
                _centerSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                          rotatedCenterBounds,
                                                          _spriteFrameRotated);
                _centerSprite->retain();
                this->addProtectedChild(_centerSprite);
            }

            // Top
            if(rotatedCenterTopBounds.size.width > 0 && rotatedCenterTopBounds.size.height > 0 )
            {
                _topSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                       rotatedCenterTopBounds,
                                                       _spriteFrameRotated);
                _topSprite->retain();
                this->addProtectedChild(_topSprite);
            }

            // Bottom
            if(rotatedCenterBottomBounds.size.width > 0 && rotatedCenterBottomBounds.size.height > 0 )
            {
                _bottomSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                          rotatedCenterBottomBounds,
                                                          _spriteFrameRotated);
                _bottomSprite->retain();
                this->addProtectedChild(_bottomSprite);
            }

            // Left
            if(rotatedLeftCenterBounds.size.width > 0 && rotatedLeftCenterBounds.size.height > 0 )
            {
                _leftSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                        rotatedLeftCenterBounds,
                                                        _spriteFrameRotated);
                _leftSprite->retain();
                this->addProtectedChild(_leftSprite);
            }

            // Right
            if(rotatedRightCenterBounds.size.width > 0 && rotatedRightCenterBounds.size.height > 0 )
            {
                _rightSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                         rotatedRightCenterBounds,
                                                         _spriteFrameRotated);
                _rightSprite->retain();
                this->addProtectedChild(_rightSprite);
            }

            // Top left
            if(rotatedLeftTopBounds.size.width > 0 && rotatedLeftTopBounds.size.height > 0 )
            {
                _topLeftSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                           rotatedLeftTopBounds,
                                                           _spriteFrameRotated);
                _topLeftSprite->retain();
                this->addProtectedChild(_topLeftSprite);
            }

            // Top right
            if(rotatedRightTopBounds.size.width > 0 && rotatedRightTopBounds.size.height > 0 )
            {
                _topRightSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                            rotatedRightTopBounds,
                                                            _spriteFrameRotated);
                _topRightSprite->retain();
                this->addProtectedChild(_topRightSprite);
            }

            // Bottom left
            if(rotatedLeftBottomBounds.size.width > 0 && rotatedLeftBottomBounds.size.height > 0 )
            {
                _bottomLeftSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                              rotatedLeftBottomBounds,
                                                              _spriteFrameRotated);
                _bottomLeftSprite->retain();
                this->addProtectedChild(_bottomLeftSprite);
            }

            // Bottom right
            if(rotatedRightBottomBounds.size.width > 0 && rotatedRightBottomBounds.size.height > 0 )
            {
                _bottomRightSprite = Sprite::createWithTexture(_scale9Image->getTexture(),
                                                               rotatedRightBottomBounds,
                                                               _spriteFrameRotated);
                _bottomRightSprite->retain();
                this->addProtectedChild(_bottomRightSprite);
            }
        }

        void Scale9Sprite::setContentSize(const MATH::Sizef &size)
        {
            Node::setContentSize(size);
            this->_positionsAreDirty = true;
        }

        void Scale9Sprite::updatePositions()
        {
            MATH::Sizef size = this->_contentSize;

            float sizableWidth = size.width - _topLeftSize.width - _bottomRightSize.width;
            float sizableHeight = size.height - _topLeftSize.height - _bottomRightSize.height;

            float horizontalScale = sizableWidth/_centerSize.width;
            float verticalScale = sizableHeight/_centerSize.height;

            if(_centerSprite)
            {
                _centerSprite->setScaleX(horizontalScale);
                _centerSprite->setScaleY(verticalScale);
            }

            float rescaledWidth = _centerSize.width * horizontalScale;
            float rescaledHeight = _centerSize.height * verticalScale;

            float leftWidth = _topLeftSize.width;
            float bottomHeight = _bottomRightSize.height;

            MATH::Vector2f centerOffset(_centerOffset.x * horizontalScale, _centerOffset.y * verticalScale);

            // Position corners
            if(_bottomLeftSprite)
            {
                _bottomLeftSprite->setAnchorPoint(MATH::Vector2f(1,1));
                _bottomLeftSprite->setPosition(leftWidth,bottomHeight);
            }
            if(_bottomRightSprite)
            {
                _bottomRightSprite->setAnchorPoint(MATH::Vector2f(0,1));
                _bottomRightSprite->setPosition(leftWidth+rescaledWidth,bottomHeight);
            }
            if(_topLeftSprite)
            {
                _topLeftSprite->setAnchorPoint(MATH::Vector2f(1,0));
                _topLeftSprite->setPosition(leftWidth, bottomHeight+rescaledHeight);
            }
            if(_topRightSprite)
            {
                _topRightSprite->setAnchorPoint(MATH::Vector2f(0,0));
                _topRightSprite->setPosition(leftWidth+rescaledWidth, bottomHeight+rescaledHeight);
            }

            // Scale and position borders
            if(_leftSprite)
            {
                _leftSprite->setAnchorPoint(MATH::Vector2f(1,0.5));
                _leftSprite->setPosition(leftWidth, bottomHeight+rescaledHeight/2 + centerOffset.y);
                _leftSprite->setScaleY(verticalScale);
            }
            if(_rightSprite)
            {
                _rightSprite->setAnchorPoint(MATH::Vector2f(0,0.5));
                _rightSprite->setPosition(leftWidth+rescaledWidth,bottomHeight+rescaledHeight/2 + centerOffset.y);
                _rightSprite->setScaleY(verticalScale);
            }
            if(_topSprite)
            {
                _topSprite->setAnchorPoint(MATH::Vector2f(0.5,0));
                _topSprite->setPosition(leftWidth+rescaledWidth/2 + centerOffset.x,bottomHeight+rescaledHeight);
                _topSprite->setScaleX(horizontalScale);
            }
            if(_bottomSprite)
            {
                _bottomSprite->setAnchorPoint(MATH::Vector2f(0.5,1));
                _bottomSprite->setPosition(leftWidth+rescaledWidth/2 + centerOffset.x,bottomHeight);
                _bottomSprite->setScaleX(horizontalScale);
            }
            // Position centre
            if(_centerSprite)
            {
                _centerSprite->setAnchorPoint(MATH::Vector2f(0.5,0.5));
                _centerSprite->setPosition(leftWidth+rescaledWidth/2 + centerOffset.x,
                                           bottomHeight+rescaledHeight/2 + centerOffset.y);
                _centerSprite->setScaleX(horizontalScale);
                _centerSprite->setScaleY(verticalScale);
            }
        }



        Scale9Sprite* Scale9Sprite::resizableSpriteWithCapInsets(const MATH::Rectf&) const
        {
            Scale9Sprite* pReturn = new (std::nothrow) Scale9Sprite();
            if ( pReturn && pReturn->init(_scale9Image,
                                          _spriteRect,
                                          _spriteFrameRotated,
                                          _offset,
                                          _originalSize,
                                          _capInsets) )
            {
                pReturn->autorelease();
                return pReturn;
            }
            SAFE_DELETE(pReturn);
            return NULL;
        }


        void Scale9Sprite::setState(Scale9Sprite::State state)
        {
            GLShaderState *glState = nullptr;
            switch (state)
            {
            case State::NORMAL:
            {
                glState = GLShaderState::getOrCreateWithGLShaderName(GLShader::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP);
            }
            break;
            case State::GRAY:
            {
                glState = GLShaderState::getOrCreateWithGLShaderName(GLShader::SHADER_NAME_POSITION_GRAYSCALE);
            }
            default:
                break;
            }

            if (nullptr != _scale9Image)
            {
                _scale9Image->setGLShaderState(glState);
            }

            if (_scale9Enabled)
            {
                for (auto& sp : _protectedChildren)
                {
                    sp->setGLShaderState(glState);
                }
            }
        }

    /** sets the opacity.
        @warning If the the texture has premultiplied alpha then, the R, G and B channels will be modifed.
        Values goes from 0 to 255, where 255 means fully opaque.
    */



        void Scale9Sprite::updateCapInset()
        {
            MATH::Rectf insets;
            if (this->_insetLeft == 0 && this->_insetTop == 0 && this->_insetRight == 0 && this->_insetBottom == 0)
            {
                insets = MATH::RectfZERO;
            }
            else
            {
                insets = MATH::Rectf(_insetLeft,
                              _insetTop,
                              _originalSize.width-_insetLeft-_insetRight,
                              _originalSize.height-_insetTop-_insetBottom);
            }
            this->setCapInsets(insets);
        }

        void Scale9Sprite::setPreferredSize(const MATH::Sizef& preferedSize)
        {
            this->setContentSize(preferedSize);
            this->_preferredSize = preferedSize;
        }


        void Scale9Sprite::setCapInsets(const MATH::Rectf& capInsets)
        {
            MATH::Sizef contentSize = this->_contentSize;
            this->updateWithSprite(this->_scale9Image,
                                   _spriteRect,
                                   _spriteFrameRotated,
                                   _offset,
                                   _originalSize,
                                   capInsets);
            this->_insetLeft = capInsets.origin.x;
            this->_insetTop = capInsets.origin.y;
            this->_insetRight = _originalSize.width - _insetLeft - capInsets.size.width;
            this->_insetBottom = _originalSize.height - _insetTop - capInsets.size.height;
            this->setContentSize(contentSize);
        }


        void Scale9Sprite::setInsetLeft(float insetLeft)
        {
            this->_insetLeft = insetLeft;
            this->updateCapInset();
        }

        void Scale9Sprite::setInsetTop(float insetTop)
        {
            this->_insetTop = insetTop;
            this->updateCapInset();
        }

        void Scale9Sprite::setInsetRight(float insetRight)
        {
            this->_insetRight = insetRight;
            this->updateCapInset();
        }

        void Scale9Sprite::setInsetBottom(float insetBottom)
        {
            this->_insetBottom = insetBottom;
            this->updateCapInset();
        }

        void Scale9Sprite::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
        {

            // quick return if not visible. children won't be drawn.
            if (!_visible)
            {
                return;
            }

            uint32_t flags = processParentFlags(parentTransform, parentFlags);

            // IMPORTANT:
            // To ease the migration to v3.0, we still support the Mat4 stack,
            // but it is deprecated and your code should not rely on it            
            Director::getInstance().pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            Director::getInstance().loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

            int i = 0;      // used by _children
            int j = 0;      // used by _protectedChildren

            sortAllChildren();
            sortAllProtectedChildren();

            //
            // draw children and protectedChildren zOrder < 0
            //
            for( ; i < _children.size(); i++ )
            {
                auto node = _children.at(i);

                if ( node && node->getLocalZOrder() < 0 )
                    node->visit(renderer, _modelViewTransform, flags);
                else
                    break;
            }

            if (_scale9Enabled)
            {
                for( ; j < _protectedChildren.size(); j++ )
                {
                    auto node = _protectedChildren.at(j);

                    if ( node && node->getLocalZOrder() < 0 )
                        node->visit(renderer, _modelViewTransform, flags);
                    else
                        break;
                }
            }
            else
            {
                if (_scale9Image && _scale9Image->getLocalZOrder() < 0 )
                {
                    _scale9Image->visit(renderer, _modelViewTransform, flags);
                }
            }

            //
            // draw self
            //
            this->draw(renderer, _modelViewTransform, flags);

            //
            // draw children and protectedChildren zOrder >= 0
            //
            if (_scale9Enabled)
            {
                for(auto it=_protectedChildren.cbegin()+j; it != _protectedChildren.cend(); ++it)
                    (*it)->visit(renderer, _modelViewTransform, flags);
            }
            else
            {
                if (_scale9Image && _scale9Image->getLocalZOrder() >= 0 )
                {
                    _scale9Image->visit(renderer, _modelViewTransform, flags);
                }
            }


            for(auto it=_children.cbegin()+i; it != _children.cend(); ++it)
                (*it)->visit(renderer, _modelViewTransform, flags);

            // FIX ME: Why need to set _orderOfArrival to 0??
            // Please refer to https://github.com/cocos2d/cocos2d-x/pull/6920
            // setOrderOfArrival(0);

            Director::getInstance().popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

        }

        MATH::Sizef Scale9Sprite::getOriginalSize()const
        {
            return _originalSize;
        }


        MATH::Sizef Scale9Sprite::getPreferredSize() const
        {
            return _preferredSize;
        }

        MATH::Rectf Scale9Sprite::getCapInsets()const
        {
            return _capInsets;
        }


        float Scale9Sprite::getInsetLeft()const
        {
            return this->_insetLeft;
        }

        float Scale9Sprite::getInsetTop()const
        {
            return this->_insetTop;
        }

        float Scale9Sprite::getInsetRight()const
        {
            return this->_insetRight;
        }

        float Scale9Sprite::getInsetBottom()const
        {
            return this->_insetBottom;
        }

        void Scale9Sprite::setScale9Enabled(bool enabled)
        {
            if (_scale9Enabled == enabled)
            {
                return;
            }
            _scale9Enabled = enabled;

            this->cleanupSlicedSprites();
            _protectedChildren.clear();

            //we must invalide the transform when toggling scale9enabled
            _transformUpdated = _transformDirty = _inverseDirty = true;

            if (_scale9Enabled)
            {
                if (_scale9Image)
                {
                    this->updateWithSprite(this->_scale9Image,
                                           _spriteRect,
                                           _spriteFrameRotated,
                                           _offset,
                                           _originalSize,
                                           _capInsets);
                }
            }
            _positionsAreDirty = true;
        }

        bool Scale9Sprite::isScale9Enabled() const
        {
            return _scale9Enabled;
        }

        void Scale9Sprite::addProtectedChild(Node *child)
        {
            _reorderProtectedChildDirty = true;
            _protectedChildren.pushBack(child);
        }

        void Scale9Sprite::sortAllProtectedChildren()
        {
            if(this->_positionsAreDirty)
            {
                this->updatePositions();
                this->adjustScale9ImagePosition();
                this->_positionsAreDirty = false;
            }
            if( _reorderProtectedChildDirty )
            {
                std::sort( std::begin(_protectedChildren),
                           std::end(_protectedChildren),
                           Node::nodeComparisonLess );
                _reorderProtectedChildDirty = false;
            }
        }

        void Scale9Sprite::adjustScale9ImagePosition()
        {
            if (_scale9Image)
            {
                _scale9Image->setPosition(_contentSize.width * _scale9Image->getAnchorPoint().x,
                                          _contentSize.height * _scale9Image->getAnchorPoint().y);
            }
        }

        void Scale9Sprite::setAnchorPoint(const MATH::Vector2f &position)
        {
            Node::setAnchorPoint(position);
            if (!_scale9Enabled)
            {
                if (_scale9Image)
                {
                    _scale9Image->setAnchorPoint(position);
                    _positionsAreDirty = true;
                }
            }
        }

        void Scale9Sprite::cleanup()
        {
            Node::cleanup();
            // timers
            for( const auto &child: _protectedChildren)
                child->cleanup();
        }

        void Scale9Sprite::onEnter()
        {
            Node::onEnter();
            for( const auto &child: _protectedChildren)
                child->onEnter();
        }

        void Scale9Sprite::onExit()
        {
            Node::onExit();
            for( const auto &child: _protectedChildren)
                child->onExit();
        }

        void Scale9Sprite::onEnterTransitionDidFinish()
        {
            Node::onEnterTransitionDidFinish();
            for( const auto &child: _protectedChildren)
                child->onEnterTransitionDidFinish();
        }

        void Scale9Sprite::onExitTransitionDidStart()
        {
            Node::onExitTransitionDidStart();
            for( const auto &child: _protectedChildren)
                child->onExitTransitionDidStart();
        }

        void Scale9Sprite::updateDisplayedColor(const Color3B &parentColor)
        {
            _displayedColor.red = _realColor.red * parentColor.red/255.0;
            _displayedColor.green = _realColor.green * parentColor.green/255.0;
            _displayedColor.blue = _realColor.blue * parentColor.blue/255.0;
            updateColor();

            if (_scale9Image)
            {
                _scale9Image->updateDisplayedColor(_displayedColor);
            }

            for(const auto &child : _protectedChildren)
            {
                child->updateDisplayedColor(_displayedColor);
            }

            if (_cascadeColorEnabled)
            {
                for(const auto &child : _children)
                {
                    child->updateDisplayedColor(_displayedColor);
                }
            }
        }

        void Scale9Sprite::updateDisplayedOpacity(GLubyte parentOpacity)
        {
            _displayedOpacity = _realOpacity * parentOpacity/255.0;
            updateColor();

            if (_scale9Image)
            {
                _scale9Image->updateDisplayedOpacity(_displayedOpacity);
            }

            for(auto child : _protectedChildren)
            {
                child->updateDisplayedOpacity(_displayedOpacity);
            }

            if (_cascadeOpacityEnabled)
            {
                for(auto child : _children)
                {
                    child->updateDisplayedOpacity(_displayedOpacity);
                }
            }
        }

        void Scale9Sprite::disableCascadeColor()
        {
            for(auto child : _children)
            {
                child->updateDisplayedColor(Color3B::WHITE);
            }
            for(auto child : _protectedChildren)
            {
                child->updateDisplayedColor(Color3B::WHITE);
            }
            if (_scale9Image)
            {
                _scale9Image->updateDisplayedColor(Color3B::WHITE);
            }
        }

        void Scale9Sprite::disableCascadeOpacity()
        {
            _displayedOpacity = _realOpacity;

            for(auto child : _children){
                child->updateDisplayedOpacity(255);
            }

            for(auto child : _protectedChildren){
                child->updateDisplayedOpacity(255);
            }
        }

        Sprite* Scale9Sprite::getSprite()const
        {
            return _scale9Image;
        }

        void Scale9Sprite::setFlippedX(bool flippedX)
        {

            float realScale = this->getScaleX();
            _flippedX = flippedX;
            this->setScaleX(realScale);
        }

        void Scale9Sprite::setFlippedY(bool flippedY)
        {
            float realScale = this->getScaleY();
            _flippedY = flippedY;
            this->setScaleY(realScale);
        }

        bool Scale9Sprite::isFlippedX()const
        {
            return _flippedX;
        }

        bool Scale9Sprite::isFlippedY()const
        {
            return _flippedY;
        }

        void Scale9Sprite::setScaleX(float scaleX)
        {
            if (_flippedX) {
                scaleX = scaleX * -1;
            }
            Node::setScaleX(scaleX);
        }

        void Scale9Sprite::setScaleY(float scaleY)
        {
            if (_flippedY) {
                scaleY = scaleY * -1;
            }
            Node::setScaleY(scaleY);
        }

        void Scale9Sprite::setScale(float scale)
        {
            this->setScaleX(scale);
            this->setScaleY(scale);
            this->setScaleZ(scale);
        }

        void Scale9Sprite::setScale(float scaleX, float scaleY)
        {
            this->setScaleX(scaleX);
            this->setScaleY(scaleY);
        }

        float Scale9Sprite::getScaleX()const
        {
            float originalScale = Node::getScaleX();
            if (_flippedX)
            {
                originalScale = originalScale * -1.0;
            }
            return originalScale;
        }

        float Scale9Sprite::getScaleY()const
        {
            float originalScale = Node::getScaleY();
            if (_flippedY)
            {
                originalScale = originalScale * -1.0;
            }
            return originalScale;
        }

        float Scale9Sprite::getScale()const
        {
            return this->getScaleX();
        }

        void Scale9Sprite::setCameraMask(unsigned short mask, bool applyChildren)
        {
            Node::setCameraMask(mask, applyChildren);

            if(_scale9Image)
                _scale9Image->setCameraMask(mask,applyChildren);

            for(auto& iter: _protectedChildren)
            {
                iter->setCameraMask(mask);
            }
        }
    }
}
