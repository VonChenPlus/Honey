#ifndef UILAYER_H
#define UILAYER_H

#include "GRAPH/Node.h"
#include "GRAPH/Protocols.h"
#include "GRAPH/UNITY3D/RenderCommand.h"
#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/Event.h"

namespace GRAPH
{
    class EventListener;
    class EventListenerTouch;
    class EventListenerKeyboard;
    class EventListenerAcceleration;

    class Touch;

    namespace UI
    {
        class Layer : public Node
        {
        public:
            static Layer *create();

            virtual bool onTouchBegan(Touch *touch, Event *unused_event);
            /** Callback function for touch moved.
            *
            * @param touch Touch infomation.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchMoved(Touch *touch, Event *unused_event);
            /** Callback function for touch ended.
            *
            * @param touch Touch infomation.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchEnded(Touch *touch, Event *unused_event);
            /** Callback function for touch cancelled.
            *
            * @param touch Touch infomation.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchCancelled(Touch *touch, Event *unused_event);

            /** Callback function for multiple touches began.
            *
            * @param touches Touches information.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchesBegan(const std::vector<Touch*>& touches, Event *unused_event);
            /** Callback function for multiple touches moved.
            *
            * @param touches Touches information.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchesMoved(const std::vector<Touch*>& touches, Event *unused_event);
            /** Callback function for multiple touches ended.
            *
            * @param touches Touches information.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchesEnded(const std::vector<Touch*>& touches, Event *unused_event);
            /** Callback function for multiple touches cancelled.
            *
            * @param touches Touches information.
            * @param unused_event Event information.
            * @js NA
            */
            virtual void onTouchesCancelled(const std::vector<Touch*>&touches, Event *unused_event);

            /* Callback function should not be deprecated, it will generate lots of warnings.
            Since 'setAccelerometerEnabled' was deprecated, it will make warnings if developer overrides onAcceleration and invokes setAccelerometerEnabled(true) instead of using EventDispatcher::addEventListenerWithXXX.
            */
            /** Callback funtion for acceleration.
             * @param acc Acceleration information.
             * @param unused_event Event information.
             * @js NA
             */
            virtual void onAcceleration(Acceleration* acc, Event* unused_event);

            /* Callback function should not be deprecated, it will generate lots of warnings.
            Since 'setKeyboardEnabled' was deprecated, it will make warnings if developer overrides onKeyXXX and invokes setKeyboardEnabled(true) instead of using EventDispatcher::addEventListenerWithXXX.
            */
            /** Callback function for key pressed.
             * @param keyCode KeyCode information.
             * @param event Event information.
             * @js NA
             */
            virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
            /** Callback function for key released.
            * @param keyCode KeyCode information.
            * @param event Event information.
            * @js NA
            */
            virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

        public:
            Layer();
            virtual ~Layer();

            virtual bool init() override;

        protected:
            bool _touchEnabled;
            bool _accelerometerEnabled;
            bool _keyboardEnabled;
            EventListener* _touchListener;
            EventListenerKeyboard* _keyboardListener;
            EventListenerAcceleration* _accelerationListener;

            Touch::DispatchMode _touchMode;
            bool _swallowsTouches;

        private:
            DISALLOW_COPY_AND_ASSIGN(Layer)

        };

        class LayerRGBA : public Layer, public RGBAProtocol
        {
        public:
            static LayerRGBA* create()
            {
                LayerRGBA *pRet = new(std::nothrow) LayerRGBA();
                if (pRet && pRet->init())
                {
                    pRet->autorelease();
                    return pRet;
                }
                else
                {
                    delete pRet;
                    pRet = NULL;
                    return NULL;
                }
            }

            //
            // Overrides
            //
            virtual GLubyte getOpacity() const override { return Layer::getOpacity(); }
            virtual GLubyte getDisplayedOpacity() const override { return Layer::getDisplayedOpacity(); }
            virtual void setOpacity(GLubyte opacity) override { return Layer::setOpacity(opacity); }
            virtual void updateDisplayedOpacity(GLubyte parentOpacity) override { return Layer::updateDisplayedOpacity(parentOpacity); }
            virtual bool isCascadeOpacityEnabled() const override { return Layer::isCascadeOpacityEnabled(); }
            virtual void setCascadeOpacityEnabled(bool cascadeOpacityEnabled) override { return Layer::setCascadeOpacityEnabled(cascadeOpacityEnabled); }

            virtual const Color3B& getColor() const override { return Layer::getColor(); }
            virtual const Color3B& getDisplayedColor() const override { return Layer::getDisplayedColor(); }
            virtual void setColor(const Color3B& color) override { return Layer::setColor(color); }
            virtual void updateDisplayedColor(const Color3B& parentColor) override { return Layer::updateDisplayedColor(parentColor); }
            virtual bool isCascadeColorEnabled() const override { return Layer::isCascadeOpacityEnabled(); }
            virtual void setCascadeColorEnabled(bool cascadeColorEnabled) override { return Layer::setCascadeColorEnabled(cascadeColorEnabled); }

            virtual void setOpacityModifyRGB(bool bValue) override { return Layer::setOpacityModifyRGB(bValue); }
            virtual bool isOpacityModifyRGB() const override { return Layer::isOpacityModifyRGB(); }

        public:
            LayerRGBA();
            virtual ~LayerRGBA() {}

        private:
            DISALLOW_COPY_AND_ASSIGN(LayerRGBA)
        };

        class LayerColor : public Layer, public BlendProtocol
        {
        public:
            /** Creates a fullscreen black layer.
             *
             * @return An autoreleased LayerColor object.
             */
            static LayerColor* create();
            /** Creates a Layer with color, width and height in Points.
             *
             * @param color The color of layer.
             * @param width The width of layer.
             * @param height The height of layer.
             * @return An autoreleased LayerColor object.
             */
            static LayerColor * create(const Color4B& color, GLfloat width, GLfloat height);
            /** Creates a Layer with color. Width and height are the window size.
             *
             * @param color The color of layer.
             * @return An autoreleased LayerColor object.
             */
            static LayerColor * create(const Color4B& color);

            /** Change width in Points.
             *
             * @param w The width of layer.
             */
            void changeWidth(GLfloat w);
            /** Change height in Points.
             *
             * @param h The height of layer.
             */
            void changeHeight(GLfloat h);
            /** Change width and height in Points.
             *
             * @param w The width of layer.
             * @param h The Height of layer.
            @since v0.8
            */
            void changeWidthAndHeight(GLfloat w ,GLfloat h);

            //
            // Overrides
            //
            virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

            virtual void setContentSize(const MATH::Sizef & var) override;
            /** BlendFunction. Conforms to BlendProtocol protocol */
            /**
            * @lua NA
            */
            virtual const BlendFunc& getBlendFunc() const override;
            /**
            *@code
            *When this function bound into js or lua,the parameter will be changed
            *In js: var setBlendFunc(var src, var dst)
            *In lua: local setBlendFunc(local src, local dst)
            *@endcode
            */
            virtual void setBlendFunc(const BlendFunc& blendFunc) override;

        public:
            LayerColor();
            virtual ~LayerColor();

            bool init() override;
            bool initWithColor(const Color4B& color, GLfloat width, GLfloat height);
            bool initWithColor(const Color4B& color);

        protected:
            void onDraw(const MATH::Matrix4& transform, uint32_t flags);

            virtual void updateColor() override;

            BlendFunc _blendFunc;
            MATH::Vector2f _squareVertices[4];
            Color4F  _squareColors[4];
            CustomCommand _customCommand;
            MATH::Vector3f _noMVPVertices[4];
            Unity3DVertexFormat *u3dVertexFormat_;
            Unity3DContext *u3dContext_;

        private:
            DISALLOW_COPY_AND_ASSIGN(LayerColor)

        };

        class LayerGradient : public LayerColor
        {
        public:
            /** Creates a fullscreen black layer.
             *
             * @return An autoreleased LayerGradient object.
             */
            static LayerGradient* create();

            /** Creates a full-screen Layer with a gradient between start and end.
             *
             * @param start The start color.
             * @param end The end color.
             * @return An autoreleased LayerGradient object.
             */
            static LayerGradient* create(const Color4B& start, const Color4B& end);

            /** Creates a full-screen Layer with a gradient between start and end in the direction of v.
             *
             * @param start The start color.
             * @param end The end color.
             * @param v The direction of gradient color.
             * @return An autoreleased LayerGradient object.
             */
            static LayerGradient* create(const Color4B& start, const Color4B& end, const MATH::Vector2f& v);

            /** Whether or not the interpolation will be compressed in order to display all the colors of the gradient both in canonical and non canonical vectors.
             Default: true.
             *
             * @param compressedInterpolation The interpolation will be compressed if true.
             */
            void setCompressedInterpolation(bool compressedInterpolation);
            /** Get the compressedInterpolation
             *
             * @return The interpolation will be compressed if true.
             */
            bool isCompressedInterpolation() const;

            /** Sets the start color of the gradient.
             *
             * @param startColor The start color.
             */
            void setStartColor( const Color3B& startColor );
            /** Returns the start color of the gradient.
             *
             * @return The start color.
             */
            const Color3B& getStartColor() const;

            /** Sets the end color of the gradient.
             *
             * @param endColor The end color.
             */
            void setEndColor( const Color3B& endColor );
            /** Returns the end color of the gradient.
             *
             * @return The end color.
             */
            const Color3B& getEndColor() const;

            /** Returns the start opacity of the gradient.
             *
             * @param startOpacity The start opacity, from 0 to 255.
             */
            void setStartOpacity( GLubyte startOpacity );
            /** Returns the start opacity of the gradient.
             *
             * @return The start opacity.
             */
            GLubyte getStartOpacity() const;

            /** Returns the end opacity of the gradient.
             *
             * @param endOpacity The end opacity, from 0 to 255.
             */
            void setEndOpacity( GLubyte endOpacity );
            /** Returns the end opacity of the gradient.
             *
             * @return The end opacity.
             */
            GLubyte getEndOpacity() const;

            /** Sets the directional vector that will be used for the gradient.
            The default value is vertical direction (0,-1).
             *
             * @param alongVector The direction of gradient.
             */
            void setVector(const MATH::Vector2f& alongVector);
            /** Returns the directional vector used for the gradient.
             *
             * @return The direction of gradient.
             */
            const MATH::Vector2f& getVector() const;

        public:
            LayerGradient();
            virtual ~LayerGradient();

            virtual bool init() override;
            /** Initializes the Layer with a gradient between start and end.
             * @js init
             * @lua init
             */
            bool initWithColor(const Color4B& start, const Color4B& end);

            /** Initializes the Layer with a gradient between start and end in the direction of v.
             * @js init
             * @lua init
             */
            bool initWithColor(const Color4B& start, const Color4B& end, const MATH::Vector2f& v);

        protected:
            virtual void updateColor() override;

            Color3B _startColor;
            Color3B _endColor;
            GLubyte _startOpacity;
            GLubyte _endOpacity;
            MATH::Vector2f   _alongVector;
            bool    _compressedInterpolation;
        };

        class LayerMultiplex : public Layer
        {
        public:
            /** Creates and initializes a LayerMultiplex object.
             * @lua NA
             *
             * @return An autoreleased LayerMultiplex object.
             */
            static LayerMultiplex* create();

            /** Creates a LayerMultiplex with an array of layers.
             @since v2.1
             * @js NA
             *
             * @param arrayOfLayers An array of layers.
             * @return An autoreleased LayerMultiplex object.
             */
            static LayerMultiplex* createWithArray(const HObjectVector<Layer*>& arrayOfLayers);

            /** Creates a LayerMultiplex with one or more layers using a variable argument list.
             * @code
             * When this function bound to lua or js,the input params are changed.
             * In js:var create(...)
             * In lua:local create(...)
             * @endcode
             */
            static LayerMultiplex * create(Layer* layer, ... );

            /** Creates a LayerMultiplex with one layer.
             * Lua script can not init with undetermined number of variables
             * so add these functions to be used with lua.
             * @js NA
             * @lua NA
             *
             * @param layer A certain layer.
             * @return An autoreleased LayerMultiplex object.
             */
            static LayerMultiplex * createWithLayer(Layer* layer);


            /** Add a certain layer to LayerMultiplex.
             *
             * @param layer A layer need to be added to the LayerMultiplex.
             */
            void addLayer(Layer* layer);

            /** Switches to a certain layer indexed by n.
             The current (old) layer will be removed from it's parent with 'cleanup=true'.
             *
             * @param n The layer indexed by n will display.
             */
            void switchTo(int n);
            /** release the current layer and switches to another layer indexed by n.
            The current (old) layer will be removed from it's parent with 'cleanup=true'.
             *
             * @param n The layer indexed by n will display.
             */
            void switchToAndReleaseMe(int n);

        public:
            /**
             * @js ctor
             */
            LayerMultiplex();
            /**
             * @js NA
             * @lua NA
             */
            virtual ~LayerMultiplex();

            virtual bool init() override;
            /** initializes a MultiplexLayer with one or more layers using a variable argument list.
             * @js NA
             * @lua NA
             */
            bool initWithLayers(Layer* layer, va_list params);

            /** initializes a MultiplexLayer with an array of layers
             @since v2.1
             */
            bool initWithArray(const HObjectVector<Layer*>& arrayOfLayers);

        protected:
            unsigned int _enabledLayer;
            HObjectVector<Layer*>    _layers;

        private:
            DISALLOW_COPY_AND_ASSIGN(LayerMultiplex)
        };
    }
}

#endif // UILAYER_H

