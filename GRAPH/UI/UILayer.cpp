#include <stdarg.h>
#include "GRAPH/UI/UILayer.h"
#include "GRAPH/Director.h"
#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"
#include "GRAPH/UNITY3D/Renderer.h"

namespace GRAPH
{
    namespace UI
    {
        // Layer
        Layer::Layer()
        : touchEnabled_(false)
        , accelerometerEnabled_(false)
        , keyboardEnabled_(false)
        , touchListener_(nullptr)
        , keyboardListener_(nullptr)
        , accelerationListener_(nullptr)
        , touchMode_(Touch::DispatchMode::ALL_AT_ONCE)
        , swallowsTouches_(true)
        {
            ignoreAnchorPointForPosition_ = true;
            setAnchorPoint(MATH::Vector2f(0.5f, 0.5f));
        }

        Layer::~Layer()
        {

        }

        bool Layer::init()
        {
            setContentSize(Director::getInstance().getWinSize());
            return true;
        }

        Layer *Layer::create()
        {
            Layer *ret = new (std::nothrow) Layer();
            if (ret && ret->init())
            {
                ret->autorelease();
                return ret;
            }
            else
            {
                SAFE_DELETE(ret);
                return nullptr;
            }
        }

        void Layer::onAcceleration(Acceleration*, Event*)
        {
        }

        void Layer::onKeyPressed(EventKeyboard::KeyCode, Event*)
        {
        }

        void Layer::onKeyReleased(EventKeyboard::KeyCode, Event*)
        {
        }

        bool Layer::onTouchBegan(Touch *, Event *)
        {
            return true;
        }

        void Layer::onTouchMoved(Touch *, Event *)
        {
        }

        void Layer::onTouchEnded(Touch *, Event *)
        {
        }

        void Layer::onTouchCancelled(Touch *, Event *)
        {
        }

        void Layer::onTouchesBegan(const std::vector<Touch*>&, Event *)
        {
        }

        void Layer::onTouchesMoved(const std::vector<Touch*>&, Event *)
        {
        }

        void Layer::onTouchesEnded(const std::vector<Touch*>&, Event *)
        {
        }

        void Layer::onTouchesCancelled(const std::vector<Touch*>&, Event *)
        {
        }

        LayerRGBA::LayerRGBA()
        {
        }

        LayerColor::LayerColor()
        {
            // default blend function
            blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
            u3dContext_ = Unity3DCreator::CreateContext();
        }

        LayerColor::~LayerColor()
        {
            SAFE_RELEASE(u3dVertexFormat_);
            SAFE_RELEASE(u3dContext_);
        }

        /// blendFunc getter
        const BlendFunc &LayerColor::getBlendFunc() const
        {
            return blendFunc_;
        }
        /// blendFunc setter
        void LayerColor::setBlendFunc(const BlendFunc &var)
        {
            blendFunc_ = var;
        }

        LayerColor* LayerColor::create()
        {
            LayerColor* ret = new (std::nothrow) LayerColor();
            if (ret && ret->init())
            {
                ret->autorelease();
            }
            else
            {
                SAFE_DELETE(ret);
            }
            return ret;
        }

        LayerColor * LayerColor::create(const Color4B& color, GLfloat width, GLfloat height)
        {
            LayerColor * layer = new (std::nothrow) LayerColor();
            if( layer && layer->initWithColor(color,width,height))
            {
                layer->autorelease();
                return layer;
            }
            SAFE_DELETE(layer);
            return nullptr;
        }

        LayerColor * LayerColor::create(const Color4B& color)
        {
            LayerColor * layer = new (std::nothrow) LayerColor();
            if(layer && layer->initWithColor(color))
            {
                layer->autorelease();
                return layer;
            }
            SAFE_DELETE(layer);
            return nullptr;
        }

        bool LayerColor::init()
        {
            MATH::Sizef s = Director::getInstance().getWinSize();
            return initWithColor(Color4B(0,0,0,0), s.width, s.height);
        }

        bool LayerColor::initWithColor(const Color4B& color, GLfloat w, GLfloat h) {
            if (Layer::init()) {
                // default blend function
                blendFunc_ = BlendFunc::ALPHA_NON_PREMULTIPLIED;

                displayedColor_.red = realColor_.red = color.red;
                displayedColor_.green = realColor_.green = color.green;
                displayedColor_.blue = realColor_.blue = color.blue;
                displayedOpacity_ = realOpacity_ = color.alpha;

                for (uint64 i = 0; i<sizeof(squareVertices_) / sizeof( squareVertices_[0]); i++ ) {
                    squareVertices_[i].x = 0.0f;
                    squareVertices_[i].y = 0.0f;
                }

                updateColor();
                setContentSize(MATH::Sizef(w, h));

                std::vector<U3DVertexComponent> vertexFormat = {
                    U3DVertexComponent(SEM_POSITION, FLOATx3, 0, intptr(noMVPVertices_)),
                    U3DVertexComponent(SEM_COLOR0, FLOATx4, 0, intptr(squareColors_)) };
                u3dVertexFormat_ = Unity3DCreator::CreateVertexFormat(vertexFormat);

                setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_COLOR_NO_MVP));
                return true;
            }
            return false;
        }

        bool LayerColor::initWithColor(const Color4B& color)
        {
            MATH::Sizef s = Director::getInstance().getWinSize();
            this->initWithColor(color, s.width, s.height);
            return true;
        }

        /// override contentSize
        void LayerColor::setContentSize(const MATH::Sizef & size)
        {
            squareVertices_[1].x = size.width;
            squareVertices_[2].y = size.height;
            squareVertices_[3].x = size.width;
            squareVertices_[3].y = size.height;

            Layer::setContentSize(size);
        }

        void LayerColor::changeWidthAndHeight(GLfloat w ,GLfloat h)
        {
            this->setContentSize(MATH::Sizef(w, h));
        }

        void LayerColor::changeWidth(GLfloat w)
        {
            this->setContentSize(MATH::Sizef(w, contentSize_.height));
        }

        void LayerColor::changeHeight(GLfloat h)
        {
            this->setContentSize(MATH::Sizef(contentSize_.width, h));
        }

        void LayerColor::updateColor()
        {
            for( unsigned int i=0; i < 4; i++ )
            {
                squareColors_[i].red = displayedColor_.red / 255.0f;
                squareColors_[i].green = displayedColor_.green / 255.0f;
                squareColors_[i].blue = displayedColor_.blue / 255.0f;
                squareColors_[i].alpha = displayedOpacity_ / 255.0f;
            }
        }

        void LayerColor::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
        {
            customCommand_.init(globalZOrder_, transform, flags);
            customCommand_.func = std::bind(&LayerColor::onDraw, this, transform, flags);
            renderer->addCommand(&customCommand_);

            for(int i = 0; i < 4; ++i)
            {
                MATH::Vector4f pos;
                pos.x = squareVertices_[i].x; pos.y = squareVertices_[i].y; pos.z = positionZ_;
                pos.w = 1;
                modelViewTransform_.transformVector(&pos);
                noMVPVertices_[i] = MATH::Vector3f(pos.x,pos.y,pos.z)/pos.w;
            }
        }

        void LayerColor::onDraw(const MATH::Matrix4& transform, uint32_t) {
            getU3DShader()->apply();
            getU3DShader()->setUniformsForBuiltins(transform);

            Unity3DGLState::OpenGLState().blendFunc.set(blendFunc_.src, blendFunc_.dst);
            u3dContext_->drawUp(PRIM_TRIANGLESGL_STRIP, u3dVertexFormat_, 0, 4);
        }

        //
        // LayerGradient
        //
        LayerGradient::LayerGradient()
        : startColor_(Color4B::BLACK)
        , endColor_(Color4B::BLACK)
        , startOpacity_(255)
        , endOpacity_(255)
        , alongVector_(MATH::Vector2f(0, -1))
        , compressedInterpolation_(true)
        {

        }

        LayerGradient::~LayerGradient()
        {
        }

        LayerGradient* LayerGradient::create(const Color4B& start, const Color4B& end)
        {
            LayerGradient * layer = new (std::nothrow) LayerGradient();
            if( layer && layer->initWithColor(start, end))
            {
                layer->autorelease();
                return layer;
            }
            SAFE_DELETE(layer);
            return nullptr;
        }

        LayerGradient* LayerGradient::create(const Color4B& start, const Color4B& end, const MATH::Vector2f& v)
        {
            LayerGradient * layer = new (std::nothrow) LayerGradient();
            if( layer && layer->initWithColor(start, end, v))
            {
                layer->autorelease();
                return layer;
            }
            SAFE_DELETE(layer);
            return nullptr;
        }

        LayerGradient* LayerGradient::create()
        {
            LayerGradient* ret = new (std::nothrow) LayerGradient();
            if (ret && ret->init())
            {
                ret->autorelease();
            }
            else
            {
                SAFE_DELETE(ret);
            }
            return ret;
        }

        bool LayerGradient::init()
        {
            return initWithColor(Color4B(0, 0, 0, 255), Color4B(0, 0, 0, 255));
        }

        bool LayerGradient::initWithColor(const Color4B& start, const Color4B& end)
        {
            return initWithColor(start, end, MATH::Vector2f(0, -1));
        }

        bool LayerGradient::initWithColor(const Color4B& start, const Color4B& end, const MATH::Vector2f& v)
        {
            endColor_.red  = end.red;
            endColor_.green  = end.green;
            endColor_.blue  = end.blue;

            endOpacity_     = end.alpha;
            startOpacity_   = start.alpha;
            alongVector_    = v;

            compressedInterpolation_ = true;

            return LayerColor::initWithColor(Color4B(start.red, start.green, start.blue, 255));
        }

        void LayerGradient::updateColor()
        {
            LayerColor::updateColor();

            float h = alongVector_.length();
            if (h == 0)
                return;

            float c = sqrtf(2.0f);
            MATH::Vector2f u(alongVector_.x / h, alongVector_.y / h);

            // Compressed Interpolation mode
            if (compressedInterpolation_)
            {
                float h2 = 1 / ( fabsf(u.x) + fabsf(u.y) );
                u = u * (h2 * (float)c);
            }

            float opacityf = (float)displayedOpacity_ / 255.0f;

            Color4F S(
                displayedColor_.red / 255.0f,
                displayedColor_.green / 255.0f,
                displayedColor_.blue / 255.0f,
                startOpacity_ * opacityf / 255.0f
            );

            Color4F E(
                endColor_.red / 255.0f,
                endColor_.green / 255.0f,
                endColor_.blue / 255.0f,
                endOpacity_ * opacityf / 255.0f
            );

            // (-1, -1)
            squareColors_[0].red = E.red + (S.red - E.red) * ((c + u.x + u.y) / (2.0f * c));
            squareColors_[0].green = E.green + (S.green - E.green) * ((c + u.x + u.y) / (2.0f * c));
            squareColors_[0].blue = E.blue + (S.blue - E.blue) * ((c + u.x + u.y) / (2.0f * c));
            squareColors_[0].alpha = E.alpha + (S.alpha - E.alpha) * ((c + u.x + u.y) / (2.0f * c));
            // (1, -1)
            squareColors_[1].red = E.red + (S.red - E.red) * ((c - u.x + u.y) / (2.0f * c));
            squareColors_[1].green = E.green + (S.green - E.green) * ((c - u.x + u.y) / (2.0f * c));
            squareColors_[1].blue = E.blue + (S.blue - E.blue) * ((c - u.x + u.y) / (2.0f * c));
            squareColors_[1].alpha = E.alpha + (S.alpha - E.alpha) * ((c - u.x + u.y) / (2.0f * c));
            // (-1, 1)
            squareColors_[2].red = E.red + (S.red - E.red) * ((c + u.x - u.y) / (2.0f * c));
            squareColors_[2].green = E.green + (S.green - E.green) * ((c + u.x - u.y) / (2.0f * c));
            squareColors_[2].blue = E.blue + (S.blue - E.blue) * ((c + u.x - u.y) / (2.0f * c));
            squareColors_[2].alpha = E.alpha + (S.alpha - E.alpha) * ((c + u.x - u.y) / (2.0f * c));
            // (1, 1)
            squareColors_[3].red = E.red + (S.red - E.red) * ((c - u.x - u.y) / (2.0f * c));
            squareColors_[3].green = E.green + (S.green - E.green) * ((c - u.x - u.y) / (2.0f * c));
            squareColors_[3].blue = E.blue + (S.blue - E.blue) * ((c - u.x - u.y) / (2.0f * c));
            squareColors_[3].alpha = E.alpha + (S.alpha - E.alpha) * ((c - u.x - u.y) / (2.0f * c));
        }

        const Color3B& LayerGradient::getStartColor() const
        {
            return realColor_;
        }

        void LayerGradient::setStartColor(const Color3B& color)
        {
            setColor(color);
        }

        void LayerGradient::setEndColor(const Color3B& color)
        {
            endColor_ = color;
            updateColor();
        }

        const Color3B& LayerGradient::getEndColor() const
        {
            return endColor_;
        }

        void LayerGradient::setStartOpacity(uint8 o)
        {
            startOpacity_ = o;
            updateColor();
        }

        uint8 LayerGradient::getStartOpacity() const
        {
            return startOpacity_;
        }

        void LayerGradient::setEndOpacity(uint8 o)
        {
            endOpacity_ = o;
            updateColor();
        }

        uint8 LayerGradient::getEndOpacity() const
        {
            return endOpacity_;
        }

        void LayerGradient::setVector(const MATH::Vector2f& var)
        {
            alongVector_ = var;
            updateColor();
        }

        const MATH::Vector2f& LayerGradient::getVector() const
        {
            return alongVector_;
        }

        bool LayerGradient::isCompressedInterpolation() const
        {
            return compressedInterpolation_;
        }

        void LayerGradient::setCompressedInterpolation(bool compress)
        {
            compressedInterpolation_ = compress;
            updateColor();
        }

        /// MultiplexLayer

        LayerMultiplex::LayerMultiplex()
        : enabledLayer_(0)
        {
        }

        LayerMultiplex::~LayerMultiplex()
        {
            for(const auto &layer : layers_) {
                layer->cleanup();
            }
        }

        LayerMultiplex * LayerMultiplex::create(Layer * layer, ...)
        {
            va_list args;
            va_start(args,layer);

            LayerMultiplex * multiplexLayer = new (std::nothrow) LayerMultiplex();
            if(multiplexLayer && multiplexLayer->initWithLayers(layer, args))
            {
                multiplexLayer->autorelease();
                va_end(args);
                return multiplexLayer;
            }
            va_end(args);
            SAFE_DELETE(multiplexLayer);
            return nullptr;
        }

        LayerMultiplex * LayerMultiplex::createWithLayer(Layer* layer)
        {
            return LayerMultiplex::create(layer, nullptr);
        }

        LayerMultiplex* LayerMultiplex::create()
        {
            LayerMultiplex* ret = new (std::nothrow) LayerMultiplex();
            if (ret && ret->init())
            {
                ret->autorelease();
            }
            else
            {
                SAFE_DELETE(ret);
            }
            return ret;
        }

        LayerMultiplex* LayerMultiplex::createWithArray(const HObjectVector<Layer*>& arrayOfLayers)
        {
            LayerMultiplex* ret = new (std::nothrow) LayerMultiplex();
            if (ret && ret->initWithArray(arrayOfLayers))
            {
                ret->autorelease();
            }
            else
            {
                SAFE_DELETE(ret);
            }
            return ret;
        }

        void LayerMultiplex::addLayer(Layer* layer)
        {
            layers_.pushBack(layer);
        }

        bool LayerMultiplex::init()
        {
            if (Layer::init())
            {
                enabledLayer_ = 0;
                return true;
            }
            return false;
        }

        bool LayerMultiplex::initWithLayers(Layer *layer, va_list params)
        {
            if (Layer::init())
            {
                layers_.reserve(5);
                layers_.pushBack(layer);

                Layer *l = va_arg(params,Layer*);
                while( l ) {
                    layers_.pushBack(l);
                    l = va_arg(params,Layer*);
                }

                enabledLayer_ = 0;
                this->addChild(layers_.at(enabledLayer_));
                return true;
            }

            return false;
        }

        bool LayerMultiplex::initWithArray(const HObjectVector<Layer*>& arrayOfLayers)
        {
            if (Layer::init())
            {
                layers_.reserve(arrayOfLayers.size());
                layers_.pushBack(arrayOfLayers);

                enabledLayer_ = 0;
                this->addChild(layers_.at(enabledLayer_));
                return true;
            }
            return false;
        }

        void LayerMultiplex::switchTo(int n)
        {
            this->removeChild(layers_.at(enabledLayer_), true);

            enabledLayer_ = n;

            this->addChild(layers_.at(n));
        }

        void LayerMultiplex::switchToAndReleaseMe(int n)
        {
            this->removeChild(layers_.at(enabledLayer_), true);

            layers_.replace(enabledLayer_, nullptr);

            enabledLayer_ = n;

            this->addChild(layers_.at(n));
        }
    }
}
