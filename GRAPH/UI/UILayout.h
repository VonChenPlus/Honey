#ifndef LAYOUT_H
#define LAYOUT_H

#include "GRAPH/Node.h"
#include "GRAPH/Component.h"
#include "GRAPH/RENDERER/RenderCommand.h"
#include "GRAPH/UI/BASE/UIWidget.h"

namespace GRAPH
{
    class DrawNode;
    class LayerColor;
    class LayerGradient;

    namespace UI
    {
        class Scale9Sprite;
        class LayoutManager;

        class LayoutProtocol
        {
        public:
            LayoutProtocol(){}
            virtual ~LayoutProtocol(){}

            virtual LayoutManager* createLayoutManager() = 0;

            virtual MATH::Sizef getLayoutContentSize()const = 0;
            virtual const HObjectVector<Node*>& getLayoutElements()const = 0;
            virtual void doLayout() = 0;
        };

        class Layout : public Widget, public LayoutProtocol
        {

            DECLARE_CLASS_GUI_INFO

        public:
            enum class Type
            {
                TABSOLUTE,
                TVERTICAL,
                THORIZONTAL,
                TRELATIVE
            };

            enum class ClippingType
            {
                STENCIL,
                SCISSOR
            };

            enum class BackGroundColorType
            {
                NONE,
                SOLID,
                GRADIENT
            };

            Layout();
            virtual ~Layout();

            static Layout* create();

            void setBackGroundImage(const std::string& fileName,TextureResType texType = TextureResType::LOCAL);
            void setBackGroundImageCapInsets(const MATH::Rectf& capInsets);

            const MATH::Rectf& getBackGroundImageCapInsets()const;
            void setBackGroundColorType(BackGroundColorType type);

            BackGroundColorType getBackGroundColorType()const;

            void setBackGroundImageScale9Enabled(bool enabled);
            bool isBackGroundImageScale9Enabled()const;

            void setBackGroundColor(const Color3B &color);
            const Color3B& getBackGroundColor()const;
            void setBackGroundColor(const Color3B &startColor, const Color3B &endColor);

            const Color3B& getBackGroundStartColor()const;
            const Color3B& getBackGroundEndColor()const;

            void setBackGroundColorOpacity(GLubyte opacity);
            GLubyte getBackGroundColorOpacity()const;

            void setBackGroundColorVector(const MATH::Vector2f &vector);
            const MATH::Vector2f& getBackGroundColorVector()const;

            void setBackGroundImageColor(const Color3B& color);
            const Color3B& getBackGroundImageColor()const;

            void setBackGroundImageOpacity(GLubyte opacity);
            GLubyte getBackGroundImageOpacity()const;

            void removeBackGroundImage();

            const MATH::Sizef& getBackGroundImageTextureSize() const;

            virtual void setClippingEnabled(bool enabled);

            void setClippingType(ClippingType type);
            ClippingType getClippingType()const;

            virtual bool isClippingEnabled()const;

            virtual void setLayoutType(Type type);
            virtual  Type getLayoutType() const;

            virtual void addChild(Node* child)override;
            virtual void addChild(Node * child, int localZOrder)override;
            virtual void addChild(Node* child, int localZOrder, int tag) override;
            virtual void addChild(Node* child, int localZOrder, const std::string &name) override;

            virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;

            virtual void removeChild(Node* child, bool cleanup = true) override;
            virtual void removeAllChildren() override;
            virtual void removeAllChildrenWithCleanup(bool cleanup) override;

            virtual void forceDoLayout();

            void requestDoLayout();

            virtual void onEnter() override;
            virtual void onExit() override;

            void setLoopFocus(bool loop);
            bool isLoopFocus()const;

            void setPassFocusToChild(bool pass);
            bool isPassFocusToChild()const;

            virtual Widget* findNextFocusedWidget(FocusDirection direction, Widget* current) override;

            std::function<int(FocusDirection, Widget*)> onPassFocusToChild;

            virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

        public:
            virtual bool init() override;

        protected:
            virtual void onSizeChanged() override;

            void addBackGroundImage();

            void supplyTheLayoutParameterLackToChild(Widget* child);
            virtual Widget* createCloneInstance() override;
            virtual void copySpecialProperties(Widget* model) override;
            virtual void copyClonedWidgetChildren(Widget* model) override;

            void stencilClippingVisit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags);
            void scissorClippingVisit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags);

            void setStencilClippingSize(const MATH::Sizef& size);
            const MATH::Rectf& getClippingRect();

            virtual void doLayout()override;
            virtual LayoutManager* createLayoutManager()override;
            virtual MATH::Sizef getLayoutContentSize()const override;
            virtual const HObjectVector<Node*>& getLayoutElements()const override;

            void onBeforeVisitStencil();
            void onAfterDrawStencil();
            void onAfterVisitStencil();

            void drawFullScreenQuadClearStencil();

            void onBeforeVisitScissor();
            void onAfterVisitScissor();
            void updateBackGroundImageColor();
            void updateBackGroundImageOpacity();
            void updateBackGroundImageRGBA();

            MATH::Sizef getLayoutAccumulatedSize() const;

            int findNearestChildWidgetIndex(FocusDirection direction, Widget* baseWidget);
            int findFarthestChildWidgetIndex(FocusDirection direction, Widget* baseWidget);

            float calculateNearestDistance(Widget* baseWidget);
            float calculateFarthestDistance(Widget* baseWidget);

            void findProperSearchingFunctor(FocusDirection dir, Widget* baseWidget);

            Widget *findFirstNonLayoutWidget();

            int findFirstFocusEnabledWidgetIndex();

            Widget* findFocusEnabledChildWidgetByIndex(int64 index);

            MATH::Vector2f getWorldCenterPoint(Widget* node)const;

            Widget* getNextFocusedWidget(FocusDirection direction,Widget *current);
            Widget* getPreviousFocusedWidget(FocusDirection direction, Widget *current);

            Widget* getChildWidgetByIndex(int64 index)const;
            bool  isLastWidgetInContainer(Widget* widget, FocusDirection direction)const;
            bool  isWidgetAncestorSupportLoopFocus(Widget* widget, FocusDirection direction)const;

            Widget* passFocusToChild(FocusDirection direction, Widget* current);

            bool checkFocusEnabledChild()const;

        protected:
            bool _backGroundScale9Enabled;
            Scale9Sprite* _backGroundImage;
            std::string _backGroundImageFileName;
            MATH::Rectf _backGroundImageCapInsets;
            BackGroundColorType _colorType;
            TextureResType _bgImageTexType;
            MATH::Sizef _backGroundImageTextureSize;
            Color3B _backGroundImageColor;
            GLubyte _backGroundImageOpacity;

            LayerColor* _colorRender;
            LayerGradient* _gradientRender;
            Color3B _cColor;
            Color3B _gStartColor;
            Color3B _gEndColor;
            MATH::Vector2f _alongVector;
            GLubyte _cOpacity;

            bool _clippingEnabled;
            Type _layoutType;
            ClippingType _clippingType;
            DrawNode* _clippingStencil;
            bool _scissorRectDirty;
            MATH::Rectf _clippingRect;
            Layout* _clippingParent;
            bool _clippingRectDirty;

            GLboolean _currentStencilEnabled;
            GLuint _currentStencilWriteMask;
            GLenum _currentStencilFunc;
            GLint _currentStencilRef;
            GLuint _currentStencilValueMask;
            GLenum _currentStencilFail;
            GLenum _currentStencilPassDepthFail;
            GLenum _currentStencilPassDepthPass;
            GLboolean _currentDepthWriteMask;

            GLboolean _currentAlphaTestEnabled;
            GLenum _currentAlphaTestFunc;
            GLclampf _currentAlphaTestRef;


            GLint _mask_layer_le;
            GroupCommand _groupCommand;
            CustomCommand _beforeVisitCmdStencil;
            CustomCommand _afterDrawStencilCmd;
            CustomCommand _afterVisitCmdStencil;
            CustomCommand _beforeVisitCmdScissor;
            CustomCommand _afterVisitCmdScissor;

            bool _doLayoutDirty;
            bool _isInterceptTouch;

            bool _loopFocus;
            bool _passFocusToChild;
            bool _isFocusPassing;
        };

        class HBox : public Layout
        {
        public:
            HBox();
            virtual ~HBox();

            static HBox* create();
            static HBox* create(const MATH::Sizef& size);

        public:
            virtual bool init() override;
            virtual bool initWithSize(const MATH::Sizef& size);
        };

        class VBox : public Layout
        {
        public:
            VBox();
            virtual ~VBox();

            static VBox* create();
            static VBox* create(const MATH::Sizef& size);

        public:
            virtual bool init() override;
            virtual bool initWithSize(const MATH::Sizef& size);
        };

        class RelativeBox : public Layout
        {
        public:
            RelativeBox();
            virtual ~RelativeBox();

            static RelativeBox* create();
            static RelativeBox* create(const MATH::Sizef& size);

        public:
            //initializes state of widget.
            virtual bool init() override;
            virtual bool initWithSize(const MATH::Sizef& size);
        };

        class LayoutManager : public HObject
        {
        public:
            virtual ~LayoutManager(){}
            LayoutManager(){}

            virtual void doLayout(LayoutProtocol *layout) = 0;

            friend class Layout;
        };

        class LinearVerticalLayoutManager : public LayoutManager
        {
        private:
            LinearVerticalLayoutManager(){}
            virtual ~LinearVerticalLayoutManager(){}
            static LinearVerticalLayoutManager* create();
            virtual void doLayout(LayoutProtocol *layout) override;

            friend class Layout;
        };

        class LinearHorizontalLayoutManager : public LayoutManager
        {
        private:
            LinearHorizontalLayoutManager(){}
            virtual ~LinearHorizontalLayoutManager(){}
            static LinearHorizontalLayoutManager* create();
            virtual void doLayout(LayoutProtocol *layout) override;

            friend class Layout;
        };

        class RelativeLayoutManager : public LayoutManager
        {
        private:
            RelativeLayoutManager()
            :_unlayoutChildCount(0),
            _widget(nullptr),
            _finalPositionX(0.0f),
            _finalPositionY(0.0f),
            _relativeWidgetLP(nullptr)
            {}
            virtual ~RelativeLayoutManager(){}
            static RelativeLayoutManager* create();
            virtual void doLayout(LayoutProtocol *layout) override;

            HObjectVector<Widget*> getAllWidgets(LayoutProtocol *layout);
            Widget* getRelativeWidget(Widget* widget);
            bool caculateFinalPositionWithRelativeWidget(LayoutProtocol *layout);
            void caculateFinalPositionWithRelativeAlign();

            int64 _unlayoutChildCount;
            HObjectVector<Widget*> _widgetChildren;
            Widget* _widget;
            float _finalPositionX;
            float _finalPositionY;

            RelativeLayoutParameter* _relativeWidgetLP;

            friend class Layout;
        };

        #define __LAYOUT_COMPONENT_NAME "__ui_layout"
        class LayoutComponent : public Component
            {
            public:
                LayoutComponent();
                ~LayoutComponent();

                virtual bool init()override;

                static LayoutComponent* create()
                {
                    LayoutComponent *pRet = new(std::nothrow) LayoutComponent();
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

                static LayoutComponent* bindLayoutComponent(Node* node);

                enum class HorizontalEdge
                {
                    None,
                    Left,
                    Right,
                    Center
                };

                enum class VerticalEdge
                {
                    None,
                    Bottom,
                    Top,
                    Center
                };

                void setUsingPercentContentSize(bool isUsed);
                bool getUsingPercentContentSize()const;

                void setPercentContentSize(const MATH::Vector2f &percent);
                MATH::Vector2f getPercentContentSize()const;

                const MATH::Vector2f& getAnchorPosition()const;
                void setAnchorPosition(const MATH::Vector2f& point);

                const MATH::Vector2f& getPosition()const;
                void setPosition(const MATH::Vector2f& position);

                bool isPositionPercentXEnabled()const;
                void setPositionPercentXEnabled(bool isUsed);

                float getPositionPercentX()const;
                void setPositionPercentX(float percentMargin);

                bool isPositionPercentYEnabled()const;
                void setPositionPercentYEnabled(bool isUsed);

                float getPositionPercentY()const;
                void setPositionPercentY(float percentMargin);

                HorizontalEdge getHorizontalEdge()const;
                void setHorizontalEdge(HorizontalEdge hEage);

                VerticalEdge getVerticalEdge()const;
                void setVerticalEdge(VerticalEdge vEage);

                float getLeftMargin()const;
                void setLeftMargin(float margin);

                float getRightMargin()const;
                void setRightMargin(float margin);

                float getTopMargin()const;
                void setTopMargin(float margin);

                float getBottomMargin()const;
                void setBottomMargin(float margin);

                const MATH::Sizef& getSize()const;
                void setSize(const MATH::Sizef& size);

                bool isPercentWidthEnabled()const;
                void setPercentWidthEnabled(bool isUsed);

                float getSizeWidth()const;
                void setSizeWidth(float width);

                float getPercentWidth()const;
                void setPercentWidth(float percentWidth);

                bool isPercentHeightEnabled()const;
                void setPercentHeightEnabled(bool isUsed);

                float getSizeHeight()const;
                void setSizeHeight(float height);

                float getPercentHeight()const;
                void setPercentHeight(float percentHeight);

                bool isStretchWidthEnabled()const;
                void setStretchWidthEnabled(bool isUsed);

                bool isStretchHeightEnabled()const;
                void setStretchHeightEnabled(bool isUsed);

                void setPercentOnlyEnabled(bool enable);

                void setActiveEnabled(bool enable);

                void refreshLayout();

            protected:
                Node* getOwnerParent();
                void refreshHorizontalMargin();
                void refreshVerticalMargin();
            protected:
                HorizontalEdge  _horizontalEdge;
                VerticalEdge    _verticalEdge;

                float           _leftMargin;
                float           _rightMargin;
                float           _bottomMargin;
                float           _topMargin;

                bool            _usingPositionPercentX;
                float           _positionPercentX;
                bool            _usingPositionPercentY;
                float           _positionPercentY;

                bool            _usingStretchWidth;
                bool            _usingStretchHeight;

                float           _percentWidth;
                bool            _usingPercentWidth;

                float           _percentHeight;
                bool            _usingPercentHeight;

                bool            _actived;
                bool            _isPercentOnly;
            };
    }
}
    
#endif // LAYOUT_H
