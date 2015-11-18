#include "GRAPH/UI/UILayout.h"
#include "GRAPH/DrawNode.h"
#include "GRAPH/Director.h"
#include "GRAPH/UNITY3D/Renderer.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"
#include "MATH/AffineTransform.h"
#include "GRAPH/UNITY3D/ShaderCache.h"
#include "GRAPH/UI/UIScale9Sprite.h"
#include "GRAPH/UI/UILayer.h"
#include "GRAPH/RenderView.h"
#include "GRAPH/UI/UIHelper.h"

namespace GRAPH
{
    namespace UI
    {
        static const int BACKGROUNDIMAGE_Z = (-1);
        static const int BCAKGROUNDCOLORRENDERER_Z = (-2);

        static GLint g_sStencilBits = -1;
        static GLint s_layer = -1;

        IMPLEMENT_CLASS_GUI_INFO(Layout)

        Layout::Layout():
        backGroundScale9Enabled_(false),
        backGroundImage_(nullptr),
        backGroundImageFileName_(""),
        backGroundImageCapInsets_(MATH::RectfZERO),
        colorType_(BackGroundColorType::NONE),
        bgImageTexType_(TextureResType::LOCAL),
        backGroundImageTextureSize_(MATH::SizefZERO),
        backGroundImageColor_(Color3B::WHITE),
        backGroundImageOpacity_(255),
        colorRender_(nullptr),
        gradientRender_(nullptr),
        cColor_(Color3B::WHITE),
        gStartColor_(Color3B::WHITE),
        gEndColor_(Color3B::WHITE),
        alongVector_(MATH::Vector2f(0.0f, -1.0f)),
        cOpacity_(255),
        clippingEnabled_(false),
        layoutType_(Type::TABSOLUTE),
        clippingType_(ClippingType::STENCIL),
        clippingStencil_(nullptr),
        scissorRectDirty_(false),
        clippingRect_(MATH::RectfZERO),
        clippingParent_(nullptr),
        clippingRectDirty_(true),
        groupCommand_(Director::getInstance().getRenderer()),
        doLayoutDirty_(true),
        isInterceptTouch_(false),
        loopFocus_(false),
        passFocusToChild_(true),
        isFocusPassing_(false),
        u3dContext_(Unity3DCreator::CreateContext())
        {
            //no-op
        }

        Layout::~Layout()
        {
            SAFE_RELEASE(clippingStencil_);
            SAFE_RELEASE(u3dVertexFormat_);
            SAFE_RELEASE(u3dContext_);
        }

        void Layout::onEnter()
        {
            Widget::onEnter();
            if (clippingStencil_)
            {
                clippingStencil_->onEnter();
            }
            doLayoutDirty_ = true;
            clippingRectDirty_ = true;
        }

        void Layout::onExit()
        {
            Widget::onExit();
            if (clippingStencil_)
            {
                clippingStencil_->onExit();
            }
        }

        Layout* Layout::create()
        {
            Layout* layout = new (std::nothrow) Layout();
            if (layout && layout->init())
            {
                layout->autorelease();
                return layout;
            }
            SAFE_DELETE(layout);
            return nullptr;
        }

        bool Layout::init()
        {
            if (Widget::init())
            {
                ignoreContentAdaptWithSize(false);
                setContentSize(MATH::SizefZERO);
                setAnchorPoint(MATH::Vec2fZERO);
                onPassFocusToChild = std::bind(&Layout::findNearestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);

                MATH::Vector2f vertices [] =
                {
                    MATH::Vector2f(-1, -1),
                    MATH::Vector2f(1, -1),
                    MATH::Vector2f(1, 1),
                    MATH::Vector2f(-1, 1)
                };
                std::vector<U3DVertexComponent> vertexFormat = {
                    U3DVertexComponent(SEM_POSITION, FLOATx2, 0, intptr(vertices)) };
                u3dVertexFormat_ = Unity3DCreator::CreateVertexFormat(vertexFormat);

                return true;
            }
            return false;
        }

        void Layout::addChild(Node* child)
        {
            Layout::addChild(child, child->getLocalZOrder(), child->getTag());
        }

        void Layout::addChild(Node * child, int localZOrder)
        {
            Layout::addChild(child, localZOrder, child->getTag());
        }

        void Layout::addChild(Node *child, int zOrder, int tag)
        {
            if (dynamic_cast<Widget*>(child)) {
                supplyTheLayoutParameterLackToChild(static_cast<Widget*>(child));
            }
            Widget::addChild(child, zOrder, tag);
            doLayoutDirty_ = true;
        }

        void Layout::addChild(Node* child, int zOrder, const std::string &name)
        {
            if (dynamic_cast<Widget*>(child)) {
                supplyTheLayoutParameterLackToChild(static_cast<Widget*>(child));
            }
            Widget::addChild(child, zOrder, name);
            doLayoutDirty_ = true;
        }

        void Layout::removeChild(Node *child, bool cleanup)
        {
            Widget::removeChild(child, cleanup);
            doLayoutDirty_ = true;
        }

        void Layout::removeAllChildren()
        {
            Widget::removeAllChildren();
            doLayoutDirty_ = true;
        }

        void Layout::removeAllChildrenWithCleanup(bool cleanup)
        {
            Widget::removeAllChildrenWithCleanup(cleanup);
            doLayoutDirty_ = true;
        }

        bool Layout::isClippingEnabled()const
        {
            return clippingEnabled_;
        }

        void Layout::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
        {
            if (!visible_)
            {
                return;
            }

            adaptRenderers();
            doLayout();

            if (clippingEnabled_)
            {
                switch (clippingType_)
                {
                    case ClippingType::STENCIL:
                        stencilClippingVisit(renderer, parentTransform, parentFlags);
                        break;
                    case ClippingType::SCISSOR:
                        scissorClippingVisit(renderer, parentTransform, parentFlags);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                Widget::visit(renderer, parentTransform, parentFlags);
            }
        }

        void Layout::stencilClippingVisit(Renderer *renderer, const MATH::Matrix4& parentTransform, uint32_t parentFlags)
        {
            if(!visible_)
                return;

            uint32_t flags = processParentFlags(parentTransform, parentFlags);

            // IMPORTANT:
            // To ease the migration to v3.0, we still support the MATH::Matrix4 stack,
            // but it is deprecated and your code should not rely on it
            Director::getInstance().pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            Director::getInstance().loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, modelViewTransform_);
            //Add group command

            groupCommand_.init(globalZOrder_);
            renderer->addCommand(&groupCommand_);

            renderer->pushGroup(groupCommand_.getRenderQueueID());

            uint64 i = 0;      // used by _children
            uint64 j = 0;      // used by _protectedChildren

            sortAllChildren();
            sortAllProtectedChildren();

            //
            // draw children and protectedChildren zOrder < 0
            //
            for( ; i < children_.size(); i++ )
            {
                auto node = children_.at(i);

                if ( node && node->getLocalZOrder() < 0 )
                    node->visit(renderer, modelViewTransform_, flags);
                else
                    break;
            }

            for( ; j < protectedChildren_.size(); j++ )
            {
                auto node = protectedChildren_.at(j);

                if ( node && node->getLocalZOrder() < 0 )
                    node->visit(renderer, modelViewTransform_, flags);
                else
                    break;
            }

            //
            // draw self
            //
            this->draw(renderer, modelViewTransform_, flags);

            //
            // draw children and protectedChildren zOrder >= 0
            //
            for(auto it=protectedChildren_.cbegin()+j; it != protectedChildren_.cend(); ++it)
                (*it)->visit(renderer, modelViewTransform_, flags);

            for(auto it=children_.cbegin()+i; it != children_.cend(); ++it)
                (*it)->visit(renderer, modelViewTransform_, flags);

            renderer->popGroup();

            Director::getInstance().popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }

        void Layout::drawFullScreenQuadClearStencil()
        {
            Director::getInstance().pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            Director::getInstance().loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

            Director::getInstance().pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
            Director::getInstance().loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

            auto u3dShader = ShaderCache::getInstance().getU3DShader(Unity3DShader::SHADER_NAME_POSITION_U_COLOR);

            int colorLocation = u3dShader->getUniformLocation("u_color");

            Color4F color(1, 1, 1, 1);

            u3dShader->apply();
            u3dShader->setUniformsForBuiltins();
            u3dShader->setUniformLocationWith4fv(colorLocation, (GLfloat*) &color.red, 1);

            u3dContext_->drawUp(PRIM_TRIANGLE_FAN, u3dVertexFormat_, 0, 4);

            Director::getInstance().popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
            Director::getInstance().popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }

        void Layout::onBeforeVisitScissor()
        {
            glEnable(GL_SCISSOR_TEST);
            // TODO
        }

        void Layout::onAfterVisitScissor()
        {
            glDisable(GL_SCISSOR_TEST);
        }

        void Layout::scissorClippingVisit(Renderer *renderer, const MATH::Matrix4& parentTransform, uint32_t parentFlags)
        {
            UNUSED(renderer);
            UNUSED(parentTransform);
            UNUSED(parentFlags);
        }

        void Layout::setClippingEnabled(bool able)
        {
            if (able == clippingEnabled_)
            {
                return;
            }
            clippingEnabled_ = able;
            switch (clippingType_)
            {
                case ClippingType::STENCIL:
                    if (able)
                    {
                        static bool once = true;
                        if (once)
                        {
                            glGetIntegerv(GL_STENCIL_BITS, &g_sStencilBits);
                            once = false;
                        }
                        clippingStencil_ = DrawNode::create();
                        if (running_)
                        {
                            clippingStencil_->onEnter();
                        }
                        clippingStencil_->retain();
                        setStencilClippingSize(contentSize_);
                    }
                    else
                    {
                        if (running_)
                        {
                            clippingStencil_->onExit();
                        }
                        clippingStencil_->release();
                        clippingStencil_ = nullptr;
                    }
                    break;
                default:
                    break;
            }
        }

        void Layout::setClippingType(ClippingType type)
        {
            if (type == clippingType_)
            {
                return;
            }
            bool clippingEnabled = isClippingEnabled();
            setClippingEnabled(false);
            clippingType_ = type;
            setClippingEnabled(clippingEnabled);
        }

        Layout::ClippingType Layout::getClippingType()const
        {
            return clippingType_;
        }

        void Layout::setStencilClippingSize(const MATH::Sizef &)
        {
            if (clippingEnabled_ && clippingType_ == ClippingType::STENCIL)
            {
                MATH::Vector2f rect[4];
                // rect[0].setZero(); Zero default
                rect[1].set(contentSize_.width, 0.0f);
                rect[2].set(contentSize_.width, contentSize_.height);
                rect[3].set(0.0f, contentSize_.height);
                Color4F green(0.0f, 1.0f, 0.0f, 1.0f);
                clippingStencil_->clear();
                clippingStencil_->drawPolygon(rect, 4, green, 0, green);
            }
        }

        const MATH::Rectf& Layout::getClippingRect()
        {
            if (clippingRectDirty_)
            {
                MATH::Vector2f worldPos = convertToWorldSpace(MATH::Vec2fZERO);
                MATH::AffineTransform t = getNodeToWorldAffineTransform();
                float scissorWidth = contentSize_.width*t.a;
                float scissorHeight = contentSize_.height*t.d;
                MATH::Rectf parentClippingRect;
                Layout* parent = this;

                while (parent)
                {
                    parent = dynamic_cast<Layout*>(parent->getParent());
                    if(parent)
                    {
                        if (parent->isClippingEnabled())
                        {
                            clippingParent_ = parent;
                            break;
                        }
                    }
                }

                if (clippingParent_)
                {
                    parentClippingRect = clippingParent_->getClippingRect();
                    float finalX = worldPos.x - (scissorWidth * anchorPoint_.x);
                    float finalY = worldPos.y - (scissorHeight * anchorPoint_.y);
                    float finalWidth = scissorWidth;
                    float finalHeight = scissorHeight;

                    float leftOffset = worldPos.x - parentClippingRect.origin.x;
                    if (leftOffset < 0.0f)
                    {
                        finalX = parentClippingRect.origin.x;
                        finalWidth += leftOffset;
                    }
                    float rightOffset = (worldPos.x + scissorWidth) - (parentClippingRect.origin.x + parentClippingRect.size.width);
                    if (rightOffset > 0.0f)
                    {
                        finalWidth -= rightOffset;
                    }
                    float topOffset = (worldPos.y + scissorHeight) - (parentClippingRect.origin.y + parentClippingRect.size.height);
                    if (topOffset > 0.0f)
                    {
                        finalHeight -= topOffset;
                    }
                    float bottomOffset = worldPos.y - parentClippingRect.origin.y;
                    if (bottomOffset < 0.0f)
                    {
                        finalY = parentClippingRect.origin.x;
                        finalHeight += bottomOffset;
                    }
                    if (finalWidth < 0.0f)
                    {
                        finalWidth = 0.0f;
                    }
                    if (finalHeight < 0.0f)
                    {
                        finalHeight = 0.0f;
                    }
                    clippingRect_.origin.x = finalX;
                    clippingRect_.origin.y = finalY;
                    clippingRect_.size.width = finalWidth;
                    clippingRect_.size.height = finalHeight;
                }
                else
                {
                    clippingRect_.origin.x = worldPos.x - (scissorWidth * anchorPoint_.x);
                    clippingRect_.origin.y = worldPos.y - (scissorHeight * anchorPoint_.y);
                    clippingRect_.size.width = scissorWidth;
                    clippingRect_.size.height = scissorHeight;
                }
                clippingRectDirty_ = false;
            }
            return clippingRect_;
        }

        void Layout::onSizeChanged()
        {
            Widget::onSizeChanged();
            setStencilClippingSize(contentSize_);
            doLayoutDirty_ = true;
            clippingRectDirty_ = true;
            if (backGroundImage_)
            {
                backGroundImage_->setPosition(contentSize_.width/2.0f, contentSize_.height/2.0f);
                if (backGroundScale9Enabled_ && backGroundImage_)
                {
                    backGroundImage_->setPreferredSize(contentSize_);
                }
            }
            if (colorRender_)
            {
                colorRender_->setContentSize(contentSize_);
            }
            if (gradientRender_)
            {
                gradientRender_->setContentSize(contentSize_);
            }
        }

        void Layout::setBackGroundImageScale9Enabled(bool able)
        {
            if (backGroundScale9Enabled_ == able)
            {
                return;
            }
            backGroundScale9Enabled_ = able;
            if (nullptr == backGroundImage_)
            {
                addBackGroundImage();
                setBackGroundImage(backGroundImageFileName_,bgImageTexType_);
            }
            backGroundImage_->setScale9Enabled(backGroundScale9Enabled_);
            setBackGroundImageCapInsets(backGroundImageCapInsets_);
        }

        bool Layout::isBackGroundImageScale9Enabled()const
        {
            return backGroundScale9Enabled_;
        }

        void Layout::setBackGroundImage(const std::string& fileName,TextureResType texType)
        {
            if (fileName.empty())
            {
                return;
            }
            if (backGroundImage_ == nullptr)
            {
                addBackGroundImage();
                backGroundImage_->setScale9Enabled(backGroundScale9Enabled_);
            }
            backGroundImageFileName_ = fileName;
            bgImageTexType_ = texType;

            switch (bgImageTexType_)
            {
                case TextureResType::LOCAL:
                    backGroundImage_->initWithFile(fileName);
                    break;
                case TextureResType::PLIST:
                    backGroundImage_->initWithSpriteFrameName(fileName);
                    break;
                default:
                    break;
            }
            if (backGroundScale9Enabled_) {
                backGroundImage_->setPreferredSize(contentSize_);
            }

            backGroundImageTextureSize_ = backGroundImage_->getContentSize();
            backGroundImage_->setPosition(contentSize_.width/2.0f, contentSize_.height/2.0f);
            updateBackGroundImageRGBA();
        }

        void Layout::setBackGroundImageCapInsets(const MATH::Rectf &capInsets)
        {
            backGroundImageCapInsets_ = capInsets;
            if (backGroundScale9Enabled_ && backGroundImage_)
            {
                backGroundImage_->setCapInsets(capInsets);
            }
        }

        const MATH::Rectf& Layout::getBackGroundImageCapInsets()const
        {
            return backGroundImageCapInsets_;
        }

        void Layout::supplyTheLayoutParameterLackToChild(Widget *child)
        {
            if (!child)
            {
                return;
            }
            switch (layoutType_)
            {
                case Type::TABSOLUTE:
                    break;
                case Type::THORIZONTAL:
                case Type::TVERTICAL:
                {
                    LinearLayoutParameter* layoutParameter = dynamic_cast<LinearLayoutParameter*>(child->getLayoutParameter());
                    if (!layoutParameter)
                    {
                        child->setLayoutParameter(LinearLayoutParameter::create());
                    }
                    break;
                }
                case Type::TRELATIVE:
                {
                    RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(child->getLayoutParameter());
                    if (!layoutParameter)
                    {
                        child->setLayoutParameter(RelativeLayoutParameter::create());
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void Layout::addBackGroundImage()
        {
            backGroundImage_ = Scale9Sprite::create();
            backGroundImage_->setScale9Enabled(false);

            addProtectedChild(backGroundImage_, BACKGROUNDIMAGE_Z, -1);

            backGroundImage_->setPosition(contentSize_.width/2.0f, contentSize_.height/2.0f);
        }

        void Layout::removeBackGroundImage()
        {
            if (!backGroundImage_)
            {
                return;
            }
            removeProtectedChild(backGroundImage_);
            backGroundImage_ = nullptr;
            backGroundImageFileName_ = "";
            backGroundImageTextureSize_ = MATH::SizefZERO;
        }

        void Layout::setBackGroundColorType(BackGroundColorType type)
        {
            if (colorType_ == type)
            {
                return;
            }
            switch (colorType_)
            {
                case BackGroundColorType::NONE:
                    if (colorRender_)
                    {
                        removeProtectedChild(colorRender_);
                        colorRender_ = nullptr;
                    }
                    if (gradientRender_)
                    {
                        removeProtectedChild(gradientRender_);
                        gradientRender_ = nullptr;
                    }
                    break;
                case BackGroundColorType::SOLID:
                    if (colorRender_)
                    {
                        removeProtectedChild(colorRender_);
                        colorRender_ = nullptr;
                    }
                    break;
                case BackGroundColorType::GRADIENT:
                    if (gradientRender_)
                    {
                        removeProtectedChild(gradientRender_);
                        gradientRender_ = nullptr;
                    }
                    break;
                default:
                    break;
            }
            colorType_ = type;
            switch (colorType_)
            {
                case BackGroundColorType::NONE:
                    break;
                case BackGroundColorType::SOLID:
                    colorRender_ = LayerColor::create();
                    colorRender_->setContentSize(contentSize_);
                    colorRender_->setOpacity(cOpacity_);
                    colorRender_->setColor(cColor_);
                    addProtectedChild(colorRender_, BCAKGROUNDCOLORRENDERER_Z, -1);
                    break;
                case BackGroundColorType::GRADIENT:
                    gradientRender_ = LayerGradient::create();
                    gradientRender_->setContentSize(contentSize_);
                    gradientRender_->setOpacity(cOpacity_);
                    gradientRender_->setStartColor(gStartColor_);
                    gradientRender_->setEndColor(gEndColor_);
                    gradientRender_->setVector(alongVector_);
                    addProtectedChild(gradientRender_, BCAKGROUNDCOLORRENDERER_Z, -1);
                    break;
                default:
                    break;
            }
        }

        Layout::BackGroundColorType Layout::getBackGroundColorType()const
        {
            return colorType_;
        }

        void Layout::setBackGroundColor(const Color3B &color)
        {
            cColor_ = color;
            if (colorRender_)
            {
                colorRender_->setColor(color);
            }
        }

        const Color3B& Layout::getBackGroundColor()const
        {
            return cColor_;
        }

        void Layout::setBackGroundColor(const Color3B &startColor, const Color3B &endColor)
        {
            gStartColor_ = startColor;
            if (gradientRender_)
            {
                gradientRender_->setStartColor(startColor);
            }
            gEndColor_ = endColor;
            if (gradientRender_)
            {
                gradientRender_->setEndColor(endColor);
            }
        }

        const Color3B& Layout::getBackGroundStartColor()const
        {
            return gStartColor_;
        }

        const Color3B& Layout::getBackGroundEndColor()const
        {
            return gEndColor_;
        }

        void Layout::setBackGroundColorOpacity(uint8 opacity)
        {
            cOpacity_ = opacity;
            switch (colorType_)
            {
                case BackGroundColorType::NONE:
                    break;
                case BackGroundColorType::SOLID:
                    colorRender_->setOpacity(opacity);
                    break;
                case BackGroundColorType::GRADIENT:
                    gradientRender_->setOpacity(opacity);
                    break;
                default:
                    break;
            }
        }

        uint8 Layout::getBackGroundColorOpacity()const
        {
            return cOpacity_;
        }

        void Layout::setBackGroundColorVector(const MATH::Vector2f &vector)
        {
            alongVector_ = vector;
            if (gradientRender_)
            {
                gradientRender_->setVector(vector);
            }
        }

        const MATH::Vector2f& Layout::getBackGroundColorVector()const
        {
            return alongVector_;
        }

        void Layout::setBackGroundImageColor(const Color3B &color)
        {
            backGroundImageColor_ = color;
            updateBackGroundImageColor();
        }

        void Layout::setBackGroundImageOpacity(uint8 opacity)
        {
            backGroundImageOpacity_ = opacity;
            updateBackGroundImageOpacity();
        }

        const Color3B& Layout::getBackGroundImageColor()const
        {
            return backGroundImageColor_;
        }

        uint8 Layout::getBackGroundImageOpacity()const
        {
            return backGroundImageOpacity_;
        }

        void Layout::updateBackGroundImageColor()
        {
            if (backGroundImage_)
            {
                backGroundImage_->setColor(backGroundImageColor_);
            }
        }

        void Layout::updateBackGroundImageOpacity()
        {
            if (backGroundImage_)
            {
                backGroundImage_->setOpacity(backGroundImageOpacity_);
            }
        }

        void Layout::updateBackGroundImageRGBA()
        {
            if (backGroundImage_)
            {
                backGroundImage_->setColor(backGroundImageColor_);
                backGroundImage_->setOpacity(backGroundImageOpacity_);
            }
        }

        const MATH::Sizef& Layout::getBackGroundImageTextureSize() const
        {
            return backGroundImageTextureSize_;
        }

        void Layout::setLayoutType(Type type)
        {
            layoutType_ = type;

            for (auto& child : children_)
            {
                Widget* widgetChild = dynamic_cast<Widget*>(child);
                if (widgetChild)
                {
                    supplyTheLayoutParameterLackToChild(static_cast<Widget*>(child));
                }
            }
            doLayoutDirty_ = true;
        }



        Layout::Type Layout::getLayoutType() const
        {
            return layoutType_;
        }

        void Layout::forceDoLayout()
        {
            this->requestDoLayout();
            this->doLayout();
        }

        void Layout::requestDoLayout()
        {
            doLayoutDirty_ = true;
        }

        MATH::Sizef Layout::getLayoutContentSize()const
        {
            return this->getContentSize();
        }

        const HObjectVector<Node*>& Layout::getLayoutElements()const
        {
            return this->getChildren();
        }

        LayoutManager* Layout::createLayoutManager()
        {
            LayoutManager* exe = nullptr;
            switch (layoutType_)
            {
                case Type::TVERTICAL:
                    exe = LinearVerticalLayoutManager::create();
                    break;
                case Type::THORIZONTAL:
                    exe = LinearHorizontalLayoutManager::create();
                    break;
                case Type::TRELATIVE:
                    exe = RelativeLayoutManager::create();
                    break;
                default:
                    break;
            }
            return exe;

        }

        void Layout::doLayout()
        {

            if (!doLayoutDirty_)
            {
                return;
            }

            sortAllChildren();

            LayoutManager* executant = this->createLayoutManager();

            if (executant)
            {
                executant->doLayout(this);
            }

            doLayoutDirty_ = false;
        }

        Widget* Layout::createCloneInstance()
        {
            return Layout::create();
        }

        void Layout::copyClonedWidgetChildren(Widget* model)
        {
            Widget::copyClonedWidgetChildren(model);
        }

        void Layout::copySpecialProperties(Widget *widget)
        {
            Layout* layout = dynamic_cast<Layout*>(widget);
            if (layout)
            {
                setBackGroundImageScale9Enabled(layout->backGroundScale9Enabled_);
                setBackGroundImage(layout->backGroundImageFileName_,layout->bgImageTexType_);
                setBackGroundImageCapInsets(layout->backGroundImageCapInsets_);
                setBackGroundColorType(layout->colorType_);
                setBackGroundColor(layout->cColor_);
                setBackGroundColor(layout->gStartColor_, layout->gEndColor_);
                setBackGroundColorOpacity(layout->cOpacity_);
                setBackGroundColorVector(layout->alongVector_);
                setLayoutType(layout->layoutType_);
                setClippingEnabled(layout->clippingEnabled_);
                setClippingType(layout->clippingType_);
                loopFocus_ = layout->loopFocus_;
                passFocusToChild_ = layout->passFocusToChild_;
                isInterceptTouch_ = layout->isInterceptTouch_;
            }
        }

        void Layout::setLoopFocus(bool loop)
        {
            loopFocus_ = loop;
        }

        bool Layout::isLoopFocus()const
        {
            return loopFocus_;
        }


        void Layout::setPassFocusToChild(bool pass)
        {
            passFocusToChild_ = pass;
        }

        bool Layout::isPassFocusToChild()const
        {
            return passFocusToChild_;
        }

        MATH::Sizef Layout::getLayoutAccumulatedSize()const
        {
            const auto& children = this->getChildren();
            MATH::Sizef layoutSize = MATH::SizefZERO;
            int widgetCount =0;
            for(const auto& widget : children)
            {
                Layout *layout = dynamic_cast<Layout*>(widget);
                if (nullptr != layout)
                {
                    layoutSize = layoutSize + layout->getLayoutAccumulatedSize();
                }
                else
                {
                    Widget *w = dynamic_cast<Widget*>(widget);
                    if (w)
                    {
                        widgetCount++;
                        Margin m = w->getLayoutParameter()->getMargin();
                        layoutSize = layoutSize + w->getContentSize() + MATH::Sizef(m.right + m.left,  m.top + m.bottom) * 0.5;
                    }
                }
            }

            //substract extra size
            Type type = this->getLayoutType();
            if (type == Type::THORIZONTAL)
            {
                layoutSize = layoutSize - MATH::Sizef(0, layoutSize.height/widgetCount * (widgetCount-1));
            }
            if (type == Type::TVERTICAL)
            {
                layoutSize = layoutSize - MATH::Sizef(layoutSize.width/widgetCount * (widgetCount-1), 0);
            }
            return layoutSize;
        }

        MATH::Vector2f Layout::getWorldCenterPoint(Widget* widget)const
        {
            Layout *layout = dynamic_cast<Layout*>(widget);
            //FIXEDME: we don't need to calculate the content size of layout anymore
            MATH::Sizef widgetSize = layout ? layout->getLayoutAccumulatedSize() :  widget->getContentSize();
            return widget->convertToWorldSpace(MATH::Vector2f(widgetSize.width/2, widgetSize.height/2));
        }

        float Layout::calculateNearestDistance(Widget* baseWidget)
        {
            float distance = MATH::MATH_FLOAT_MAX();

            MATH::Vector2f widgetPosition =  this->getWorldCenterPoint(baseWidget);

            for (Node* node : children_)
            {
                Layout *layout = dynamic_cast<Layout*>(node);
                int length;
                if (layout)
                {
                    length = layout->calculateNearestDistance(baseWidget);
                }
                else
                {
                    Widget* w = dynamic_cast<Widget*>(node);
                    if (w && w->isFocusEnabled())
                    {
                        MATH::Vector2f wPosition = this->getWorldCenterPoint(w);
                        length = (wPosition - widgetPosition).length();
                    }
                    else
                    {
                        continue;
                    }
                }

                if (length < distance)
                {
                    distance = length;
                }


            }
            return distance;
        }

        float Layout::calculateFarthestDistance(Widget *baseWidget)
        {
            float distance = -MATH::MATH_FLOAT_MAX();

            MATH::Vector2f widgetPosition =  this->getWorldCenterPoint(baseWidget);

            for (Node* node : children_)
            {
                Layout *layout = dynamic_cast<Layout*>(node);
                int length;
                if (layout)
                {
                    length = layout->calculateFarthestDistance(baseWidget);
                }
                else
                {
                    Widget* w = dynamic_cast<Widget*>(node);
                    if (w && w->isFocusEnabled()) {
                        MATH::Vector2f wPosition = this->getWorldCenterPoint(w);
                        length = (wPosition - widgetPosition).length();
                    }
                    else
                    {
                        continue;
                    }
                }

                if (length > distance)
                {
                    distance = length;
                }
            }
            return distance;
        }

        int Layout::findFirstFocusEnabledWidgetIndex()
        {
            uint64 index = 0;
            uint64 count = this->getChildren().size();
            while (index < count)
            {
                Widget* w =  dynamic_cast<Widget*>(children_.at(index));
                if (w && w->isFocusEnabled())
                {
                    return (int)index;
                }
                index++;
            }

            return 0;
        }

        int Layout::findNearestChildWidgetIndex(FocusDirection direction, Widget* baseWidget)
        {
            if (baseWidget == nullptr || baseWidget == this)
            {
                return this->findFirstFocusEnabledWidgetIndex();
            }
            uint64 index = 0;
            uint64 count = this->getChildren().size();

            float distance = MATH::MATH_FLOAT_MAX();
            int found = 0;
            if (direction == FocusDirection::LEFT || direction == FocusDirection::RIGHT ||
                direction == FocusDirection::DOWN || direction == FocusDirection::UP)
            {
                MATH::Vector2f widgetPosition =  this->getWorldCenterPoint(baseWidget);
                while (index <  count)
                {
                    Widget *w = dynamic_cast<Widget*>(this->getChildren().at(index));
                    if (w && w->isFocusEnabled())
                    {
                        MATH::Vector2f wPosition = this->getWorldCenterPoint(w);
                        float length;
                        Layout *layout = dynamic_cast<Layout*>(w);
                        if (layout)
                        {
                            length = layout->calculateNearestDistance(baseWidget);
                        }
                        else
                        {
                            length = (wPosition - widgetPosition).length();
                        }

                        if (length < distance)
                        {
                                found = index;
                                distance = length;
                        }
                    }
                    index++;
                }
                return  found;
            }

            return 0;
        }

        int Layout::findFarthestChildWidgetIndex(FocusDirection direction, Widget *baseWidget)
        {
            if (baseWidget == nullptr || baseWidget == this)
            {
                return this->findFirstFocusEnabledWidgetIndex();
            }
            uint64 index = 0;
            uint64 count = this->getChildren().size();

            float distance = -MATH::MATH_FLOAT_MAX();
            int found = 0;
            if (direction == FocusDirection::LEFT || direction == FocusDirection::RIGHT
                || direction == FocusDirection::DOWN || direction == FocusDirection::UP)
            {
                MATH::Vector2f widgetPosition =  this->getWorldCenterPoint(baseWidget);
                while (index <  count)
                {
                    Widget *w = dynamic_cast<Widget*>(this->getChildren().at(index));
                    if (w && w->isFocusEnabled())
                    {
                        MATH::Vector2f wPosition = this->getWorldCenterPoint(w);
                        float length;
                        Layout *layout = dynamic_cast<Layout*>(w);
                        if (layout)
                        {
                            length = layout->calculateFarthestDistance(baseWidget);
                        }
                        else
                        {
                            length = (wPosition - widgetPosition).length();
                        }

                        if (length > distance)
                        {
                            found = index;
                            distance = length;
                        }
                    }
                    index++;
                }
                return  found;
            }

            return 0;
        }



        Widget* Layout::findFocusEnabledChildWidgetByIndex(uint64 index)
        {

            Widget *widget = this->getChildWidgetByIndex(index);

            if (widget)
            {
                if (widget->isFocusEnabled())
                {
                    return widget;
                }
                index = index + 1;
                return this->findFocusEnabledChildWidgetByIndex(index);
            }
            return nullptr;
        }

        Widget *Layout::findFirstNonLayoutWidget()
        {
            Widget* widget = nullptr;
            for(Node *node : children_)
            {
                Layout* layout = dynamic_cast<Layout*>(node);
                if (layout)
                {
                    widget = layout->findFirstNonLayoutWidget();
                    if (widget != nullptr)
                    {
                        return widget;
                    }
                }
                else
                {
                    Widget *w = dynamic_cast<Widget*>(node);
                    if (w)
                    {
                        widget = w;
                        break;
                    }
                }

            }

            return widget;
        }

        void Layout::findProperSearchingFunctor(FocusDirection dir, Widget* baseWidget)
        {
            if (baseWidget == nullptr)
            {
                return;
            }

            MATH::Vector2f previousWidgetPosition = this->getWorldCenterPoint(baseWidget);

            MATH::Vector2f widgetPosition = this->getWorldCenterPoint(this->findFirstNonLayoutWidget());

            if (dir == FocusDirection::LEFT)
            {
                if (previousWidgetPosition.x > widgetPosition.x)
                {
                    onPassFocusToChild = std::bind(&Layout::findNearestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
                else
                {
                    onPassFocusToChild = std::bind(&Layout::findFarthestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
            }
            else if(dir == FocusDirection::RIGHT)
            {
                if (previousWidgetPosition.x > widgetPosition.x)
                {
                    onPassFocusToChild = std::bind(&Layout::findFarthestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
                else
                {
                    onPassFocusToChild = std::bind(&Layout::findNearestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
            }
            else if(dir == FocusDirection::DOWN)
            {
                if (previousWidgetPosition.y > widgetPosition.y)
                {
                    onPassFocusToChild = std::bind(&Layout::findNearestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
                else
                {
                    onPassFocusToChild = std::bind(&Layout::findFarthestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
            }
            else if(dir == FocusDirection::UP)
            {
                if (previousWidgetPosition.y < widgetPosition.y)
                {
                    onPassFocusToChild = std::bind(&Layout::findNearestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
                else
                {
                    onPassFocusToChild = std::bind(&Layout::findFarthestChildWidgetIndex, this, std::placeholders::_1, std::placeholders::_2);
                }
            }
        }


        Widget* Layout::passFocusToChild(FocusDirection dir, Widget *current)
        {
            if (checkFocusEnabledChild())
            {
                Widget* previousWidget = this->getCurrentFocusedWidget();

                this->findProperSearchingFunctor(dir, previousWidget);

                int index = onPassFocusToChild(dir, previousWidget);

                Widget *widget = this->getChildWidgetByIndex(index);
                Layout *layout = dynamic_cast<Layout*>(widget);
                if (layout)
                {
                    layout->isFocusPassing_ = true;
                    return layout->findNextFocusedWidget(dir, layout);
                }
                else
                {
                    this->dispatchFocusEvent(current, widget);
                    return widget;
                }
            }
            else
            {
                return this;
            }

        }

        bool Layout::checkFocusEnabledChild()const
        {
            bool ret = false;
            for(Node* node : children_)
            {
                Widget* widget = dynamic_cast<Widget*>(node);
                if (widget && widget->isFocusEnabled())
                {
                    ret = true;
                    break;
                }
            }
            return ret;
        }

        Widget* Layout::getChildWidgetByIndex(uint64 index)const
        {
            uint64 size = children_.size();
            int count = 0;
            uint64 oldIndex = index;
            Widget *widget = nullptr;
            while (index < size)
            {
                Widget* firstChild = dynamic_cast<Widget*>(children_.at(index));
                if (firstChild)
                {
                    widget = firstChild;
                    break;
                }
                count++;
                index++;
            }

            if (nullptr == widget)
            {
                uint64 begin = 0;
                while (begin < oldIndex)
                {
                    Widget* firstChild = dynamic_cast<Widget*>(children_.at(begin));
                    if (firstChild)
                    {
                        widget = firstChild;
                        break;
                    }
                    count++;
                    begin++;
                }
            }


            return widget;
        }

        Widget* Layout::getPreviousFocusedWidget(FocusDirection direction, Widget *current)
        {
            Widget *nextWidget = nullptr;
            int64 previousWidgetPos = children_.getIndex(current);
            previousWidgetPos = previousWidgetPos - 1;
            if (previousWidgetPos >= 0)
            {
                nextWidget = this->getChildWidgetByIndex(previousWidgetPos);
                if (nextWidget->isFocusEnabled())
                {
                    Layout* layout = dynamic_cast<Layout*>(nextWidget);
                    if (layout)
                    {
                        layout->isFocusPassing_ = true;
                        return layout->findNextFocusedWidget(direction, layout);
                    }
                    this->dispatchFocusEvent(current, nextWidget);
                    return nextWidget;
                }
                else
                {
                    //handling the disabled widget, there is no actual focus lose or get, so we don't need any envet
                    return this->getPreviousFocusedWidget(direction, nextWidget);
                }
            }
            else
            {
                if (loopFocus_)
                {
                    if (checkFocusEnabledChild())
                    {
                        previousWidgetPos = children_.size()-1;
                        nextWidget = this->getChildWidgetByIndex(previousWidgetPos);
                        if (nextWidget->isFocusEnabled())
                        {
                            Layout* layout = dynamic_cast<Layout*>(nextWidget);
                            if (layout)
                            {
                                layout->isFocusPassing_ = true;
                                return layout->findNextFocusedWidget(direction, layout);
                            }
                            else
                            {
                                this->dispatchFocusEvent(current, nextWidget);
                                return nextWidget;
                            }
                        }
                        else
                        {
                            return this->getPreviousFocusedWidget(direction, nextWidget);
                        }
                    }
                    else
                    {
                        if (dynamic_cast<Layout*>(current))
                        {
                            return current;
                        }
                        else
                        {
                            return focusedWidget_;
                        }
                    }
                }
                else
                {
                    if (isLastWidgetInContainer(current, direction))
                    {
                        if (isWidgetAncestorSupportLoopFocus(this, direction))
                        {
                            return Widget::findNextFocusedWidget(direction, this);
                        }
                        if (dynamic_cast<Layout*>(current))
                        {
                            return current;
                        }
                        else
                        {
                            return focusedWidget_;
                        }
                    }
                    else
                    {
                        return Widget::findNextFocusedWidget(direction, this);
                    }
                }
            }
        }

        Widget* Layout::getNextFocusedWidget(FocusDirection direction, Widget *current)
        {
            Widget *nextWidget = nullptr;
            uint64 previousWidgetPos = children_.getIndex(current);
            previousWidgetPos = previousWidgetPos + 1;
            if (previousWidgetPos < children_.size())
            {
                nextWidget = this->getChildWidgetByIndex(previousWidgetPos);
                //handle widget
                if (nextWidget)
                {
                    if (nextWidget->isFocusEnabled())
                    {
                        Layout* layout = dynamic_cast<Layout*>(nextWidget);
                        if (layout)
                        {
                            layout->isFocusPassing_ = true;
                            return layout->findNextFocusedWidget(direction, layout);
                        }
                        else
                        {
                            this->dispatchFocusEvent(current, nextWidget);
                            return nextWidget;
                        }
                    }
                    else
                    {
                        return this->getNextFocusedWidget(direction, nextWidget);
                    }
                }
                else
                {
                    return current;
                }
            }
            else
            {
                if (loopFocus_)
                {
                    if (checkFocusEnabledChild())
                    {
                        previousWidgetPos = 0;
                        nextWidget = this->getChildWidgetByIndex(previousWidgetPos);
                        if (nextWidget->isFocusEnabled())
                        {
                            Layout* layout = dynamic_cast<Layout*>(nextWidget);
                            if (layout)
                            {
                                layout->isFocusPassing_ = true;
                                return layout->findNextFocusedWidget(direction, layout);
                            }
                            else
                            {
                                this->dispatchFocusEvent(current, nextWidget);
                                return nextWidget;
                            }
                        }
                        else
                        {
                            return this->getNextFocusedWidget(direction, nextWidget);
                        }
                    }
                    else
                    {
                        if (dynamic_cast<Layout*>(current)) {
                            return current;
                        }
                        else
                        {
                            return focusedWidget_;
                        }
                    }
                }
                else
                {
                    if (isLastWidgetInContainer(current, direction))
                    {
                        if (isWidgetAncestorSupportLoopFocus(this, direction))
                        {
                            return Widget::findNextFocusedWidget(direction, this);
                        }
                        if (dynamic_cast<Layout*>(current)) {
                            return current;
                        }
                        else
                        {
                            return focusedWidget_;
                        }
                    }
                    else
                    {
                        return Widget::findNextFocusedWidget(direction, this);
                    }
                }
            }
        }

        bool  Layout::isLastWidgetInContainer(Widget* widget, FocusDirection direction)const
        {
            Layout* parent = dynamic_cast<Layout*>(widget->getParent());
            if (parent == nullptr)
            {
                return true;
            }

            auto& container = parent->getChildren();
            uint64 index = container.getIndex(widget);
            if (parent->getLayoutType() == Type::THORIZONTAL)
            {
                if (direction == FocusDirection::LEFT)
                {
                    if (index == 0)
                    {
                        return isLastWidgetInContainer(parent, direction);
                    }
                    else
                    {
                        return false;
                    }
                }
                if (direction == FocusDirection::RIGHT)
                {
                    if (index == container.size()-1)
                    {
                        return isLastWidgetInContainer(parent, direction);
                    }
                    else
                    {
                        return false;
                    }
                }
                if (direction == FocusDirection::DOWN)
                {
                    return isLastWidgetInContainer(parent, direction);
                }

                if (direction == FocusDirection::UP)
                {
                    return isLastWidgetInContainer(parent, direction);
                }
            }
            else if(parent->getLayoutType() == Type::TVERTICAL)
            {
                if (direction == FocusDirection::UP)
                {
                    if (index == 0)
                    {
                        return isLastWidgetInContainer(parent, direction);

                    }
                    else
                    {
                        return false;
                    }
                }
                if (direction == FocusDirection::DOWN)
                {
                    if (index == container.size() - 1)
                    {
                        return isLastWidgetInContainer(parent, direction);
                    }
                    else
                    {
                        return false;
                    }
                }
                if (direction == FocusDirection::LEFT)
                {
                    return isLastWidgetInContainer(parent, direction);
                }

                if (direction == FocusDirection::RIGHT)
                {
                    return isLastWidgetInContainer(parent, direction);
                }
            }

            return false;
        }

        bool  Layout::isWidgetAncestorSupportLoopFocus(Widget* widget, FocusDirection direction)const
        {
            Layout* parent = dynamic_cast<Layout*>(widget->getParent());
            if (parent == nullptr)
            {
                return false;
            }
            if (parent->isLoopFocus())
            {
                auto layoutType = parent->getLayoutType();
                if (layoutType == Type::THORIZONTAL)
                {
                    if (direction == FocusDirection::LEFT || direction == FocusDirection::RIGHT)
                    {
                        return true;
                    }
                    else
                    {
                        return isWidgetAncestorSupportLoopFocus(parent, direction);
                    }
                }
                if (layoutType == Type::TVERTICAL)
                {
                    if (direction == FocusDirection::DOWN || direction == FocusDirection::UP)
                    {
                        return true;
                    }
                    else
                    {
                        return isWidgetAncestorSupportLoopFocus(parent, direction);
                    }
                }
            }
            return isWidgetAncestorSupportLoopFocus(parent, direction);
        }

        Widget* Layout::findNextFocusedWidget(FocusDirection direction, Widget* current)
        {
            if (isFocusPassing_ || this->isFocused())
            {
                Layout* parent = dynamic_cast<Layout*>(this->getParent());
                isFocusPassing_ = false;

                if (passFocusToChild_)
                {
                    Widget * w = this->passFocusToChild(direction, current);
                    if (dynamic_cast<Layout*>(w))
                    {
                        if (parent)
                        {
                            parent->isFocusPassing_ = true;
                            return parent->findNextFocusedWidget(direction, this);
                        }
                    }
                    return w;
                }

                if (nullptr == parent)
                {
                    return this;
                }
                parent->isFocusPassing_ = true;
                return parent->findNextFocusedWidget(direction, this);

            }
            else if(current->isFocused() || dynamic_cast<Layout*>(current))
            {
                if (layoutType_ == Type::THORIZONTAL)
                {
                    switch (direction)
                    {
                        case FocusDirection::LEFT:
                        {
                            return this->getPreviousFocusedWidget(direction, current);
                        }break;
                        case FocusDirection::RIGHT:
                        {
                            return this->getNextFocusedWidget(direction, current);
                        }break;
                        case FocusDirection::DOWN:
                        case FocusDirection::UP:
                        {
                            if (isLastWidgetInContainer(this, direction))
                            {
                                if (isWidgetAncestorSupportLoopFocus(current, direction))
                                {
                                    return Widget::findNextFocusedWidget(direction, this);
                                }
                                return current;
                            }
                            else
                            {
                                return Widget::findNextFocusedWidget(direction, this);
                            }
                        }break;
                        default:
                        {
                            return current;
                        }
                            break;
                    }
                }
                else if (layoutType_ == Type::TVERTICAL)
                {
                    switch (direction)
                    {
                        case FocusDirection::LEFT:
                        case FocusDirection::RIGHT:
                        {
                            if (isLastWidgetInContainer(this, direction))
                            {
                                if (isWidgetAncestorSupportLoopFocus(current, direction))
                                {
                                    return Widget::findNextFocusedWidget(direction, this);
                                }
                                return current;
                            }
                            else
                            {
                                return Widget::findNextFocusedWidget(direction, this);
                            }
                        } break;
                        case FocusDirection::DOWN:
                        {
                            return getNextFocusedWidget(direction, current);
                        }
                            break;
                        case FocusDirection::UP:
                        {
                            return getPreviousFocusedWidget(direction, current);
                        }
                            break;
                        default:
                        {
                            return current;
                        }
                            break;
                    }
                }
                else
                {
                    return current;
                }
            }
            else
            {
                return current;
            }
        }

        void Layout::setCameraMask(unsigned short mask, bool applyChildren)
        {
            Widget::setCameraMask(mask, applyChildren);
            if (clippingStencil_){
                clippingStencil_->setCameraMask(mask, applyChildren);
            }
        }

        HBox::HBox()
        {
        }

        HBox::~HBox()
        {
        }

        HBox* HBox::create()
        {
            HBox* widget = new (std::nothrow) HBox();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        HBox* HBox::create(const MATH::Sizef &size)
        {
            HBox* widget = new (std::nothrow) HBox();
            if (widget && widget->initWithSize(size))
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        bool HBox::init()
        {
            if (Layout::init())
            {
                setLayoutType(Layout::Type::THORIZONTAL);
                return true;
            }
            return false;
        }

        bool HBox::initWithSize(const MATH::Sizef& size)
        {
            if (init())
            {
                setContentSize(size);
                return true;
            }
            return false;
        }

        VBox::VBox()
        {
        }

        VBox::~VBox()
        {
        }

        VBox* VBox::create()
        {
            VBox* widget = new (std::nothrow) VBox();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        VBox* VBox::create(const MATH::Sizef &size)
        {
            VBox* widget = new (std::nothrow) VBox();
            if (widget && widget->initWithSize(size))
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        bool VBox::init()
        {
            if (Layout::init())
            {
                setLayoutType(Layout::Type::TVERTICAL);
                return true;
            }
            return false;
        }

        bool VBox::initWithSize(const MATH::Sizef& size)
        {
            if (init())
            {
                setContentSize(size);
                return true;
            }
            return false;
        }

        RelativeBox::RelativeBox()
        {
        }

        RelativeBox::~RelativeBox()
        {
        }

        RelativeBox* RelativeBox::create()
        {
            RelativeBox* widget = new (std::nothrow) RelativeBox();
            if (widget && widget->init())
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        RelativeBox* RelativeBox::create(const MATH::Sizef &size)
        {
            RelativeBox* widget = new (std::nothrow) RelativeBox();
            if (widget && widget->initWithSize(size))
            {
                widget->autorelease();
                return widget;
            }
            SAFE_DELETE(widget);
            return nullptr;
        }

        bool RelativeBox::init()
        {
            if (Layout::init())
            {
                setLayoutType(Layout::Type::TRELATIVE);
                return true;
            }
            return false;
        }

        bool RelativeBox::initWithSize(const MATH::Sizef& size)
        {
            if (init())
            {
                setContentSize(size);
                return true;
            }
            return false;
        }

        LinearHorizontalLayoutManager* LinearHorizontalLayoutManager::create()
        {
            LinearHorizontalLayoutManager* exe = new (std::nothrow) LinearHorizontalLayoutManager();
            if (exe)
            {
                exe->autorelease();
                return exe;
            }
            SAFE_DELETE(exe);
            return nullptr;
        }


        void LinearHorizontalLayoutManager::doLayout(LayoutProtocol* layout)
        {
            MATH::Sizef layoutSize = layout->getLayoutContentSize();
            HObjectVector<Node*> container = layout->getLayoutElements();
            float leftBoundary = 0.0f;
            for (auto& subWidget : container)
            {
                Widget* child = dynamic_cast<Widget*>(subWidget);
                if (child)
                {
                    LinearLayoutParameter* layoutParameter = dynamic_cast<LinearLayoutParameter*>(child->getLayoutParameter());
                    if (layoutParameter)
                    {
                        LinearLayoutParameter::LinearGravity childGravity = layoutParameter->getGravity();
                        MATH::Vector2f ap = child->getAnchorPoint();
                        MATH::Sizef cs = child->getContentSize();
                        float finalPosX = leftBoundary + (ap.x * cs.width);
                        float finalPosY = layoutSize.height - (1.0f - ap.y) * cs.height;
                        switch (childGravity)
                        {
                            case LinearLayoutParameter::LinearGravity::NONE:
                            case LinearLayoutParameter::LinearGravity::TOP:
                                break;
                            case LinearLayoutParameter::LinearGravity::BOTTOM:
                                finalPosY = ap.y * cs.height;
                                break;
                            case LinearLayoutParameter::LinearGravity::CENTER_VERTICAL:
                                finalPosY = layoutSize.height / 2.0f - cs.height * (0.5f - ap.y);
                                break;
                            default:
                                break;
                        }
                        Margin mg = layoutParameter->getMargin();
                        finalPosX += mg.left;
                        finalPosY -= mg.top;
                        child->setPosition(MATH::Vector2f(finalPosX, finalPosY));
                        leftBoundary = child->getRightBoundary() + mg.right;
                    }
                }
            }
        }


        //LinearVerticalLayoutManager
        LinearVerticalLayoutManager* LinearVerticalLayoutManager::create()
        {
            LinearVerticalLayoutManager* exe = new (std::nothrow) LinearVerticalLayoutManager();
            if (exe)
            {
                exe->autorelease();
                return exe;
            }
            SAFE_DELETE(exe);
            return nullptr;
        }

        void LinearVerticalLayoutManager::doLayout(LayoutProtocol* layout)
        {
            MATH::Sizef layoutSize = layout->getLayoutContentSize();
            HObjectVector<Node*> container = layout->getLayoutElements();
            float topBoundary = layoutSize.height;

            for (auto& subWidget : container)
            {
                LayoutParameterProtocol* child = dynamic_cast<LayoutParameterProtocol*>(subWidget);
                if (child)
                {
                    LinearLayoutParameter* layoutParameter = dynamic_cast<LinearLayoutParameter*>(child->getLayoutParameter());

                    if (layoutParameter)
                    {
                        LinearLayoutParameter::LinearGravity childGravity = layoutParameter->getGravity();
                        MATH::Vector2f ap = subWidget->getAnchorPoint();
                        MATH::Sizef cs = subWidget->getContentSize();
                        float finalPosX = ap.x * cs.width;
                        float finalPosY = topBoundary - ((1.0f-ap.y) * cs.height);
                        switch (childGravity)
                        {
                            case LinearLayoutParameter::LinearGravity::NONE:
                            case LinearLayoutParameter::LinearGravity::LEFT:
                                break;
                            case LinearLayoutParameter::LinearGravity::RIGHT:
                                finalPosX = layoutSize.width - ((1.0f - ap.x) * cs.width);
                                break;
                            case LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL:
                                finalPosX = layoutSize.width / 2.0f - cs.width * (0.5f-ap.x);
                                break;
                            default:
                                break;
                        }
                        Margin mg = layoutParameter->getMargin();
                        finalPosX += mg.left;
                        finalPosY -= mg.top;
                        subWidget->setPosition(finalPosX, finalPosY);
                        topBoundary = subWidget->getPosition().y - subWidget->getAnchorPoint().y * subWidget->getContentSize().height - mg.bottom;
                    }
                }
            }
        }

        //RelativeLayoutManager

        RelativeLayoutManager* RelativeLayoutManager::create()
        {
            RelativeLayoutManager* exe = new (std::nothrow) RelativeLayoutManager();
            if (exe)
            {
                exe->autorelease();
                return exe;
            }
            SAFE_DELETE(exe);
            return nullptr;
        }




        HObjectVector<Widget*> RelativeLayoutManager::getAllWidgets(LayoutProtocol *layout)
        {
            HObjectVector<Node*> container = layout->getLayoutElements();
            HObjectVector<Widget*> widgetChildren;
            for (auto& subWidget : container)
            {
                Widget* child = dynamic_cast<Widget*>(subWidget);
                if (child)
                {
                    RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(child->getLayoutParameter());
                    layoutParameter->_put = false;
                    unlayoutChildCount_++;
                    widgetChildren.pushBack(child);
                }
            }
            return widgetChildren;

        }

        Widget* RelativeLayoutManager::getRelativeWidget(Widget* widget)
        {
            Widget* relativeWidget = nullptr;
            RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(widget->getLayoutParameter());
            const std::string relativeName = layoutParameter->getRelativeToWidgetName();

            if (!relativeName.empty())
            {
                for (auto& sWidget : widgetChildren_)
                {
                    if (sWidget)
                    {
                        RelativeLayoutParameter* rlayoutParameter = dynamic_cast<RelativeLayoutParameter*>(sWidget->getLayoutParameter());
                        if (rlayoutParameter &&  rlayoutParameter->getRelativeName() == relativeName)
                        {
                            relativeWidget = sWidget;
                            relativeWidgetLP_ = rlayoutParameter;
                            break;
                        }
                    }
                }
            }
            return relativeWidget;
        }

        bool RelativeLayoutManager::caculateFinalPositionWithRelativeWidget(LayoutProtocol *layout)
        {
            MATH::Vector2f ap = widget_->getAnchorPoint();
            MATH::Sizef cs = widget_->getContentSize();

            finalPositionX_ = 0.0f;
            finalPositionY_ = 0.0f;

            Widget* relativeWidget = this->getRelativeWidget(widget_);

            RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(widget_->getLayoutParameter());

            RelativeLayoutParameter::RelativeAlign align = layoutParameter->getAlign();

            MATH::Sizef layoutSize = layout->getLayoutContentSize();


            switch (align)
            {
                case RelativeLayoutParameter::RelativeAlign::NONE:
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT:
                    finalPositionX_ = ap.x * cs.width;
                    finalPositionY_ = layoutSize.height - ((1.0f - ap.y) * cs.height);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_CENTER_HORIZONTAL:
                    finalPositionX_ = layoutSize.width * 0.5f - cs.width * (0.5f - ap.x);
                    finalPositionY_ = layoutSize.height - ((1.0f - ap.y) * cs.height);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT:
                    finalPositionX_ = layoutSize.width - ((1.0f - ap.x) * cs.width);
                    finalPositionY_ = layoutSize.height - ((1.0f - ap.y) * cs.height);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL:
                    finalPositionX_ = ap.x * cs.width;
                    finalPositionY_ = layoutSize.height * 0.5f - cs.height * (0.5f - ap.y);
                    break;
                case RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT:
                    finalPositionX_ = layoutSize.width * 0.5f - cs.width * (0.5f - ap.x);
                    finalPositionY_ = layoutSize.height * 0.5f - cs.height * (0.5f - ap.y);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL:
                    finalPositionX_ = layoutSize.width - ((1.0f - ap.x) * cs.width);
                    finalPositionY_ = layoutSize.height * 0.5f - cs.height * (0.5f - ap.y);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_BOTTOM:
                    finalPositionX_ = ap.x * cs.width;
                    finalPositionY_ = ap.y * cs.height;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_BOTTOM_CENTER_HORIZONTAL:
                    finalPositionX_ = layoutSize.width * 0.5f - cs.width * (0.5f - ap.x);
                    finalPositionY_ = ap.y * cs.height;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_BOTTOM:
                    finalPositionX_ = layoutSize.width - ((1.0f - ap.x) * cs.width);
                    finalPositionY_ = ap.y * cs.height;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_LEFTALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        finalPositionY_ = locationTop + ap.y * cs.height;
                        finalPositionX_ = locationLeft + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationTop = relativeWidget->getTopBoundary();

                        finalPositionY_ = locationTop + ap.y * cs.height;
                        finalPositionX_ = relativeWidget->getLeftBoundary() + rbs.width * 0.5f + ap.x * cs.width - cs.width * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_RIGHTALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        finalPositionY_ = locationTop + ap.y * cs.height;
                        finalPositionX_ = locationRight - (1.0f - ap.x) * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_TOPALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        finalPositionY_ = locationTop - (1.0f - ap.y) * cs.height;
                        finalPositionX_ = locationLeft - (1.0f - ap.x) * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_CENTER:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        finalPositionX_ = locationLeft - (1.0f - ap.x) * cs.width;

                        finalPositionY_ = relativeWidget->getBottomBoundary() + rbs.height * 0.5f + ap.y * cs.height - cs.height * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_BOTTOMALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        finalPositionY_ = locationBottom + ap.y * cs.height;
                        finalPositionX_ = locationLeft - (1.0f - ap.x) * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_TOPALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        finalPositionY_ = locationTop - (1.0f - ap.y) * cs.height;
                        finalPositionX_ = locationRight + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_CENTER:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationRight = relativeWidget->getRightBoundary();
                        finalPositionX_ = locationRight + ap.x * cs.width;

                        finalPositionY_ = relativeWidget->getBottomBoundary() + rbs.height * 0.5f + ap.y * cs.height - cs.height * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_BOTTOMALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        finalPositionY_ = locationBottom + ap.y * cs.height;
                        finalPositionX_ = locationRight + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_LEFTALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        finalPositionY_ = locationBottom - (1.0f - ap.y) * cs.height;
                        finalPositionX_ = locationLeft + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_CENTER:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationBottom = relativeWidget->getBottomBoundary();

                        finalPositionY_ = locationBottom - (1.0f - ap.y) * cs.height;
                        finalPositionX_ = relativeWidget->getLeftBoundary() + rbs.width * 0.5f + ap.x * cs.width - cs.width * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_RIGHTALIGN:
                    if (relativeWidget)
                    {
                        if (relativeWidgetLP_ && !relativeWidgetLP_->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        finalPositionY_ = locationBottom - (1.0f - ap.y) * cs.height;
                        finalPositionX_ = locationRight - (1.0f - ap.x) * cs.width;
                    }
                    break;
                default:
                    break;
            }
            return true;
        }

        void RelativeLayoutManager::caculateFinalPositionWithRelativeAlign()
        {
            RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(widget_->getLayoutParameter());

            Margin mg = layoutParameter->getMargin();


            RelativeLayoutParameter::RelativeAlign align = layoutParameter->getAlign();

            //handle margin
            switch (align)
            {
                case RelativeLayoutParameter::RelativeAlign::NONE:
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT:
                    finalPositionX_ += mg.left;
                    finalPositionY_ -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_CENTER_HORIZONTAL:
                    finalPositionY_ -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT:
                    finalPositionX_ -= mg.right;
                    finalPositionY_ -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL:
                    finalPositionX_ += mg.left;
                    break;
                case RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT:
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL:
                    finalPositionX_ -= mg.right;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_BOTTOM:
                    finalPositionX_ += mg.left;
                    finalPositionY_ += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_BOTTOM_CENTER_HORIZONTAL:
                    finalPositionY_ += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_BOTTOM:
                    finalPositionX_ -= mg.right;
                    finalPositionY_ += mg.bottom;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_LEFTALIGN:
                    finalPositionY_ += mg.bottom;
                    finalPositionX_ += mg.left;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_RIGHTALIGN:
                    finalPositionY_ += mg.bottom;
                    finalPositionX_ -= mg.right;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER:
                    finalPositionY_ += mg.bottom;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_TOPALIGN:
                    finalPositionX_ -= mg.right;
                    finalPositionY_ -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_BOTTOMALIGN:
                    finalPositionX_ -= mg.right;
                    finalPositionY_ += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_CENTER:
                    finalPositionX_ -= mg.right;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_TOPALIGN:
                    finalPositionX_ += mg.left;
                    finalPositionY_ -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_BOTTOMALIGN:
                    finalPositionX_ += mg.left;
                    finalPositionY_ += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_CENTER:
                    finalPositionX_ += mg.left;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_LEFTALIGN:
                    finalPositionY_ -= mg.top;
                    finalPositionX_ += mg.left;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_RIGHTALIGN:
                    finalPositionY_ -= mg.top;
                    finalPositionX_ -= mg.right;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_CENTER:
                    finalPositionY_ -= mg.top;
                    break;
                default:
                    break;
            }
        }

        void RelativeLayoutManager::doLayout(LayoutProtocol *layout)
        {
            widgetChildren_ = this->getAllWidgets(layout);

            while (unlayoutChildCount_ > 0)
            {
                for (auto& subWidget : widgetChildren_)
                {
                    widget_ = static_cast<Widget*>(subWidget);

                    RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(widget_->getLayoutParameter());

                    if (layoutParameter)
                    {
                        if (layoutParameter->_put)
                        {
                            continue;
                        }


                        bool ret = this->caculateFinalPositionWithRelativeWidget(layout);
                        if (!ret) {
                            continue;
                        }

                        this->caculateFinalPositionWithRelativeAlign();


                        widget_->setPosition(MATH::Vector2f(finalPositionX_, finalPositionY_));

                        layoutParameter->_put = true;
                    }
                }
                unlayoutChildCount_--;

            }
            widgetChildren_.clear();
        }

        LayoutComponent::LayoutComponent()
            :horizontalEdge_(HorizontalEdge::None)
            , verticalEdge_(VerticalEdge::None)
            , leftMargin_(0)
            , rightMargin_(0)
            , bottomMargin_(0)
            , topMargin_(0)
            , usingPositionPercentX_(false)
            , positionPercentX_(0)
            , usingPositionPercentY_(false)
            , positionPercentY_(0)
            , usingStretchWidth_(false)
            , usingStretchHeight_(false)
            , percentWidth_(0)
            , usingPercentWidth_(false)
            , percentHeight_(0)
            , usingPercentHeight_(false)
            , actived_(true)
            , isPercentOnly_(false)
        {
            name_ = __LAYOUT_COMPONENT_NAME;
        }

        LayoutComponent::~LayoutComponent()
        {

        }

        LayoutComponent* LayoutComponent::bindLayoutComponent(Node* node)
        {
            LayoutComponent * layout = (LayoutComponent*)node->getComponent(__LAYOUT_COMPONENT_NAME);
            if (layout != nullptr)
                return layout;

            layout = new (std::nothrow) LayoutComponent();
            if (layout && layout->init())
            {
                layout->autorelease();
                node->addComponent(layout);
                return layout;
            }
            SAFE_DELETE(layout);
            return nullptr;
        }

        bool LayoutComponent::init()
        {
            bool ret = true;
            do
            {
                if (!Component::init())
                {
                    ret = false;
                    break;
                }

                //put layout component initalized code here

            } while (0);
            return ret;
        }

        Node* LayoutComponent::getOwnerParent()
        {
            Node* parent = owner_->getParent();
            return parent;
        }
        void LayoutComponent::refreshHorizontalMargin()
        {
            Node* parent = this->getOwnerParent();
            if (parent == nullptr)
                return;

            const MATH::Vector2f& ownerPoint = owner_->getPosition();
            const MATH::Vector2f& ownerAnchor = owner_->getAnchorPoint();
            const MATH::Sizef& ownerSize = owner_->getContentSize();
            const MATH::Sizef& parentSize = parent->getContentSize();

            leftMargin_ = ownerPoint.x - ownerAnchor.x * ownerSize.width;
            rightMargin_ = parentSize.width - (ownerPoint.x + (1 - ownerAnchor.x) * ownerSize.width);
        }
        void LayoutComponent::refreshVerticalMargin()
        {
            Node* parent = this->getOwnerParent();
            if (parent == nullptr)
                return;

            const MATH::Vector2f& ownerPoint = owner_->getPosition();
            const MATH::Vector2f& ownerAnchor = owner_->getAnchorPoint();
            const MATH::Sizef& ownerSize = owner_->getContentSize();
            const MATH::Sizef& parentSize = parent->getContentSize();

            bottomMargin_ = ownerPoint.y - ownerAnchor.y * ownerSize.height;
            topMargin_ = parentSize.height - (ownerPoint.y + (1 - ownerAnchor.y) * ownerSize.height);
        }

        //OldVersion
        void LayoutComponent::setUsingPercentContentSize(bool isUsed)
        {
            usingPercentWidth_ = usingPercentHeight_ = isUsed;
        }

        bool LayoutComponent::getUsingPercentContentSize()const
        {
            return usingPercentWidth_ && usingPercentHeight_;
        }

        void LayoutComponent::setPercentContentSize(const MATH::Vector2f &percent)
        {
            this->setPercentWidth(percent.x);
            this->setPercentHeight(percent.y);
        }

        MATH::Vector2f LayoutComponent::getPercentContentSize()const
        {
            MATH::Vector2f vec2 = MATH::Vector2f(percentWidth_,percentHeight_);
            return vec2;
        }

        //Position & Margin
        const MATH::Vector2f& LayoutComponent::getAnchorPosition()const
        {
            return owner_->getAnchorPoint();
        }

        void LayoutComponent::setAnchorPosition(const MATH::Vector2f& point)
        {
            MATH::Rectf oldRect = owner_->getBoundingBox();
            owner_->setAnchorPoint(point);
            MATH::Rectf newRect = owner_->getBoundingBox();
            float offSetX = oldRect.origin.x - newRect.origin.x;
            float offSetY = oldRect.origin.y - newRect.origin.y;

            MATH::Vector2f ownerPosition = owner_->getPosition();
            ownerPosition.x += offSetX;
            ownerPosition.y += offSetY;

            this->setPosition(ownerPosition);
        }

        const MATH::Vector2f& LayoutComponent::getPosition()const
        {
            return owner_->getPosition();
        }

        void LayoutComponent::setPosition(const MATH::Vector2f& position)
        {
            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Vector2f ownerPoint = position;
                const MATH::Sizef& parentSize = parent->getContentSize();

                if (parentSize.width != 0)
                    positionPercentX_ = ownerPoint.x / parentSize.width;
                else
                {
                    positionPercentX_ = 0;
                    if (usingPositionPercentX_)
                        ownerPoint.x = 0;
                }

                if (parentSize.height != 0)
                    positionPercentY_ = ownerPoint.y / parentSize.height;
                else
                {
                    positionPercentY_ = 0;
                    if (usingPositionPercentY_)
                        ownerPoint.y = 0;
                }

                owner_->setPosition(ownerPoint);

                this->refreshHorizontalMargin();
                this->refreshVerticalMargin();
            }
            else
                owner_->setPosition(position);
        }

        bool LayoutComponent::isPositionPercentXEnabled()const
        {
            return usingPositionPercentX_;
        }
        void LayoutComponent::setPositionPercentXEnabled(bool isUsed)
        {
            usingPositionPercentX_ = isUsed;
            if (usingPositionPercentX_)
            {
                horizontalEdge_ = HorizontalEdge::None;
            }
        }

        float LayoutComponent::getPositionPercentX()const
        {
            return positionPercentX_;
        }

        void LayoutComponent::setPositionPercentX(float percentMargin)
        {
            positionPercentX_ = percentMargin;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                owner_->setPositionX(parent->getContentSize().width * positionPercentX_);
                this->refreshHorizontalMargin();
            }
        }

        bool LayoutComponent::isPositionPercentYEnabled()const
        {
            return usingPositionPercentY_;
        }
        void LayoutComponent::setPositionPercentYEnabled(bool isUsed)
        {
            usingPositionPercentY_ = isUsed;
            if (usingPositionPercentY_)
            {
                verticalEdge_ = VerticalEdge::None;
            }
        }

        float LayoutComponent::getPositionPercentY()const
        {
            return positionPercentY_;
        }
        void LayoutComponent::setPositionPercentY(float percentMargin)
        {
            positionPercentY_ = percentMargin;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                owner_->setPositionY(parent->getContentSize().height * positionPercentY_);
                this->refreshVerticalMargin();
            }
        }

        LayoutComponent::HorizontalEdge LayoutComponent::getHorizontalEdge()const
        {
            return horizontalEdge_;
        }
        void LayoutComponent::setHorizontalEdge(HorizontalEdge hEage)
        {
            horizontalEdge_ = hEage;
            if (horizontalEdge_ != HorizontalEdge::None)
            {
                usingPositionPercentX_ = false;
            }

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Vector2f ownerPoint = owner_->getPosition();
                const MATH::Sizef& parentSize = parent->getContentSize();
                if (parentSize.width != 0)
                    positionPercentX_ = ownerPoint.x / parentSize.width;
                else
                {
                    positionPercentX_ = 0;
                    ownerPoint.x = 0;
                    if (usingPositionPercentX_)
                        owner_->setPosition(ownerPoint);
                }

                this->refreshHorizontalMargin();
            }
        }

        LayoutComponent::VerticalEdge LayoutComponent::getVerticalEdge()const
        {
            return verticalEdge_;
        }
        void LayoutComponent::setVerticalEdge(VerticalEdge vEage)
        {
            verticalEdge_ = vEage;
            if (verticalEdge_ != VerticalEdge::None)
            {
                usingPositionPercentY_ = false;
            }

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Vector2f ownerPoint = owner_->getPosition();
                const MATH::Sizef& parentSize = parent->getContentSize();
                if (parentSize.height != 0)
                    positionPercentY_ = ownerPoint.y / parentSize.height;
                else
                {
                    positionPercentY_ = 0;
                    ownerPoint.y = 0;
                    if (usingPositionPercentY_)
                        owner_->setPosition(ownerPoint);
                }

                this->refreshVerticalMargin();
            }
        }

        float LayoutComponent::getLeftMargin()const
        {
            return leftMargin_;
        }
        void LayoutComponent::setLeftMargin(float margin)
        {
            leftMargin_ = margin;
        }

        float LayoutComponent::getRightMargin()const
        {
            return rightMargin_;
        }
        void LayoutComponent::setRightMargin(float margin)
        {
            rightMargin_ = margin;
        }

        float LayoutComponent::getTopMargin()const
        {
            return topMargin_;
        }
        void LayoutComponent::setTopMargin(float margin)
        {
            topMargin_ = margin;
        }

        float LayoutComponent::getBottomMargin()const
        {
            return bottomMargin_;
        }
        void LayoutComponent::setBottomMargin(float margin)
        {
            bottomMargin_ = margin;
        }

        //Size & Percent
        const MATH::Sizef& LayoutComponent::getSize()const
        {
            return this->getOwner()->getContentSize();
        }
        void LayoutComponent::setSize(const MATH::Sizef& size)
        {
            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Sizef ownerSize = size;
                const MATH::Sizef& parentSize = parent->getContentSize();

                if (parentSize.width != 0)
                    percentWidth_ = ownerSize.width / parentSize.width;
                else
                {
                    percentWidth_ = 0;
                    if (usingPercentWidth_)
                        ownerSize.width = 0;
                }

                if (parentSize.height != 0)
                    percentHeight_ = ownerSize.height / parentSize.height;
                else
                {
                    percentHeight_ = 0;
                    if (usingPercentHeight_)
                        ownerSize.height = 0;
                }

                owner_->setContentSize(ownerSize);

                this->refreshHorizontalMargin();
                this->refreshVerticalMargin();
            }
            else
                owner_->setContentSize(size);
        }

        bool LayoutComponent::isPercentWidthEnabled()const
        {
            return usingPercentWidth_;
        }
        void LayoutComponent::setPercentWidthEnabled(bool isUsed)
        {
            usingPercentWidth_ = isUsed;
            if (usingPercentWidth_)
            {
                usingStretchWidth_ = false;
            }
        }

        float LayoutComponent::getSizeWidth()const
        {
            return owner_->getContentSize().width;
        }
        void LayoutComponent::setSizeWidth(float width)
        {
            MATH::Sizef ownerSize = owner_->getContentSize();
            ownerSize.width = width;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                const MATH::Sizef& parentSize = parent->getContentSize();
                if (parentSize.width != 0)
                    percentWidth_ = ownerSize.width / parentSize.width;
                else
                {
                    percentWidth_ = 0;
                    if (usingPercentWidth_)
                        ownerSize.width = 0;
                }
                owner_->setContentSize(ownerSize);
                this->refreshHorizontalMargin();
            }
            else
                owner_->setContentSize(ownerSize);
        }

        float LayoutComponent::getPercentWidth()const
        {
            return percentWidth_;
        }
        void LayoutComponent::setPercentWidth(float percentWidth)
        {
            percentWidth_ = percentWidth;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Sizef ownerSize = owner_->getContentSize();
                ownerSize.width = parent->getContentSize().width * percentWidth_;
                owner_->setContentSize(ownerSize);

                this->refreshHorizontalMargin();
            }
        }

        bool LayoutComponent::isPercentHeightEnabled()const
        {
            return usingPercentHeight_;
        }
        void LayoutComponent::setPercentHeightEnabled(bool isUsed)
        {
            usingPercentHeight_ = isUsed;
            if (usingPercentHeight_)
            {
                usingStretchHeight_ = false;
            }
        }

        float LayoutComponent::getSizeHeight()const
        {
            return owner_->getContentSize().height;
        }
        void LayoutComponent::setSizeHeight(float height)
        {
            MATH::Sizef ownerSize = owner_->getContentSize();
            ownerSize.height = height;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                const MATH::Sizef& parentSize = parent->getContentSize();
                if (parentSize.height != 0)
                    percentHeight_ = ownerSize.height / parentSize.height;
                else
                {
                    percentHeight_ = 0;
                    if (usingPercentHeight_)
                        ownerSize.height = 0;
                }
                owner_->setContentSize(ownerSize);
                this->refreshVerticalMargin();
            }
            else
                owner_->setContentSize(ownerSize);
        }

        float LayoutComponent::getPercentHeight()const
        {
            return percentHeight_;
        }
        void LayoutComponent::setPercentHeight(float percentHeight)
        {
            percentHeight_ = percentHeight;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Sizef ownerSize = owner_->getContentSize();
                ownerSize.height = parent->getContentSize().height * percentHeight_;
                owner_->setContentSize(ownerSize);

                this->refreshVerticalMargin();
            }
        }

        bool LayoutComponent::isStretchWidthEnabled()const
        {
            return usingStretchWidth_;
        }
        void LayoutComponent::setStretchWidthEnabled(bool isUsed)
        {
            usingStretchWidth_ = isUsed;
            if (usingStretchWidth_)
            {
                usingPercentWidth_ = false;
            }
        }

        bool LayoutComponent::isStretchHeightEnabled()const
        {
            return usingStretchHeight_;
        }
        void LayoutComponent::setStretchHeightEnabled(bool isUsed)
        {
            usingStretchHeight_ = isUsed;
            if (usingStretchHeight_)
            {
                usingPercentHeight_ = false;
            }
        }

        void LayoutComponent::refreshLayout()
        {
            if (!actived_)
                return;

            Node* parent = this->getOwnerParent();
            if (parent == nullptr)
                return;

            const MATH::Sizef& parentSize = parent->getContentSize();
            const MATH::Vector2f& ownerAnchor = owner_->getAnchorPoint();
            MATH::Sizef ownerSize = owner_->getContentSize();
            MATH::Vector2f ownerPosition = owner_->getPosition();

            switch (this->horizontalEdge_)
            {
            case HorizontalEdge::None:
                if (usingStretchWidth_ && !isPercentOnly_)
                {
                    ownerSize.width = parentSize.width * percentWidth_;
                    ownerPosition.x = leftMargin_ + ownerAnchor.x * ownerSize.width;
                }
                else
                {
                    if (usingPositionPercentX_)
                        ownerPosition.x = parentSize.width * positionPercentX_;
                    if (usingPercentWidth_)
                        ownerSize.width = parentSize.width * percentWidth_;
                }
                break;
            case HorizontalEdge::Left:
                if (isPercentOnly_)
                    break;
                if (usingPercentWidth_ || usingStretchWidth_)
                    ownerSize.width = parentSize.width * percentWidth_;
                ownerPosition.x = leftMargin_ + ownerAnchor.x * ownerSize.width;
                break;
            case HorizontalEdge::Right:
                if (isPercentOnly_)
                    break;
                if (usingPercentWidth_ || usingStretchWidth_)
                    ownerSize.width = parentSize.width * percentWidth_;
                ownerPosition.x = parentSize.width - (rightMargin_ + (1 - ownerAnchor.x) * ownerSize.width);
                break;
            case HorizontalEdge::Center:
                if (isPercentOnly_)
                    break;
                if (usingStretchWidth_)
                {
                    ownerSize.width = parentSize.width - leftMargin_ - rightMargin_;
                    if (ownerSize.width < 0)
                        ownerSize.width = 0;
                    ownerPosition.x = leftMargin_ + ownerAnchor.x * ownerSize.width;
                }
                else
                {
                    if (usingPercentWidth_)
                        ownerSize.width = parentSize.width * percentWidth_;
                    ownerPosition.x = parentSize.width * positionPercentX_;
                }
                break;
            default:
                break;
            }

            switch (this->verticalEdge_)
            {
            case VerticalEdge::None:
                if (usingStretchHeight_ && !isPercentOnly_)
                {
                    ownerSize.height = parentSize.height * percentHeight_;
                    ownerPosition.y = bottomMargin_ + ownerAnchor.y * ownerSize.height;
                }
                else
                {
                    if (usingPositionPercentY_)
                        ownerPosition.y = parentSize.height * positionPercentY_;
                    if (usingPercentHeight_)
                        ownerSize.height = parentSize.height * percentHeight_;
                }
                break;
            case VerticalEdge::Bottom:
                if (isPercentOnly_)
                    break;
                if (usingPercentHeight_ || usingStretchHeight_)
                    ownerSize.height = parentSize.height * percentHeight_;
                ownerPosition.y = bottomMargin_ + ownerAnchor.y * ownerSize.height;
                break;
            case VerticalEdge::Top:
                if (isPercentOnly_)
                    break;
                if (usingPercentHeight_ || usingStretchHeight_)
                    ownerSize.height = parentSize.height * percentHeight_;
                ownerPosition.y = parentSize.height - (topMargin_ + (1 - ownerAnchor.y) * ownerSize.height);
                break;
            case VerticalEdge::Center:
                if (isPercentOnly_)
                    break;
                if (usingStretchHeight_)
                {
                    ownerSize.height = parentSize.height - topMargin_ - bottomMargin_;
                    if (ownerSize.height < 0)
                        ownerSize.height = 0;
                    ownerPosition.y = bottomMargin_ + ownerAnchor.y * ownerSize.height;
                }
                else
                {
                    if (usingPercentHeight_)
                        ownerSize.height = parentSize.height * percentHeight_;
                    ownerPosition.y = parentSize.height* positionPercentY_;
                }
                break;
            default:
                break;
            }

            owner_->setPosition(ownerPosition);
            owner_->setContentSize(ownerSize);

            Helper::doLayout(owner_);
        }

        void LayoutComponent::setActiveEnabled(bool enable)
        {
            actived_ = enable;
        }

        void LayoutComponent::setPercentOnlyEnabled(bool enable)
        {
            isPercentOnly_ = enable;
        }
    }
}
