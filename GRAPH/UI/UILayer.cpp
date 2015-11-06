#include <stdarg.h>
#include "GRAPH/UI/UILayer.h"
#include "GRAPH/Director.h"
#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "GRAPH/UNITY3D/Renderer.h"

namespace GRAPH
{
    namespace UI
    {
        // Layer
        Layer::Layer()
        : _touchEnabled(false)
        , _accelerometerEnabled(false)
        , _keyboardEnabled(false)
        , _touchListener(nullptr)
        , _keyboardListener(nullptr)
        , _accelerationListener(nullptr)
        , _touchMode(Touch::DispatchMode::ALL_AT_ONCE)
        , _swallowsTouches(true)
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
            _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
        }

        LayerColor::~LayerColor()
        {
        }

        /// blendFunc getter
        const BlendFunc &LayerColor::getBlendFunc() const
        {
            return _blendFunc;
        }
        /// blendFunc setter
        void LayerColor::setBlendFunc(const BlendFunc &var)
        {
            _blendFunc = var;
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

        bool LayerColor::initWithColor(const Color4B& color, GLfloat w, GLfloat h)
        {
            if (Layer::init())
            {

                // default blend function
                _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;

                displayedColor_.red = realColor_.red = color.red;
                displayedColor_.green = realColor_.green = color.green;
                displayedColor_.blue = realColor_.blue = color.blue;
                displayedOpacity_ = realOpacity_ = color.alpha;

                for (uint64 i = 0; i<sizeof(_squareVertices) / sizeof( _squareVertices[0]); i++ )
                {
                    _squareVertices[i].x = 0.0f;
                    _squareVertices[i].y = 0.0f;
                }

                updateColor();
                setContentSize(MATH::Sizef(w, h));

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
            _squareVertices[1].x = size.width;
            _squareVertices[2].y = size.height;
            _squareVertices[3].x = size.width;
            _squareVertices[3].y = size.height;

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
                _squareColors[i].red = displayedColor_.red / 255.0f;
                _squareColors[i].green = displayedColor_.green / 255.0f;
                _squareColors[i].blue = displayedColor_.blue / 255.0f;
                _squareColors[i].alpha = displayedOpacity_ / 255.0f;
            }
        }

        void LayerColor::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
        {
            _customCommand.init(globalZOrder_, transform, flags);
            _customCommand.func = std::bind(&LayerColor::onDraw, this, transform, flags);
            renderer->addCommand(&_customCommand);

            for(int i = 0; i < 4; ++i)
            {
                MATH::Vector4f pos;
                pos.x = _squareVertices[i].x; pos.y = _squareVertices[i].y; pos.z = positionZ_;
                pos.w = 1;
                modelViewTransform_.transformVector(&pos);
                _noMVPVertices[i] = MATH::Vector3f(pos.x,pos.y,pos.z)/pos.w;
            }
        }

        void LayerColor::onDraw(const MATH::Matrix4& transform, uint32_t)
        {
            getU3DShader()->apply();
            getU3DShader()->setUniformsForBuiltins(transform);

            GLStateCache::EnableVertexAttribs(VERTEX_ATTRIB_FLAG_POSITION | VERTEX_ATTRIB_FLAG_COLOR );

            //
            // Attributes
            //
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(SEM_POSITION, 3, GL_FLOAT, GL_FALSE, 0, _noMVPVertices);
            glVertexAttribPointer(SEM_COLOR0, 4, GL_FLOAT, GL_FALSE, 0, _squareColors);

            GLStateCache::BlendFunc( _blendFunc.src, _blendFunc.dst );

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        //
        // LayerGradient
        //
        LayerGradient::LayerGradient()
        : _startColor(Color4B::BLACK)
        , _endColor(Color4B::BLACK)
        , _startOpacity(255)
        , _endOpacity(255)
        , _alongVector(MATH::Vector2f(0, -1))
        , _compressedInterpolation(true)
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
            _endColor.red  = end.red;
            _endColor.green  = end.green;
            _endColor.blue  = end.blue;

            _endOpacity     = end.alpha;
            _startOpacity   = start.alpha;
            _alongVector    = v;

            _compressedInterpolation = true;

            return LayerColor::initWithColor(Color4B(start.red, start.green, start.blue, 255));
        }

        void LayerGradient::updateColor()
        {
            LayerColor::updateColor();

            float h = _alongVector.length();
            if (h == 0)
                return;

            float c = sqrtf(2.0f);
            MATH::Vector2f u(_alongVector.x / h, _alongVector.y / h);

            // Compressed Interpolation mode
            if (_compressedInterpolation)
            {
                float h2 = 1 / ( fabsf(u.x) + fabsf(u.y) );
                u = u * (h2 * (float)c);
            }

            float opacityf = (float)displayedOpacity_ / 255.0f;

            Color4F S(
                displayedColor_.red / 255.0f,
                displayedColor_.green / 255.0f,
                displayedColor_.blue / 255.0f,
                _startOpacity * opacityf / 255.0f
            );

            Color4F E(
                _endColor.red / 255.0f,
                _endColor.green / 255.0f,
                _endColor.blue / 255.0f,
                _endOpacity * opacityf / 255.0f
            );

            // (-1, -1)
            _squareColors[0].red = E.red + (S.red - E.red) * ((c + u.x + u.y) / (2.0f * c));
            _squareColors[0].green = E.green + (S.green - E.green) * ((c + u.x + u.y) / (2.0f * c));
            _squareColors[0].blue = E.blue + (S.blue - E.blue) * ((c + u.x + u.y) / (2.0f * c));
            _squareColors[0].alpha = E.alpha + (S.alpha - E.alpha) * ((c + u.x + u.y) / (2.0f * c));
            // (1, -1)
            _squareColors[1].red = E.red + (S.red - E.red) * ((c - u.x + u.y) / (2.0f * c));
            _squareColors[1].green = E.green + (S.green - E.green) * ((c - u.x + u.y) / (2.0f * c));
            _squareColors[1].blue = E.blue + (S.blue - E.blue) * ((c - u.x + u.y) / (2.0f * c));
            _squareColors[1].alpha = E.alpha + (S.alpha - E.alpha) * ((c - u.x + u.y) / (2.0f * c));
            // (-1, 1)
            _squareColors[2].red = E.red + (S.red - E.red) * ((c + u.x - u.y) / (2.0f * c));
            _squareColors[2].green = E.green + (S.green - E.green) * ((c + u.x - u.y) / (2.0f * c));
            _squareColors[2].blue = E.blue + (S.blue - E.blue) * ((c + u.x - u.y) / (2.0f * c));
            _squareColors[2].alpha = E.alpha + (S.alpha - E.alpha) * ((c + u.x - u.y) / (2.0f * c));
            // (1, 1)
            _squareColors[3].red = E.red + (S.red - E.red) * ((c - u.x - u.y) / (2.0f * c));
            _squareColors[3].green = E.green + (S.green - E.green) * ((c - u.x - u.y) / (2.0f * c));
            _squareColors[3].blue = E.blue + (S.blue - E.blue) * ((c - u.x - u.y) / (2.0f * c));
            _squareColors[3].alpha = E.alpha + (S.alpha - E.alpha) * ((c - u.x - u.y) / (2.0f * c));
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
            _endColor = color;
            updateColor();
        }

        const Color3B& LayerGradient::getEndColor() const
        {
            return _endColor;
        }

        void LayerGradient::setStartOpacity(GLubyte o)
        {
            _startOpacity = o;
            updateColor();
        }

        GLubyte LayerGradient::getStartOpacity() const
        {
            return _startOpacity;
        }

        void LayerGradient::setEndOpacity(GLubyte o)
        {
            _endOpacity = o;
            updateColor();
        }

        GLubyte LayerGradient::getEndOpacity() const
        {
            return _endOpacity;
        }

        void LayerGradient::setVector(const MATH::Vector2f& var)
        {
            _alongVector = var;
            updateColor();
        }

        const MATH::Vector2f& LayerGradient::getVector() const
        {
            return _alongVector;
        }

        bool LayerGradient::isCompressedInterpolation() const
        {
            return _compressedInterpolation;
        }

        void LayerGradient::setCompressedInterpolation(bool compress)
        {
            _compressedInterpolation = compress;
            updateColor();
        }

        /// MultiplexLayer

        LayerMultiplex::LayerMultiplex()
        : _enabledLayer(0)
        {
        }

        LayerMultiplex::~LayerMultiplex()
        {
            for(const auto &layer : _layers) {
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
            _layers.pushBack(layer);
        }

        bool LayerMultiplex::init()
        {
            if (Layer::init())
            {
                _enabledLayer = 0;
                return true;
            }
            return false;
        }

        bool LayerMultiplex::initWithLayers(Layer *layer, va_list params)
        {
            if (Layer::init())
            {
                _layers.reserve(5);
                _layers.pushBack(layer);

                Layer *l = va_arg(params,Layer*);
                while( l ) {
                    _layers.pushBack(l);
                    l = va_arg(params,Layer*);
                }

                _enabledLayer = 0;
                this->addChild(_layers.at(_enabledLayer));
                return true;
            }

            return false;
        }

        bool LayerMultiplex::initWithArray(const HObjectVector<Layer*>& arrayOfLayers)
        {
            if (Layer::init())
            {
                _layers.reserve(arrayOfLayers.size());
                _layers.pushBack(arrayOfLayers);

                _enabledLayer = 0;
                this->addChild(_layers.at(_enabledLayer));
                return true;
            }
            return false;
        }

        void LayerMultiplex::switchTo(int n)
        {
            this->removeChild(_layers.at(_enabledLayer), true);

            _enabledLayer = n;

            this->addChild(_layers.at(n));
        }

        void LayerMultiplex::switchToAndReleaseMe(int n)
        {
            this->removeChild(_layers.at(_enabledLayer), true);

            _layers.replace(_enabledLayer, nullptr);

            _enabledLayer = n;

            this->addChild(_layers.at(n));
        }
    }
}
