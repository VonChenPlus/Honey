#ifndef LAYOUT_H
#define LAYOUT_H

#include "GRAPH/UI/BASE/UIWidget.h"
#include "GRAPH/RENDERER/RenderCommand.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/BASE/Component.h"

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

            /**
             * Clipping Type, default is STENCIL.
             */
            enum class ClippingType
            {
                STENCIL,
                SCISSOR
            };

            /**
             * Background color type, default is NONE.
             */
            enum class BackGroundColorType
            {
                NONE,
                SOLID,
                GRADIENT
            };

            /**
             * Default constructor
             * @js ctor
             * @lua new
             */
            Layout();

            /**
             * Default destructor
             * @js NA
             * @lua NA
             */
            virtual ~Layout();

            /**
             * Create a empty layout.
             */
            static Layout* create();

            /**
             * Sets a background image for layout.
             *
             * @param fileName image file path.
             * @param texType @see TextureResType.
             */
            void setBackGroundImage(const std::string& fileName,TextureResType texType = TextureResType::LOCAL);

            /**
             * Sets a background image capinsets for layout, it only affects the scale9 enabled background image
             *
             * @param capInsets  The capInsets in Rect.
             *
             */
            void setBackGroundImageCapInsets(const MATH::Rectf& capInsets);

            /**
             * Query background image's capInsets size.
             *@return The background image capInsets.
             */
            const MATH::Rectf& getBackGroundImageCapInsets()const;

            /**
             * Sets Color Type for layout's background
             *
             * @param type   @see `BackGroundColorType`
             */
            void setBackGroundColorType(BackGroundColorType type);

            /**
             * Query the layout's background color type.
             *@return The layout's background color type.
             */
            BackGroundColorType getBackGroundColorType()const;

            /**
             * Enable background image scale9 rendering.
             *
             * @param enabled  True means enable scale9 rendering for background image, false otherwise.
             */
            void setBackGroundImageScale9Enabled(bool enabled);

            /**
             * Query background image scale9 enable status.
             *@return Whehter background image is scale9 enabled or not.
             */
            bool isBackGroundImageScale9Enabled()const;

            /**
             * Set background color for layout
             * The color only applies to layout when it's color type is BackGroundColorType::SOLIDE
             *
             * @param color Color in Color3B.
             */
            void setBackGroundColor(const Color3B &color);

            /**
             * Query the layout's background color.
             *@return Background color in Color3B.
             */
            const Color3B& getBackGroundColor()const;

            /**
             * Set start and end background color for layout.
             * This setting only take effect when the layout's  color type is BackGroundColorType::GRADIENT
             *
             * @param startColor Color value in Color3B.
             * @param endColor Color value in Color3B.
             */
            void setBackGroundColor(const Color3B &startColor, const Color3B &endColor);

            /**
             * Get the gradient background start color.
             *@return  Gradient background start color value.
             */
            const Color3B& getBackGroundStartColor()const;

            /**
             * Get the gradient background end color.
             * @return Gradient background end color value.
             */
            const Color3B& getBackGroundEndColor()const;

            /**
             * Sets background color opacity of layout.
             *
             * @param opacity The opacity in `GLubyte`.
             */
            void setBackGroundColorOpacity(GLubyte opacity);

            /**
             * Get the layout's background color opacity.
             *@return Background color opacity value.
             */
            GLubyte getBackGroundColorOpacity()const;

            /**
             * Sets background color vector for layout.
             * This setting only take effect when layout's color type is BackGroundColorType::GRADIENT
             *
             * @param vector The color vector in `MATH::Vector2f`.
             */
            void setBackGroundColorVector(const MATH::Vector2f &vector);

            /**
             * Get the layout's background color vector.
             *@return Background color vector.
             */
            const MATH::Vector2f& getBackGroundColorVector()const;

            /**
             * Set layout's background image color.
             *@param color Background color value in `Color3B`.
             */
            void setBackGroundImageColor(const Color3B& color);

            /**
             * Set opacity of background image.
             *@param opacity Background image opacity in GLubyte.
             */
            void setBackGroundImageOpacity(GLubyte opacity);

            /**
             * Get color of layout's background image.
             *@return Layout's background image color.
             */
            const Color3B& getBackGroundImageColor()const;

            /**
             * Get the opacity of layout's background image.
             * @return The opacity of layout's background image.
             */
            GLubyte getBackGroundImageOpacity()const;

            /**
             * Remove the background image of layout.
             */
            void removeBackGroundImage();

            /**
             * Gets background image texture size.
             *
             * @return background image texture size.
             */
            const MATH::Sizef& getBackGroundImageTextureSize() const;

            /**
             * Toggle layout clipping.
             *
             * If you do need clipping, you pass true to this function.
             *
             * @param enabled Pass true to enable clipping, false otherwise.
             */
            virtual void setClippingEnabled(bool enabled);


            /**
             * Change the clipping type of layout.
             * On default, the clipping type is `ClippingType::STENCIL`.
             * @see `ClippingType`
             *@param type The clipping type of layout.
             */
            void setClippingType(ClippingType type);

            /**
             *
             * @see `setClippingType(ClippingType)`
             */
            ClippingType getClippingType()const;

            /**
             * Gets if layout is clipping enabled.
             *
             * @return if layout is clipping enabled.
             */
            virtual bool isClippingEnabled()const;

            /**
             * Change the layout type.
             *@param type Layout type.
             */
            virtual void setLayoutType(Type type);

            /**
             * Query layout type.
             *@return Get the layout type.
             */
            virtual  Type getLayoutType() const;


            virtual void addChild(Node* child)override;
            virtual void addChild(Node * child, int localZOrder)override;
            /**
             * Adds a child to the container with z order and tag
             *
             * If the child is added to a 'running' node, then 'onEnter' and 'onEnterTransitionDidFinish' will be called immediately.
             *
             * @param child     A child node
             * @param localZOrder    Z order for drawing priority. Please refer to setLocalZOrder(int)
             * @param tag       A interger to identify the node easily. Please refer to setTag(int)
             */
            virtual void addChild(Node* child, int localZOrder, int tag) override;
            virtual void addChild(Node* child, int localZOrder, const std::string &name) override;

            virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;

            virtual void removeChild(Node* child, bool cleanup = true) override;

            /**
             * Removes all children from the container with a cleanup.
             *
             * @see `removeAllChildrenWithCleanup(bool)`
             */
            virtual void removeAllChildren() override;
            /**
             * Removes all children from the container, and do a cleanup to all running actions depending on the cleanup parameter.
             *
             * @param cleanup   true if all running actions on all children nodes should be cleanup, false oterwise.
             * @js removeAllChildren
             * @lua removeAllChildren
             */
            virtual void removeAllChildrenWithCleanup(bool cleanup) override;

            /**
             * force refresh widget layout
             */
            virtual void forceDoLayout();

            /**
             * request to refresh widget layout
             */
            void requestDoLayout();

            /**
             * @lua NA
             */
            virtual void onEnter() override;

            /**
             * @lua NA
             */
            virtual void onExit() override;

            /**
             * If a layout is loop focused which means that the focus movement will be inside the layout
             *@param loop  pass true to let the focus movement loop inside the layout
             */
            void setLoopFocus(bool loop);

            /**
             *@return If focus loop is enabled, then it will return true, otherwise it returns false. The default value is false.
             */
            bool isLoopFocus()const;

            /**
             *@param pass To specify whether the layout pass its focus to its child
             */
            void setPassFocusToChild(bool pass);

            /**
             * @return To query whether the layout will pass the focus to its children or not. The default value is true
             */
            bool isPassFocusToChild()const;

            /**
             *  When a widget is in a layout, you could call this method to get the next focused widget within a specified direction.
             *  If the widget is not in a layout, it will return itself
             *@param direction the direction to look for the next focused widget in a layout
             *@param current  the current focused widget
             *@return the next focused widget in a layout
             */
            virtual Widget* findNextFocusedWidget(FocusDirection direction, Widget* current) override;

            /**
             * To specify a user-defined functor to decide which child widget of the layout should get focused
             * @param FocusDirection the finding direction
             * @param this previous focused widget
             * @return return the index of widget in the layout
             */
            std::function<int(FocusDirection, Widget*)> onPassFocusToChild;

            /**
             * Override function. Set camera mask, the node is visible by the camera whose camera flag & node's camera mask is true.
             * @param mask Mask being set
             * @param applyChildren If true call this function recursively from this node to its children.
             */
            virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

        public:
            //override "init" method of widget.
            virtual bool init() override;

        protected:
            //override "onSizeChanged" method of widget.
            virtual void onSizeChanged() override;

            //init background image renderer.
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

            //clipping
            void onBeforeVisitStencil();
            void onAfterDrawStencil();
            void onAfterVisitStencil();
            /**draw fullscreen quad to clear stencil bits
             */
            void drawFullScreenQuadClearStencil();

            void onBeforeVisitScissor();
            void onAfterVisitScissor();
            void updateBackGroundImageColor();
            void updateBackGroundImageOpacity();
            void updateBackGroundImageRGBA();

            /**
             *get the content size of the layout, it will accumulate all its children's content size
             */
            MATH::Sizef getLayoutAccumulatedSize() const;

            /**
             * When the layout get focused, it the layout pass the focus to its child, it will use this method to determine which child
             * will get the focus.  The current algorithm to determine which child will get focus is nearest-distance-priority algorithm
             *@param dir next focused widget direction
             *@return The index of child widget in the container
             */
             int findNearestChildWidgetIndex(FocusDirection direction, Widget* baseWidget);

            /**
             * When the layout get focused, it the layout pass the focus to its child, it will use this method to determine which child
             * will get the focus.  The current algorithm to determine which child will get focus is farthest-distance-priority algorithm
             *@param dir next focused widget direction
             *@return The index of child widget in the container
             */
            int findFarthestChildWidgetIndex(FocusDirection direction, Widget* baseWidget);

            /**
             * caculate the nearest distance between the baseWidget and the children of the layout
             *@param the base widget which will be used to caculate the distance between the layout's children and itself
             *@return return the nearest distance between the baseWidget and the layout's children
             */
            float calculateNearestDistance(Widget* baseWidget);

            /**
             * caculate the farthest distance between the baseWidget and the children of the layout
             *@param the base widget which will be used to caculate the distance between the layout's children and itself
             *@return return the farthest distance between the baseWidget and the layout's children
             */

            float calculateFarthestDistance(Widget* baseWidget);

            /**
             *  when a layout pass the focus to it's child, use this method to determine which algorithm to use, nearest or farthest distance algorithm or not
             */
            void findProperSearchingFunctor(FocusDirection dir, Widget* baseWidget);

            /**
             * find the first non-layout widget in this layout
             */
            Widget *findFirstNonLayoutWidget();

            /**
             * find the fisrt focus enabled widget index in the layout, it will recusive searching the child widget
             */
            int findFirstFocusEnabledWidgetIndex();

            /**
             * find a focus enabled child Widget in the layout by index
             */
            Widget* findFocusEnabledChildWidgetByIndex(ssize_t index);

            /**
             * get the center point of a widget in world space
             */
            MATH::Vector2f getWorldCenterPoint(Widget* node)const;

            /**
             * this method is called internally by nextFocusedWidget. When the dir is Right/Down, then this method will be called
             *@param dir  the direction.
             *@param current  the current focused widget
             *@return the next focused widget
             */
            Widget* getNextFocusedWidget(FocusDirection direction,Widget *current);

            /**
             * this method is called internally by nextFocusedWidget. When the dir is Left/Up, then this method will be called
             *@param dir  the direction.
             *@param current  the current focused widget
             *@return the next focused widget
             */
            Widget* getPreviousFocusedWidget(FocusDirection direction, Widget *current);

            /**
             * find the nth elment in the _children array. Only the Widget descendant object will be returned
             *@param index  The index of a element in the _children array
             */
            Widget* getChildWidgetByIndex(ssize_t index)const;
            /**
             * whether it is the last element according to all their parents
             */
            bool  isLastWidgetInContainer(Widget* widget, FocusDirection direction)const;

            /**Lookup any parent widget with a layout type as the direction,
             * if the layout is loop focused, then return true, otherwise
             * It returns false
             */
            bool  isWidgetAncestorSupportLoopFocus(Widget* widget, FocusDirection direction)const;

            /**
             * pass the focus to the layout's next focus enabled child
             */
            Widget* passFocusToChild(FocusDirection direction, Widget* current);

            /**
             * If there are no focus enabled child in the layout, it will return false, otherwise it returns true
             */
            bool checkFocusEnabledChild()const;

        protected:

            //background
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

            //clipping
            bool _clippingEnabled;
            Type _layoutType;
            ClippingType _clippingType;
            DrawNode* _clippingStencil;
            bool _scissorRectDirty;
            MATH::Rectf _clippingRect;
            Layout* _clippingParent;
            bool _clippingRectDirty;

            //clipping

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

            //whether enable loop focus or not
            bool _loopFocus;
            //on default, it will pass the focus to the next nearest widget
            bool _passFocusToChild;
             //when finding the next focused widget, use this variable to pass focus between layout & widget
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
            //initializes state of widget.
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
            //initializes state of widget.
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

            /**
             * The interface does the actual layouting work.
             */
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

            ssize_t _unlayoutChildCount;
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

                CREATE_FUNC(LayoutComponent)

                /**
                 * Bind a LayoutComponent to a specified node.
                 * If the node has already binded a LayoutComponent named __LAYOUT_COMPONENT_NAME, just return the LayoutComponent.
                 * Otherwise, create a new LayoutComponent and bind the LayoutComponent to the node.
                 *@param node A Node* instance pointer.
                 *@return The binded LayoutComponent instance pointer.
                 */
                static LayoutComponent* bindLayoutComponent(Node* node);

                /**
                 * Horizontal dock position type.
                 */
                enum class HorizontalEdge
                {
                    None,
                    Left,
                    Right,
                    Center
                };

                /**
                 * Vertical dock position type.
                 */
                enum class VerticalEdge
                {
                    None,
                    Bottom,
                    Top,
                    Center
                };

                /**
                 * Percent content size is used to adapt node's content size based on parent's content size.
                 * If set to true then node's content size will be changed based on the value setted by @see setPercentContentSize
                 *@param isUsed True to enable percent content size, false otherwise.
                 */
                void setUsingPercentContentSize(bool isUsed);

                /**
                 * Query whether use percent content size or not.
                 *@return True if using percent content size, false otherwise.
                 */
                bool getUsingPercentContentSize()const;

                /**
                 * Set percent content size.
                 * The value should be [0-1], 0 means the child's content size will be 0
                 * and 1 means the child's content size is the same as its parents.
                 *@param percent The percent (x,y) of the node in [0-1] scope.
                 */
                void setPercentContentSize(const MATH::Vector2f &percent);

                /**
                 * Query the percent content size value.
                 *@return Percet (x,y) in Vec2.
                 */
                MATH::Vector2f getPercentContentSize()const;

                /**
                 * Query the anchor position.
                 *@return Anchor position to it's parent
                 */
                const MATH::Vector2f& getAnchorPosition()const;

                /**
                 * Change the anchor position to it's parent.
                 *@param point A value in (x,y) format.
                 */
                void setAnchorPosition(const MATH::Vector2f& point);

                /**
                 * Query the owner's position.
                 *@return The owner's position.
                 */
                const MATH::Vector2f& getPosition()const;

                /**
                 * Change the position of component owner.
                 * @param position A position in (x,y)
                 */
                void setPosition(const MATH::Vector2f& position);

                /**
                 * Whether position percentX is enabled or not.
                 *@return True if position percertX is enable, false otherwise.
                 */
                bool isPositionPercentXEnabled()const;

                /**
                 * Toggle position percentX enabled.
                 *@param isUsed  True if enable position percentX, false otherwise.
                 */
                void setPositionPercentXEnabled(bool isUsed);

                /**
                 * Query the position percent X value.
                 *@return Position percent X value in float.
                 */
                float getPositionPercentX()const;

                /**
                 * Change position percent X value.
                 *@param percentMargin Margin in float.
                 */
                void setPositionPercentX(float percentMargin);

                /**
                 * Whether position percentY is enabled or not.
                 *@see `setPositionPercentYEnabled`
                 *@return True if position percentY is enabled, false otherwise.
                 */
                bool isPositionPercentYEnabled()const;

                /**
                 * Toggle position percentY enabled.
                 *@param isUsed True if position percentY is enabled, false otherwise.
                 */
                void setPositionPercentYEnabled(bool isUsed);

                /**
                 * Query the position percentY Y value.
                 *@return Position percent Y value in float.
                 */
                float getPositionPercentY()const;

                /**
                 * Change position percentY value.
                 *@param percentMargin Margin in float.
                 */
                void setPositionPercentY(float percentMargin);

                /**
                 * Query element horizontal dock type.
                 *@return Horizontal dock type.
                 */
                HorizontalEdge getHorizontalEdge()const;

                /**
                 * Change element's horizontal dock type.
                 *@param hEage Horizontal dock type @see `HorizontalEdge`
                 */
                void setHorizontalEdge(HorizontalEdge hEage);

                /**
                 * Query element vertical dock type.
                 *@return Vertical dock type.
                 */
                VerticalEdge getVerticalEdge()const;

                /**
                 * Change element's vertical dock type.
                 *@param vEage Vertical dock type @see `VerticalEdge`.
                 */
                void setVerticalEdge(VerticalEdge vEage);

                /**
                 * Query left margin of owner relative to its parent.
                 *@return Left margin in float.
                 */
                float getLeftMargin()const;

                /**
                 * Change left margin of owner relative to its parent.
                 *@param margin Margin in float.
                 */
                void setLeftMargin(float margin);

                /**
                 * Query the right margin of owner relative to its parent.
                 *@return Right margin in float.
                 */
                float getRightMargin()const;

                /**
                 * Change right margin of owner relative to its parent.
                 *@param margin Margin in float.
                 */
                void setRightMargin(float margin);

                /**
                 * Query the top margin of owner relative to its parent.
                 *@return Top margin in float.
                 */
                float getTopMargin()const;

                /**
                 * Change the top margin of owner relative to its parent.
                 *@param margin Margin in float.
                 */
                void setTopMargin(float margin);

                /**
                 * Query the bottom margin of owner relative to its parent.
                 *@return Bottom margin in float.
                 */
                float getBottomMargin()const;

                /**
                 * Change the bottom margin of owner relative to its parent.
                 *@param margin in float.
                 */
                void setBottomMargin(float margin);

                /**
                 * Query owner's content size.
                 *@return Owner's content size.
                 */
                const MATH::Sizef& getSize()const;

                /**
                 * Change the content size of owner.
                 *@param size Content size in @see `Size`.
                 */
                void setSize(const MATH::Sizef& size);

                /**
                 * Query whether percent width is enabled or not.
                 *@return True if percent width is enabled, false, otherwise.
                 */
                bool isPercentWidthEnabled()const;

                /**
                 * Toggle enable percent width.
                 *@param isUsed True if percent width is enabled, false otherwise.
                 */
                void setPercentWidthEnabled(bool isUsed);

                /**
                 * Query content size width of owner.
                 *@return Content size width in float.
                 */
                float getSizeWidth()const;

                /**
                 * Change content size width of owner.
                 *@param width Content size width in float.
                 */
                void setSizeWidth(float width);

                /**
                 * Query percent width of owner.
                 *@return percent width in float.
                 */
                float getPercentWidth()const;

                /**
                 * Change percent width of owner.
                 *@param percentWidth Percent Width in float.
                 */
                void setPercentWidth(float percentWidth);

                /**
                 * Query whehter percent height is enabled or not.
                 *@return True if percent height is enabled, false otherwise.
                 */
                bool isPercentHeightEnabled()const;

                /**
                 * Toggle enable percent height.
                 *@param isUsed True if percent height is enabled, false otherwise.
                 */
                void setPercentHeightEnabled(bool isUsed);

                /**
                 * Query size height of owner.
                 *@return Size height in float.
                 */
                float getSizeHeight()const;

                /**
                 * Change size height of owner.
                 *@param height Size height in float.
                 */
                void setSizeHeight(float height);

                /**
                 * Query percent hieght of owner.
                 *@return Percent height in float.
                 */
                float getPercentHeight()const;

                /**
                 * Change percent height value of owner.
                 *@param percentHeight Percent height in float.
                 */
                void setPercentHeight(float percentHeight);

                /**
                 * Query whether stretch width is enabled or not.
                 *@return True if stretch width is enabled, false otherwise.
                 */
                bool isStretchWidthEnabled()const;

                /**
                 * Toggle enable stretch width.
                 *@param isUsed True if enable stretch width, false otherwise.
                 */
                void setStretchWidthEnabled(bool isUsed);

                /**
                 * Query whether stretch height is enabled or not.
                 *@return True if stretch height is enabled, false otherwise.
                 */
                bool isStretchHeightEnabled()const;

                /**
                 * Toggle enable stretch height.
                 *@param isUsed True if stretch height is enabled, false otherwise.
                 */
                void setStretchHeightEnabled(bool isUsed);

                /**
                 * Toggle enable percent only.
                 *@param enable True if percent only is enabled, false otherwise.
                 */
                void setPercentOnlyEnabled(bool enable);

                /**
                 * Toggle active enabled of LayoutComponent's owner.
                 *@param enable True if active layout component, false otherwise.
                 */
                void setActiveEnabled(bool enable);

                /**
                 * Refresh layout of the owner.
                 */
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
