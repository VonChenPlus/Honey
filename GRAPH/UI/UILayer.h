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
            virtual void onTouchMoved(Touch *touch, Event *unused_event);
            virtual void onTouchEnded(Touch *touch, Event *unused_event);
            virtual void onTouchCancelled(Touch *touch, Event *unused_event);
            virtual void onTouchesBegan(const std::vector<Touch*>& touches, Event *unused_event);
            virtual void onTouchesMoved(const std::vector<Touch*>& touches, Event *unused_event);
            virtual void onTouchesEnded(const std::vector<Touch*>& touches, Event *unused_event);
            virtual void onTouchesCancelled(const std::vector<Touch*>&touches, Event *unused_event);
            virtual void onAcceleration(Acceleration* acc, Event* unused_event);
            virtual void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
            virtual void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);

        public:
            Layer();
            virtual ~Layer();

            virtual bool init() override;

        protected:
            bool touchEnabled_;
            bool accelerometerEnabled_;
            bool keyboardEnabled_;
            EventListener* touchListener_;
            EventListenerKeyboard* keyboardListener_;
            EventListenerAcceleration* accelerationListener_;

            Touch::DispatchMode touchMode_;
            bool swallowsTouches_;

        private:
            DISALLOW_COPY_AND_ASSIGN(Layer)

        };

        class LayerRGBA : public Layer, public RGBAProtocol
        {
        public:
            static LayerRGBA* create() {
                LayerRGBA *pRet = new(std::nothrow) LayerRGBA();
                if (pRet && pRet->init()) {
                    pRet->autorelease();
                    return pRet;
                }
                else {
                    delete pRet;
                    return nullptr;
                }
            }

            virtual uint8 getOpacity() const override { return Layer::getOpacity(); }
            virtual uint8 getDisplayedOpacity() const override { return Layer::getDisplayedOpacity(); }
            virtual void setOpacity(uint8 opacity) override { return Layer::setOpacity(opacity); }
            virtual void updateDisplayedOpacity(uint8 parentOpacity) override { return Layer::updateDisplayedOpacity(parentOpacity); }
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
            static LayerColor* create();
            static LayerColor * create(const Color4B& color, float width, float height);
            static LayerColor * create(const Color4B& color);

            void changeWidth(float w);
            void changeHeight(float h);
            void changeWidthAndHeight(float w ,float h);

            virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

            virtual void setContentSize(const MATH::Sizef & var) override;

            virtual const BlendFunc& getBlendFunc() const override;
            virtual void setBlendFunc(const BlendFunc& blendFunc) override;

        public:
            LayerColor();
            virtual ~LayerColor();

            bool init() override;
            bool initWithColor(const Color4B& color, float width, float height);
            bool initWithColor(const Color4B& color);

        protected:
            void onDraw(const MATH::Matrix4& transform, uint32_t flags);

            virtual void updateColor() override;

            BlendFunc blendFunc_;
            MATH::Vector2f squareVertices_[4];
            Color4F  squareColors_[4];
            CustomCommand customCommand_;
            MATH::Vector3f noMVPVertices_[4];
            Unity3DVertexFormat *u3dVertexFormat_;
            Unity3DContext *u3dContext_;

        private:
            DISALLOW_COPY_AND_ASSIGN(LayerColor)

        };

        class LayerGradient : public LayerColor
        {
        public:
            static LayerGradient* create();
            static LayerGradient* create(const Color4B& start, const Color4B& end);
            static LayerGradient* create(const Color4B& start, const Color4B& end, const MATH::Vector2f& v);

            void setCompressedInterpolation(bool compressedInterpolation);
            bool isCompressedInterpolation() const;

            void setStartColor( const Color3B& startColor );
            const Color3B& getStartColor() const;

            void setEndColor( const Color3B& endColor );
            const Color3B& getEndColor() const;

            void setStartOpacity( uint8 startOpacity );
            uint8 getStartOpacity() const;

            void setEndOpacity( uint8 endOpacity );
            uint8 getEndOpacity() const;

            void setVector(const MATH::Vector2f& alongVector);
            const MATH::Vector2f& getVector() const;

        public:
            LayerGradient();
            virtual ~LayerGradient();

            virtual bool init() override;
            bool initWithColor(const Color4B& start, const Color4B& end);
            bool initWithColor(const Color4B& start, const Color4B& end, const MATH::Vector2f& v);

        protected:
            virtual void updateColor() override;

            Color3B startColor_;
            Color3B endColor_;
            uint8 startOpacity_;
            uint8 endOpacity_;
            MATH::Vector2f   alongVector_;
            bool    compressedInterpolation_;
        };

        class LayerMultiplex : public Layer
        {
        public:
            static LayerMultiplex* create();
            static LayerMultiplex* createWithArray(const HObjectVector<Layer*>& arrayOfLayers);
            static LayerMultiplex * create(Layer* layer, ... );
            static LayerMultiplex * createWithLayer(Layer* layer);

            void addLayer(Layer* layer);

            void switchTo(int n);
            void switchToAndReleaseMe(int n);

        public:
            LayerMultiplex();
            virtual ~LayerMultiplex();

            virtual bool init() override;
            bool initWithLayers(Layer* layer, va_list params);
            bool initWithArray(const HObjectVector<Layer*>& arrayOfLayers);

        protected:
            unsigned int enabledLayer_;
            HObjectVector<Layer*>    layers_;

        private:
            DISALLOW_COPY_AND_ASSIGN(LayerMultiplex)
        };
    }
}

#endif // UILAYER_H

