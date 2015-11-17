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
        _backGroundScale9Enabled(false),
        _backGroundImage(nullptr),
        _backGroundImageFileName(""),
        _backGroundImageCapInsets(MATH::RectfZERO),
        _colorType(BackGroundColorType::NONE),
        _bgImageTexType(TextureResType::LOCAL),
        _backGroundImageTextureSize(MATH::SizefZERO),
        _backGroundImageColor(Color3B::WHITE),
        _backGroundImageOpacity(255),
        _colorRender(nullptr),
        _gradientRender(nullptr),
        _cColor(Color3B::WHITE),
        _gStartColor(Color3B::WHITE),
        _gEndColor(Color3B::WHITE),
        _alongVector(MATH::Vector2f(0.0f, -1.0f)),
        _cOpacity(255),
        _clippingEnabled(false),
        _layoutType(Type::TABSOLUTE),
        _clippingType(ClippingType::STENCIL),
        _clippingStencil(nullptr),
        _scissorRectDirty(false),
        _clippingRect(MATH::RectfZERO),
        _clippingParent(nullptr),
        _clippingRectDirty(true),
        _groupCommand(Director::getInstance().getRenderer()),
        _doLayoutDirty(true),
        _isInterceptTouch(false),
        _loopFocus(false),
        _passFocusToChild(true),
        _isFocusPassing(false),
        u3dContext_(Unity3DCreator::CreateContext())
        {
            //no-op
        }

        Layout::~Layout()
        {
            SAFE_RELEASE(_clippingStencil);
            SAFE_RELEASE(u3dVertexFormat_);
            SAFE_RELEASE(u3dContext_);
        }

        void Layout::onEnter()
        {
            Widget::onEnter();
            if (_clippingStencil)
            {
                _clippingStencil->onEnter();
            }
            _doLayoutDirty = true;
            _clippingRectDirty = true;
        }

        void Layout::onExit()
        {
            Widget::onExit();
            if (_clippingStencil)
            {
                _clippingStencil->onExit();
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
            _doLayoutDirty = true;
        }

        void Layout::addChild(Node* child, int zOrder, const std::string &name)
        {
            if (dynamic_cast<Widget*>(child)) {
                supplyTheLayoutParameterLackToChild(static_cast<Widget*>(child));
            }
            Widget::addChild(child, zOrder, name);
            _doLayoutDirty = true;
        }

        void Layout::removeChild(Node *child, bool cleanup)
        {
            Widget::removeChild(child, cleanup);
            _doLayoutDirty = true;
        }

        void Layout::removeAllChildren()
        {
            Widget::removeAllChildren();
            _doLayoutDirty = true;
        }

        void Layout::removeAllChildrenWithCleanup(bool cleanup)
        {
            Widget::removeAllChildrenWithCleanup(cleanup);
            _doLayoutDirty = true;
        }

        bool Layout::isClippingEnabled()const
        {
            return _clippingEnabled;
        }

        void Layout::visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
        {
            if (!visible_)
            {
                return;
            }

            adaptRenderers();
            doLayout();

            if (_clippingEnabled)
            {
                switch (_clippingType)
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

            _groupCommand.init(globalZOrder_);
            renderer->addCommand(&_groupCommand);

            renderer->pushGroup(_groupCommand.getRenderQueueID());

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
            if (able == _clippingEnabled)
            {
                return;
            }
            _clippingEnabled = able;
            switch (_clippingType)
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
                        _clippingStencil = DrawNode::create();
                        if (running_)
                        {
                            _clippingStencil->onEnter();
                        }
                        _clippingStencil->retain();
                        setStencilClippingSize(contentSize_);
                    }
                    else
                    {
                        if (running_)
                        {
                            _clippingStencil->onExit();
                        }
                        _clippingStencil->release();
                        _clippingStencil = nullptr;
                    }
                    break;
                default:
                    break;
            }
        }

        void Layout::setClippingType(ClippingType type)
        {
            if (type == _clippingType)
            {
                return;
            }
            bool clippingEnabled = isClippingEnabled();
            setClippingEnabled(false);
            _clippingType = type;
            setClippingEnabled(clippingEnabled);
        }

        Layout::ClippingType Layout::getClippingType()const
        {
            return _clippingType;
        }

        void Layout::setStencilClippingSize(const MATH::Sizef &)
        {
            if (_clippingEnabled && _clippingType == ClippingType::STENCIL)
            {
                MATH::Vector2f rect[4];
                // rect[0].setZero(); Zero default
                rect[1].set(contentSize_.width, 0.0f);
                rect[2].set(contentSize_.width, contentSize_.height);
                rect[3].set(0.0f, contentSize_.height);
                Color4F green(0.0f, 1.0f, 0.0f, 1.0f);
                _clippingStencil->clear();
                _clippingStencil->drawPolygon(rect, 4, green, 0, green);
            }
        }

        const MATH::Rectf& Layout::getClippingRect()
        {
            if (_clippingRectDirty)
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
                            _clippingParent = parent;
                            break;
                        }
                    }
                }

                if (_clippingParent)
                {
                    parentClippingRect = _clippingParent->getClippingRect();
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
                    _clippingRect.origin.x = finalX;
                    _clippingRect.origin.y = finalY;
                    _clippingRect.size.width = finalWidth;
                    _clippingRect.size.height = finalHeight;
                }
                else
                {
                    _clippingRect.origin.x = worldPos.x - (scissorWidth * anchorPoint_.x);
                    _clippingRect.origin.y = worldPos.y - (scissorHeight * anchorPoint_.y);
                    _clippingRect.size.width = scissorWidth;
                    _clippingRect.size.height = scissorHeight;
                }
                _clippingRectDirty = false;
            }
            return _clippingRect;
        }

        void Layout::onSizeChanged()
        {
            Widget::onSizeChanged();
            setStencilClippingSize(contentSize_);
            _doLayoutDirty = true;
            _clippingRectDirty = true;
            if (_backGroundImage)
            {
                _backGroundImage->setPosition(contentSize_.width/2.0f, contentSize_.height/2.0f);
                if (_backGroundScale9Enabled && _backGroundImage)
                {
                    _backGroundImage->setPreferredSize(contentSize_);
                }
            }
            if (_colorRender)
            {
                _colorRender->setContentSize(contentSize_);
            }
            if (_gradientRender)
            {
                _gradientRender->setContentSize(contentSize_);
            }
        }

        void Layout::setBackGroundImageScale9Enabled(bool able)
        {
            if (_backGroundScale9Enabled == able)
            {
                return;
            }
            _backGroundScale9Enabled = able;
            if (nullptr == _backGroundImage)
            {
                addBackGroundImage();
                setBackGroundImage(_backGroundImageFileName,_bgImageTexType);
            }
            _backGroundImage->setScale9Enabled(_backGroundScale9Enabled);
            setBackGroundImageCapInsets(_backGroundImageCapInsets);
        }

        bool Layout::isBackGroundImageScale9Enabled()const
        {
            return _backGroundScale9Enabled;
        }

        void Layout::setBackGroundImage(const std::string& fileName,TextureResType texType)
        {
            if (fileName.empty())
            {
                return;
            }
            if (_backGroundImage == nullptr)
            {
                addBackGroundImage();
                _backGroundImage->setScale9Enabled(_backGroundScale9Enabled);
            }
            _backGroundImageFileName = fileName;
            _bgImageTexType = texType;

            switch (_bgImageTexType)
            {
                case TextureResType::LOCAL:
                    _backGroundImage->initWithFile(fileName);
                    break;
                case TextureResType::PLIST:
                    _backGroundImage->initWithSpriteFrameName(fileName);
                    break;
                default:
                    break;
            }
            if (_backGroundScale9Enabled) {
                _backGroundImage->setPreferredSize(contentSize_);
            }

            _backGroundImageTextureSize = _backGroundImage->getContentSize();
            _backGroundImage->setPosition(contentSize_.width/2.0f, contentSize_.height/2.0f);
            updateBackGroundImageRGBA();
        }

        void Layout::setBackGroundImageCapInsets(const MATH::Rectf &capInsets)
        {
            _backGroundImageCapInsets = capInsets;
            if (_backGroundScale9Enabled && _backGroundImage)
            {
                _backGroundImage->setCapInsets(capInsets);
            }
        }

        const MATH::Rectf& Layout::getBackGroundImageCapInsets()const
        {
            return _backGroundImageCapInsets;
        }

        void Layout::supplyTheLayoutParameterLackToChild(Widget *child)
        {
            if (!child)
            {
                return;
            }
            switch (_layoutType)
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
            _backGroundImage = Scale9Sprite::create();
            _backGroundImage->setScale9Enabled(false);

            addProtectedChild(_backGroundImage, BACKGROUNDIMAGE_Z, -1);

            _backGroundImage->setPosition(contentSize_.width/2.0f, contentSize_.height/2.0f);
        }

        void Layout::removeBackGroundImage()
        {
            if (!_backGroundImage)
            {
                return;
            }
            removeProtectedChild(_backGroundImage);
            _backGroundImage = nullptr;
            _backGroundImageFileName = "";
            _backGroundImageTextureSize = MATH::SizefZERO;
        }

        void Layout::setBackGroundColorType(BackGroundColorType type)
        {
            if (_colorType == type)
            {
                return;
            }
            switch (_colorType)
            {
                case BackGroundColorType::NONE:
                    if (_colorRender)
                    {
                        removeProtectedChild(_colorRender);
                        _colorRender = nullptr;
                    }
                    if (_gradientRender)
                    {
                        removeProtectedChild(_gradientRender);
                        _gradientRender = nullptr;
                    }
                    break;
                case BackGroundColorType::SOLID:
                    if (_colorRender)
                    {
                        removeProtectedChild(_colorRender);
                        _colorRender = nullptr;
                    }
                    break;
                case BackGroundColorType::GRADIENT:
                    if (_gradientRender)
                    {
                        removeProtectedChild(_gradientRender);
                        _gradientRender = nullptr;
                    }
                    break;
                default:
                    break;
            }
            _colorType = type;
            switch (_colorType)
            {
                case BackGroundColorType::NONE:
                    break;
                case BackGroundColorType::SOLID:
                    _colorRender = LayerColor::create();
                    _colorRender->setContentSize(contentSize_);
                    _colorRender->setOpacity(_cOpacity);
                    _colorRender->setColor(_cColor);
                    addProtectedChild(_colorRender, BCAKGROUNDCOLORRENDERER_Z, -1);
                    break;
                case BackGroundColorType::GRADIENT:
                    _gradientRender = LayerGradient::create();
                    _gradientRender->setContentSize(contentSize_);
                    _gradientRender->setOpacity(_cOpacity);
                    _gradientRender->setStartColor(_gStartColor);
                    _gradientRender->setEndColor(_gEndColor);
                    _gradientRender->setVector(_alongVector);
                    addProtectedChild(_gradientRender, BCAKGROUNDCOLORRENDERER_Z, -1);
                    break;
                default:
                    break;
            }
        }

        Layout::BackGroundColorType Layout::getBackGroundColorType()const
        {
            return _colorType;
        }

        void Layout::setBackGroundColor(const Color3B &color)
        {
            _cColor = color;
            if (_colorRender)
            {
                _colorRender->setColor(color);
            }
        }

        const Color3B& Layout::getBackGroundColor()const
        {
            return _cColor;
        }

        void Layout::setBackGroundColor(const Color3B &startColor, const Color3B &endColor)
        {
            _gStartColor = startColor;
            if (_gradientRender)
            {
                _gradientRender->setStartColor(startColor);
            }
            _gEndColor = endColor;
            if (_gradientRender)
            {
                _gradientRender->setEndColor(endColor);
            }
        }

        const Color3B& Layout::getBackGroundStartColor()const
        {
            return _gStartColor;
        }

        const Color3B& Layout::getBackGroundEndColor()const
        {
            return _gEndColor;
        }

        void Layout::setBackGroundColorOpacity(uint8 opacity)
        {
            _cOpacity = opacity;
            switch (_colorType)
            {
                case BackGroundColorType::NONE:
                    break;
                case BackGroundColorType::SOLID:
                    _colorRender->setOpacity(opacity);
                    break;
                case BackGroundColorType::GRADIENT:
                    _gradientRender->setOpacity(opacity);
                    break;
                default:
                    break;
            }
        }

        uint8 Layout::getBackGroundColorOpacity()const
        {
            return _cOpacity;
        }

        void Layout::setBackGroundColorVector(const MATH::Vector2f &vector)
        {
            _alongVector = vector;
            if (_gradientRender)
            {
                _gradientRender->setVector(vector);
            }
        }

        const MATH::Vector2f& Layout::getBackGroundColorVector()const
        {
            return _alongVector;
        }

        void Layout::setBackGroundImageColor(const Color3B &color)
        {
            _backGroundImageColor = color;
            updateBackGroundImageColor();
        }

        void Layout::setBackGroundImageOpacity(uint8 opacity)
        {
            _backGroundImageOpacity = opacity;
            updateBackGroundImageOpacity();
        }

        const Color3B& Layout::getBackGroundImageColor()const
        {
            return _backGroundImageColor;
        }

        uint8 Layout::getBackGroundImageOpacity()const
        {
            return _backGroundImageOpacity;
        }

        void Layout::updateBackGroundImageColor()
        {
            if (_backGroundImage)
            {
                _backGroundImage->setColor(_backGroundImageColor);
            }
        }

        void Layout::updateBackGroundImageOpacity()
        {
            if (_backGroundImage)
            {
                _backGroundImage->setOpacity(_backGroundImageOpacity);
            }
        }

        void Layout::updateBackGroundImageRGBA()
        {
            if (_backGroundImage)
            {
                _backGroundImage->setColor(_backGroundImageColor);
                _backGroundImage->setOpacity(_backGroundImageOpacity);
            }
        }

        const MATH::Sizef& Layout::getBackGroundImageTextureSize() const
        {
            return _backGroundImageTextureSize;
        }

        void Layout::setLayoutType(Type type)
        {
            _layoutType = type;

            for (auto& child : children_)
            {
                Widget* widgetChild = dynamic_cast<Widget*>(child);
                if (widgetChild)
                {
                    supplyTheLayoutParameterLackToChild(static_cast<Widget*>(child));
                }
            }
            _doLayoutDirty = true;
        }



        Layout::Type Layout::getLayoutType() const
        {
            return _layoutType;
        }

        void Layout::forceDoLayout()
        {
            this->requestDoLayout();
            this->doLayout();
        }

        void Layout::requestDoLayout()
        {
            _doLayoutDirty = true;
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
            switch (_layoutType)
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

            if (!_doLayoutDirty)
            {
                return;
            }

            sortAllChildren();

            LayoutManager* executant = this->createLayoutManager();

            if (executant)
            {
                executant->doLayout(this);
            }

            _doLayoutDirty = false;
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
                setBackGroundImageScale9Enabled(layout->_backGroundScale9Enabled);
                setBackGroundImage(layout->_backGroundImageFileName,layout->_bgImageTexType);
                setBackGroundImageCapInsets(layout->_backGroundImageCapInsets);
                setBackGroundColorType(layout->_colorType);
                setBackGroundColor(layout->_cColor);
                setBackGroundColor(layout->_gStartColor, layout->_gEndColor);
                setBackGroundColorOpacity(layout->_cOpacity);
                setBackGroundColorVector(layout->_alongVector);
                setLayoutType(layout->_layoutType);
                setClippingEnabled(layout->_clippingEnabled);
                setClippingType(layout->_clippingType);
                _loopFocus = layout->_loopFocus;
                _passFocusToChild = layout->_passFocusToChild;
                _isInterceptTouch = layout->_isInterceptTouch;
            }
        }

        void Layout::setLoopFocus(bool loop)
        {
            _loopFocus = loop;
        }

        bool Layout::isLoopFocus()const
        {
            return _loopFocus;
        }


        void Layout::setPassFocusToChild(bool pass)
        {
            _passFocusToChild = pass;
        }

        bool Layout::isPassFocusToChild()const
        {
            return _passFocusToChild;
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
                    layout->_isFocusPassing = true;
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
                        layout->_isFocusPassing = true;
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
                if (_loopFocus)
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
                                layout->_isFocusPassing = true;
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
                            layout->_isFocusPassing = true;
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
                if (_loopFocus)
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
                                layout->_isFocusPassing = true;
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
            if (_isFocusPassing || this->isFocused())
            {
                Layout* parent = dynamic_cast<Layout*>(this->getParent());
                _isFocusPassing = false;

                if (_passFocusToChild)
                {
                    Widget * w = this->passFocusToChild(direction, current);
                    if (dynamic_cast<Layout*>(w))
                    {
                        if (parent)
                        {
                            parent->_isFocusPassing = true;
                            return parent->findNextFocusedWidget(direction, this);
                        }
                    }
                    return w;
                }

                if (nullptr == parent)
                {
                    return this;
                }
                parent->_isFocusPassing = true;
                return parent->findNextFocusedWidget(direction, this);

            }
            else if(current->isFocused() || dynamic_cast<Layout*>(current))
            {
                if (_layoutType == Type::THORIZONTAL)
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
                else if (_layoutType == Type::TVERTICAL)
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
            if (_clippingStencil){
                _clippingStencil->setCameraMask(mask, applyChildren);
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
                    _unlayoutChildCount++;
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
                for (auto& sWidget : _widgetChildren)
                {
                    if (sWidget)
                    {
                        RelativeLayoutParameter* rlayoutParameter = dynamic_cast<RelativeLayoutParameter*>(sWidget->getLayoutParameter());
                        if (rlayoutParameter &&  rlayoutParameter->getRelativeName() == relativeName)
                        {
                            relativeWidget = sWidget;
                            _relativeWidgetLP = rlayoutParameter;
                            break;
                        }
                    }
                }
            }
            return relativeWidget;
        }

        bool RelativeLayoutManager::caculateFinalPositionWithRelativeWidget(LayoutProtocol *layout)
        {
            MATH::Vector2f ap = _widget->getAnchorPoint();
            MATH::Sizef cs = _widget->getContentSize();

            _finalPositionX = 0.0f;
            _finalPositionY = 0.0f;

            Widget* relativeWidget = this->getRelativeWidget(_widget);

            RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(_widget->getLayoutParameter());

            RelativeLayoutParameter::RelativeAlign align = layoutParameter->getAlign();

            MATH::Sizef layoutSize = layout->getLayoutContentSize();


            switch (align)
            {
                case RelativeLayoutParameter::RelativeAlign::NONE:
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT:
                    _finalPositionX = ap.x * cs.width;
                    _finalPositionY = layoutSize.height - ((1.0f - ap.y) * cs.height);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_CENTER_HORIZONTAL:
                    _finalPositionX = layoutSize.width * 0.5f - cs.width * (0.5f - ap.x);
                    _finalPositionY = layoutSize.height - ((1.0f - ap.y) * cs.height);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT:
                    _finalPositionX = layoutSize.width - ((1.0f - ap.x) * cs.width);
                    _finalPositionY = layoutSize.height - ((1.0f - ap.y) * cs.height);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL:
                    _finalPositionX = ap.x * cs.width;
                    _finalPositionY = layoutSize.height * 0.5f - cs.height * (0.5f - ap.y);
                    break;
                case RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT:
                    _finalPositionX = layoutSize.width * 0.5f - cs.width * (0.5f - ap.x);
                    _finalPositionY = layoutSize.height * 0.5f - cs.height * (0.5f - ap.y);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL:
                    _finalPositionX = layoutSize.width - ((1.0f - ap.x) * cs.width);
                    _finalPositionY = layoutSize.height * 0.5f - cs.height * (0.5f - ap.y);
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_BOTTOM:
                    _finalPositionX = ap.x * cs.width;
                    _finalPositionY = ap.y * cs.height;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_BOTTOM_CENTER_HORIZONTAL:
                    _finalPositionX = layoutSize.width * 0.5f - cs.width * (0.5f - ap.x);
                    _finalPositionY = ap.y * cs.height;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_BOTTOM:
                    _finalPositionX = layoutSize.width - ((1.0f - ap.x) * cs.width);
                    _finalPositionY = ap.y * cs.height;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_LEFTALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        _finalPositionY = locationTop + ap.y * cs.height;
                        _finalPositionX = locationLeft + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationTop = relativeWidget->getTopBoundary();

                        _finalPositionY = locationTop + ap.y * cs.height;
                        _finalPositionX = relativeWidget->getLeftBoundary() + rbs.width * 0.5f + ap.x * cs.width - cs.width * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_RIGHTALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        _finalPositionY = locationTop + ap.y * cs.height;
                        _finalPositionX = locationRight - (1.0f - ap.x) * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_TOPALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        _finalPositionY = locationTop - (1.0f - ap.y) * cs.height;
                        _finalPositionX = locationLeft - (1.0f - ap.x) * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_CENTER:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        _finalPositionX = locationLeft - (1.0f - ap.x) * cs.width;

                        _finalPositionY = relativeWidget->getBottomBoundary() + rbs.height * 0.5f + ap.y * cs.height - cs.height * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_BOTTOMALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        _finalPositionY = locationBottom + ap.y * cs.height;
                        _finalPositionX = locationLeft - (1.0f - ap.x) * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_TOPALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationTop = relativeWidget->getTopBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        _finalPositionY = locationTop - (1.0f - ap.y) * cs.height;
                        _finalPositionX = locationRight + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_CENTER:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationRight = relativeWidget->getRightBoundary();
                        _finalPositionX = locationRight + ap.x * cs.width;

                        _finalPositionY = relativeWidget->getBottomBoundary() + rbs.height * 0.5f + ap.y * cs.height - cs.height * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_BOTTOMALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        _finalPositionY = locationBottom + ap.y * cs.height;
                        _finalPositionX = locationRight + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_LEFTALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationLeft = relativeWidget->getLeftBoundary();
                        _finalPositionY = locationBottom - (1.0f - ap.y) * cs.height;
                        _finalPositionX = locationLeft + ap.x * cs.width;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_CENTER:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        MATH::Sizef rbs = relativeWidget->getContentSize();
                        float locationBottom = relativeWidget->getBottomBoundary();

                        _finalPositionY = locationBottom - (1.0f - ap.y) * cs.height;
                        _finalPositionX = relativeWidget->getLeftBoundary() + rbs.width * 0.5f + ap.x * cs.width - cs.width * 0.5f;
                    }
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_RIGHTALIGN:
                    if (relativeWidget)
                    {
                        if (_relativeWidgetLP && !_relativeWidgetLP->_put)
                        {
                            return false;
                        }
                        float locationBottom = relativeWidget->getBottomBoundary();
                        float locationRight = relativeWidget->getRightBoundary();
                        _finalPositionY = locationBottom - (1.0f - ap.y) * cs.height;
                        _finalPositionX = locationRight - (1.0f - ap.x) * cs.width;
                    }
                    break;
                default:
                    break;
            }
            return true;
        }

        void RelativeLayoutManager::caculateFinalPositionWithRelativeAlign()
        {
            RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(_widget->getLayoutParameter());

            Margin mg = layoutParameter->getMargin();


            RelativeLayoutParameter::RelativeAlign align = layoutParameter->getAlign();

            //handle margin
            switch (align)
            {
                case RelativeLayoutParameter::RelativeAlign::NONE:
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_LEFT:
                    _finalPositionX += mg.left;
                    _finalPositionY -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_CENTER_HORIZONTAL:
                    _finalPositionY -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_TOP_RIGHT:
                    _finalPositionX -= mg.right;
                    _finalPositionY -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_CENTER_VERTICAL:
                    _finalPositionX += mg.left;
                    break;
                case RelativeLayoutParameter::RelativeAlign::CENTER_IN_PARENT:
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_CENTER_VERTICAL:
                    _finalPositionX -= mg.right;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_LEFT_BOTTOM:
                    _finalPositionX += mg.left;
                    _finalPositionY += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_BOTTOM_CENTER_HORIZONTAL:
                    _finalPositionY += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::PARENT_RIGHT_BOTTOM:
                    _finalPositionX -= mg.right;
                    _finalPositionY += mg.bottom;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_LEFTALIGN:
                    _finalPositionY += mg.bottom;
                    _finalPositionX += mg.left;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_RIGHTALIGN:
                    _finalPositionY += mg.bottom;
                    _finalPositionX -= mg.right;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_ABOVE_CENTER:
                    _finalPositionY += mg.bottom;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_TOPALIGN:
                    _finalPositionX -= mg.right;
                    _finalPositionY -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_BOTTOMALIGN:
                    _finalPositionX -= mg.right;
                    _finalPositionY += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_LEFT_OF_CENTER:
                    _finalPositionX -= mg.right;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_TOPALIGN:
                    _finalPositionX += mg.left;
                    _finalPositionY -= mg.top;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_BOTTOMALIGN:
                    _finalPositionX += mg.left;
                    _finalPositionY += mg.bottom;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_RIGHT_OF_CENTER:
                    _finalPositionX += mg.left;
                    break;

                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_LEFTALIGN:
                    _finalPositionY -= mg.top;
                    _finalPositionX += mg.left;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_RIGHTALIGN:
                    _finalPositionY -= mg.top;
                    _finalPositionX -= mg.right;
                    break;
                case RelativeLayoutParameter::RelativeAlign::LOCATION_BELOW_CENTER:
                    _finalPositionY -= mg.top;
                    break;
                default:
                    break;
            }
        }

        void RelativeLayoutManager::doLayout(LayoutProtocol *layout)
        {
            _widgetChildren = this->getAllWidgets(layout);

            while (_unlayoutChildCount > 0)
            {
                for (auto& subWidget : _widgetChildren)
                {
                    _widget = static_cast<Widget*>(subWidget);

                    RelativeLayoutParameter* layoutParameter = dynamic_cast<RelativeLayoutParameter*>(_widget->getLayoutParameter());

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


                        _widget->setPosition(MATH::Vector2f(_finalPositionX, _finalPositionY));

                        layoutParameter->_put = true;
                    }
                }
                _unlayoutChildCount--;

            }
            _widgetChildren.clear();
        }

        LayoutComponent::LayoutComponent()
            :_horizontalEdge(HorizontalEdge::None)
            , _verticalEdge(VerticalEdge::None)
            , _leftMargin(0)
            , _rightMargin(0)
            , _bottomMargin(0)
            , _topMargin(0)
            , _usingPositionPercentX(false)
            , _positionPercentX(0)
            , _usingPositionPercentY(false)
            , _positionPercentY(0)
            , _usingStretchWidth(false)
            , _usingStretchHeight(false)
            , _percentWidth(0)
            , _usingPercentWidth(false)
            , _percentHeight(0)
            , _usingPercentHeight(false)
            , _actived(true)
            , _isPercentOnly(false)
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

            _leftMargin = ownerPoint.x - ownerAnchor.x * ownerSize.width;
            _rightMargin = parentSize.width - (ownerPoint.x + (1 - ownerAnchor.x) * ownerSize.width);
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

            _bottomMargin = ownerPoint.y - ownerAnchor.y * ownerSize.height;
            _topMargin = parentSize.height - (ownerPoint.y + (1 - ownerAnchor.y) * ownerSize.height);
        }

        //OldVersion
        void LayoutComponent::setUsingPercentContentSize(bool isUsed)
        {
            _usingPercentWidth = _usingPercentHeight = isUsed;
        }

        bool LayoutComponent::getUsingPercentContentSize()const
        {
            return _usingPercentWidth && _usingPercentHeight;
        }

        void LayoutComponent::setPercentContentSize(const MATH::Vector2f &percent)
        {
            this->setPercentWidth(percent.x);
            this->setPercentHeight(percent.y);
        }

        MATH::Vector2f LayoutComponent::getPercentContentSize()const
        {
            MATH::Vector2f vec2 = MATH::Vector2f(_percentWidth,_percentHeight);
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
                    _positionPercentX = ownerPoint.x / parentSize.width;
                else
                {
                    _positionPercentX = 0;
                    if (_usingPositionPercentX)
                        ownerPoint.x = 0;
                }

                if (parentSize.height != 0)
                    _positionPercentY = ownerPoint.y / parentSize.height;
                else
                {
                    _positionPercentY = 0;
                    if (_usingPositionPercentY)
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
            return _usingPositionPercentX;
        }
        void LayoutComponent::setPositionPercentXEnabled(bool isUsed)
        {
            _usingPositionPercentX = isUsed;
            if (_usingPositionPercentX)
            {
                _horizontalEdge = HorizontalEdge::None;
            }
        }

        float LayoutComponent::getPositionPercentX()const
        {
            return _positionPercentX;
        }

        void LayoutComponent::setPositionPercentX(float percentMargin)
        {
            _positionPercentX = percentMargin;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                owner_->setPositionX(parent->getContentSize().width * _positionPercentX);
                this->refreshHorizontalMargin();
            }
        }

        bool LayoutComponent::isPositionPercentYEnabled()const
        {
            return _usingPositionPercentY;
        }
        void LayoutComponent::setPositionPercentYEnabled(bool isUsed)
        {
            _usingPositionPercentY = isUsed;
            if (_usingPositionPercentY)
            {
                _verticalEdge = VerticalEdge::None;
            }
        }

        float LayoutComponent::getPositionPercentY()const
        {
            return _positionPercentY;
        }
        void LayoutComponent::setPositionPercentY(float percentMargin)
        {
            _positionPercentY = percentMargin;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                owner_->setPositionY(parent->getContentSize().height * _positionPercentY);
                this->refreshVerticalMargin();
            }
        }

        LayoutComponent::HorizontalEdge LayoutComponent::getHorizontalEdge()const
        {
            return _horizontalEdge;
        }
        void LayoutComponent::setHorizontalEdge(HorizontalEdge hEage)
        {
            _horizontalEdge = hEage;
            if (_horizontalEdge != HorizontalEdge::None)
            {
                _usingPositionPercentX = false;
            }

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Vector2f ownerPoint = owner_->getPosition();
                const MATH::Sizef& parentSize = parent->getContentSize();
                if (parentSize.width != 0)
                    _positionPercentX = ownerPoint.x / parentSize.width;
                else
                {
                    _positionPercentX = 0;
                    ownerPoint.x = 0;
                    if (_usingPositionPercentX)
                        owner_->setPosition(ownerPoint);
                }

                this->refreshHorizontalMargin();
            }
        }

        LayoutComponent::VerticalEdge LayoutComponent::getVerticalEdge()const
        {
            return _verticalEdge;
        }
        void LayoutComponent::setVerticalEdge(VerticalEdge vEage)
        {
            _verticalEdge = vEage;
            if (_verticalEdge != VerticalEdge::None)
            {
                _usingPositionPercentY = false;
            }

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Vector2f ownerPoint = owner_->getPosition();
                const MATH::Sizef& parentSize = parent->getContentSize();
                if (parentSize.height != 0)
                    _positionPercentY = ownerPoint.y / parentSize.height;
                else
                {
                    _positionPercentY = 0;
                    ownerPoint.y = 0;
                    if (_usingPositionPercentY)
                        owner_->setPosition(ownerPoint);
                }

                this->refreshVerticalMargin();
            }
        }

        float LayoutComponent::getLeftMargin()const
        {
            return _leftMargin;
        }
        void LayoutComponent::setLeftMargin(float margin)
        {
            _leftMargin = margin;
        }

        float LayoutComponent::getRightMargin()const
        {
            return _rightMargin;
        }
        void LayoutComponent::setRightMargin(float margin)
        {
            _rightMargin = margin;
        }

        float LayoutComponent::getTopMargin()const
        {
            return _topMargin;
        }
        void LayoutComponent::setTopMargin(float margin)
        {
            _topMargin = margin;
        }

        float LayoutComponent::getBottomMargin()const
        {
            return _bottomMargin;
        }
        void LayoutComponent::setBottomMargin(float margin)
        {
            _bottomMargin = margin;
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
                    _percentWidth = ownerSize.width / parentSize.width;
                else
                {
                    _percentWidth = 0;
                    if (_usingPercentWidth)
                        ownerSize.width = 0;
                }

                if (parentSize.height != 0)
                    _percentHeight = ownerSize.height / parentSize.height;
                else
                {
                    _percentHeight = 0;
                    if (_usingPercentHeight)
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
            return _usingPercentWidth;
        }
        void LayoutComponent::setPercentWidthEnabled(bool isUsed)
        {
            _usingPercentWidth = isUsed;
            if (_usingPercentWidth)
            {
                _usingStretchWidth = false;
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
                    _percentWidth = ownerSize.width / parentSize.width;
                else
                {
                    _percentWidth = 0;
                    if (_usingPercentWidth)
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
            return _percentWidth;
        }
        void LayoutComponent::setPercentWidth(float percentWidth)
        {
            _percentWidth = percentWidth;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Sizef ownerSize = owner_->getContentSize();
                ownerSize.width = parent->getContentSize().width * _percentWidth;
                owner_->setContentSize(ownerSize);

                this->refreshHorizontalMargin();
            }
        }

        bool LayoutComponent::isPercentHeightEnabled()const
        {
            return _usingPercentHeight;
        }
        void LayoutComponent::setPercentHeightEnabled(bool isUsed)
        {
            _usingPercentHeight = isUsed;
            if (_usingPercentHeight)
            {
                _usingStretchHeight = false;
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
                    _percentHeight = ownerSize.height / parentSize.height;
                else
                {
                    _percentHeight = 0;
                    if (_usingPercentHeight)
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
            return _percentHeight;
        }
        void LayoutComponent::setPercentHeight(float percentHeight)
        {
            _percentHeight = percentHeight;

            Node* parent = this->getOwnerParent();
            if (parent != nullptr)
            {
                MATH::Sizef ownerSize = owner_->getContentSize();
                ownerSize.height = parent->getContentSize().height * _percentHeight;
                owner_->setContentSize(ownerSize);

                this->refreshVerticalMargin();
            }
        }

        bool LayoutComponent::isStretchWidthEnabled()const
        {
            return _usingStretchWidth;
        }
        void LayoutComponent::setStretchWidthEnabled(bool isUsed)
        {
            _usingStretchWidth = isUsed;
            if (_usingStretchWidth)
            {
                _usingPercentWidth = false;
            }
        }

        bool LayoutComponent::isStretchHeightEnabled()const
        {
            return _usingStretchHeight;
        }
        void LayoutComponent::setStretchHeightEnabled(bool isUsed)
        {
            _usingStretchHeight = isUsed;
            if (_usingStretchHeight)
            {
                _usingPercentHeight = false;
            }
        }

        void LayoutComponent::refreshLayout()
        {
            if (!_actived)
                return;

            Node* parent = this->getOwnerParent();
            if (parent == nullptr)
                return;

            const MATH::Sizef& parentSize = parent->getContentSize();
            const MATH::Vector2f& ownerAnchor = owner_->getAnchorPoint();
            MATH::Sizef ownerSize = owner_->getContentSize();
            MATH::Vector2f ownerPosition = owner_->getPosition();

            switch (this->_horizontalEdge)
            {
            case HorizontalEdge::None:
                if (_usingStretchWidth && !_isPercentOnly)
                {
                    ownerSize.width = parentSize.width * _percentWidth;
                    ownerPosition.x = _leftMargin + ownerAnchor.x * ownerSize.width;
                }
                else
                {
                    if (_usingPositionPercentX)
                        ownerPosition.x = parentSize.width * _positionPercentX;
                    if (_usingPercentWidth)
                        ownerSize.width = parentSize.width * _percentWidth;
                }
                break;
            case HorizontalEdge::Left:
                if (_isPercentOnly)
                    break;
                if (_usingPercentWidth || _usingStretchWidth)
                    ownerSize.width = parentSize.width * _percentWidth;
                ownerPosition.x = _leftMargin + ownerAnchor.x * ownerSize.width;
                break;
            case HorizontalEdge::Right:
                if (_isPercentOnly)
                    break;
                if (_usingPercentWidth || _usingStretchWidth)
                    ownerSize.width = parentSize.width * _percentWidth;
                ownerPosition.x = parentSize.width - (_rightMargin + (1 - ownerAnchor.x) * ownerSize.width);
                break;
            case HorizontalEdge::Center:
                if (_isPercentOnly)
                    break;
                if (_usingStretchWidth)
                {
                    ownerSize.width = parentSize.width - _leftMargin - _rightMargin;
                    if (ownerSize.width < 0)
                        ownerSize.width = 0;
                    ownerPosition.x = _leftMargin + ownerAnchor.x * ownerSize.width;
                }
                else
                {
                    if (_usingPercentWidth)
                        ownerSize.width = parentSize.width * _percentWidth;
                    ownerPosition.x = parentSize.width * _positionPercentX;
                }
                break;
            default:
                break;
            }

            switch (this->_verticalEdge)
            {
            case VerticalEdge::None:
                if (_usingStretchHeight && !_isPercentOnly)
                {
                    ownerSize.height = parentSize.height * _percentHeight;
                    ownerPosition.y = _bottomMargin + ownerAnchor.y * ownerSize.height;
                }
                else
                {
                    if (_usingPositionPercentY)
                        ownerPosition.y = parentSize.height * _positionPercentY;
                    if (_usingPercentHeight)
                        ownerSize.height = parentSize.height * _percentHeight;
                }
                break;
            case VerticalEdge::Bottom:
                if (_isPercentOnly)
                    break;
                if (_usingPercentHeight || _usingStretchHeight)
                    ownerSize.height = parentSize.height * _percentHeight;
                ownerPosition.y = _bottomMargin + ownerAnchor.y * ownerSize.height;
                break;
            case VerticalEdge::Top:
                if (_isPercentOnly)
                    break;
                if (_usingPercentHeight || _usingStretchHeight)
                    ownerSize.height = parentSize.height * _percentHeight;
                ownerPosition.y = parentSize.height - (_topMargin + (1 - ownerAnchor.y) * ownerSize.height);
                break;
            case VerticalEdge::Center:
                if (_isPercentOnly)
                    break;
                if (_usingStretchHeight)
                {
                    ownerSize.height = parentSize.height - _topMargin - _bottomMargin;
                    if (ownerSize.height < 0)
                        ownerSize.height = 0;
                    ownerPosition.y = _bottomMargin + ownerAnchor.y * ownerSize.height;
                }
                else
                {
                    if (_usingPercentHeight)
                        ownerSize.height = parentSize.height * _percentHeight;
                    ownerPosition.y = parentSize.height* _positionPercentY;
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
            _actived = enable;
        }

        void LayoutComponent::setPercentOnlyEnabled(bool enable)
        {
            _isPercentOnly = enable;
        }
    }
}
