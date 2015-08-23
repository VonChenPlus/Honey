#include "GRAPH/UI/BASE/UILayout.h"
#include "GRAPH/BASE/DrawNode.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "GRAPH/RENDERER/RenderState.h"
#include "GRAPH/RENDERER/GLProgramCache.h"
#include "MATH/AffineTransform.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "GRAPH/UI/BASE/UIScale9Sprite.h"
#include "GRAPH/BASE/Layer.h"
#include "GRAPH/UI/BASE/UIHelper.h"

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
        _currentStencilEnabled(GL_FALSE),
        _currentStencilWriteMask(~0),
        _currentStencilFunc(GL_ALWAYS),
        _currentStencilRef(0),
        _currentStencilValueMask(~0),
        _currentStencilFail(GL_KEEP),
        _currentStencilPassDepthFail(GL_KEEP),
        _currentStencilPassDepthPass(GL_KEEP),
        _currentDepthWriteMask(GL_TRUE),
        _currentAlphaTestEnabled(GL_FALSE),
        _currentAlphaTestFunc(GL_ALWAYS),
        _currentAlphaTestRef(1),
        _doLayoutDirty(true),
        _isInterceptTouch(false),
        _loopFocus(false),
        _passFocusToChild(true),
        _isFocusPassing(false)
        {
            //no-op
        }

        Layout::~Layout()
        {
            SAFE_RELEASE(_clippingStencil);
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
                onPassFocusToChild = CC_CALLBACK_2(Layout::findNearestChildWidgetIndex, this);
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
            if (!_visible)
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
            if(!_visible)
                return;

            uint32_t flags = processParentFlags(parentTransform, parentFlags);

            // IMPORTANT:
            // To ease the migration to v3.0, we still support the MATH::Matrix4 stack,
            // but it is deprecated and your code should not rely on it
            Director* director = Director::getInstance();
            director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);
            //Add group command

            _groupCommand.init(_globalZOrder);
            renderer->addCommand(&_groupCommand);

            renderer->pushGroup(_groupCommand.getRenderQueueID());

            _beforeVisitCmdStencil.init(_globalZOrder);
            _beforeVisitCmdStencil.func = CC_CALLBACK_0(Layout::onBeforeVisitStencil, this);
            renderer->addCommand(&_beforeVisitCmdStencil);

            _clippingStencil->visit(renderer, _modelViewTransform, flags);

            _afterDrawStencilCmd.init(_globalZOrder);
            _afterDrawStencilCmd.func = CC_CALLBACK_0(Layout::onAfterDrawStencil, this);
            renderer->addCommand(&_afterDrawStencilCmd);

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

            for( ; j < _protectedChildren.size(); j++ )
            {
                auto node = _protectedChildren.at(j);

                if ( node && node->getLocalZOrder() < 0 )
                    node->visit(renderer, _modelViewTransform, flags);
                else
                    break;
            }

            //
            // draw self
            //
            this->draw(renderer, _modelViewTransform, flags);

            //
            // draw children and protectedChildren zOrder >= 0
            //
            for(auto it=_protectedChildren.cbegin()+j; it != _protectedChildren.cend(); ++it)
                (*it)->visit(renderer, _modelViewTransform, flags);

            for(auto it=_children.cbegin()+i; it != _children.cend(); ++it)
                (*it)->visit(renderer, _modelViewTransform, flags);


            _afterVisitCmdStencil.init(_globalZOrder);
            _afterVisitCmdStencil.func = CC_CALLBACK_0(Layout::onAfterVisitStencil, this);
            renderer->addCommand(&_afterVisitCmdStencil);

            renderer->popGroup();

            director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }

        void Layout::onBeforeVisitStencil()
        {
            s_layer++;
            GLint mask_layer = 0x1 << s_layer;
            GLint mask_layer_l = mask_layer - 1;
            _mask_layer_le = mask_layer | mask_layer_l;
            _currentStencilEnabled = glIsEnabled(GL_STENCIL_TEST);
            glGetIntegerv(GL_STENCIL_WRITEMASK, (GLint *)&_currentStencilWriteMask);
            glGetIntegerv(GL_STENCIL_FUNC, (GLint *)&_currentStencilFunc);
            glGetIntegerv(GL_STENCIL_REF, &_currentStencilRef);
            glGetIntegerv(GL_STENCIL_VALUE_MASK, (GLint *)&_currentStencilValueMask);
            glGetIntegerv(GL_STENCIL_FAIL, (GLint *)&_currentStencilFail);
            glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, (GLint *)&_currentStencilPassDepthFail);
            glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, (GLint *)&_currentStencilPassDepthPass);

            glEnable(GL_STENCIL_TEST);

            glStencilMask(mask_layer);

            glGetBooleanv(GL_DEPTH_WRITEMASK, &_currentDepthWriteMask);

            glDepthMask(GL_FALSE);
            RenderState::StateBlock::_defaultState->setDepthWrite(false);

            glStencilFunc(GL_NEVER, mask_layer, mask_layer);
            glStencilOp(GL_ZERO, GL_KEEP, GL_KEEP);


            this->drawFullScreenQuadClearStencil();

            glStencilFunc(GL_NEVER, mask_layer, mask_layer);
        //    RenderState::StateBlock::_defaultState->setStencilFunction(
        //                                                               RenderState::STENCIL_NEVER,
        //                                                               mask_layer,
        //                                                               mask_layer);

            glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
        //    RenderState::StateBlock::_defaultState->setStencilOperation(
        //                                                                RenderState::STENCIL_OP_REPLACE,
        //                                                                RenderState::STENCIL_OP_KEEP,
        //                                                                RenderState::STENCIL_OP_KEEP);
        }

        void Layout::drawFullScreenQuadClearStencil()
        {
            Director* director = Director::getInstance();

            director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            director->loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

            director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
            director->loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

            MATH::Vector2f vertices[] =
            {
                MATH::Vector2f(-1, -1),
                MATH::Vector2f(1, -1),
                MATH::Vector2f(1, 1),
                MATH::Vector2f(-1, 1)
            };

            auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_U_COLOR);

            int colorLocation = glProgram->getUniformLocation("u_color");

            Color4F color(1, 1, 1, 1);

            glProgram->use();
            glProgram->setUniformsForBuiltins();
            glProgram->setUniformLocationWith4fv(colorLocation, (GLfloat*) &color.red, 1);

            enableVertexAttribs( VERTEX_ATTRIB_FLAG_POSITION );

            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, vertices);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);

            director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
            director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        }

        void Layout::onAfterDrawStencil()
        {
            glDepthMask(_currentDepthWriteMask);
            RenderState::StateBlock::_defaultState->setDepthWrite(_currentDepthWriteMask);

            glStencilFunc(GL_EQUAL, _mask_layer_le, _mask_layer_le);

            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }


        void Layout::onAfterVisitStencil()
        {
            glStencilFunc(_currentStencilFunc, _currentStencilRef, _currentStencilValueMask);

            glStencilOp(_currentStencilFail, _currentStencilPassDepthFail, _currentStencilPassDepthPass);

            glStencilMask(_currentStencilWriteMask);
            if (!_currentStencilEnabled)
            {
                glDisable(GL_STENCIL_TEST);
            }
            s_layer--;
        }

        void Layout::onBeforeVisitScissor()
        {
            MATH::Rectf clippingRect = getClippingRect();
            glEnable(GL_SCISSOR_TEST);
            auto glview = Director::getInstance()->getOpenGLView();
            glview->setScissorInPoints(clippingRect.origin.x, clippingRect.origin.y, clippingRect.size.width, clippingRect.size.height);
        }

        void Layout::onAfterVisitScissor()
        {
            glDisable(GL_SCISSOR_TEST);
        }

        void Layout::scissorClippingVisit(Renderer *renderer, const MATH::Matrix4& parentTransform, uint32_t parentFlags)
        {
            _beforeVisitCmdScissor.init(_globalZOrder);
            _beforeVisitCmdScissor.func = CC_CALLBACK_0(Layout::onBeforeVisitScissor, this);
            renderer->addCommand(&_beforeVisitCmdScissor);

            ProtectedNode::visit(renderer, parentTransform, parentFlags);

            _afterVisitCmdScissor.init(_globalZOrder);
            _afterVisitCmdScissor.func = CC_CALLBACK_0(Layout::onAfterVisitScissor, this);
            renderer->addCommand(&_afterVisitCmdScissor);
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
                        if (_running)
                        {
                            _clippingStencil->onEnter();
                        }
                        _clippingStencil->retain();
                        setStencilClippingSize(_contentSize);
                    }
                    else
                    {
                        if (_running)
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

        void Layout::setStencilClippingSize(const MATH::Sizef &size)
        {
            if (_clippingEnabled && _clippingType == ClippingType::STENCIL)
            {
                MATH::Vector2f rect[4];
                // rect[0].setZero(); Zero default
                rect[1].set(_contentSize.width, 0.0f);
                rect[2].set(_contentSize.width, _contentSize.height);
                rect[3].set(0.0f, _contentSize.height);
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
                float scissorWidth = _contentSize.width*t.a;
                float scissorHeight = _contentSize.height*t.d;
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
                    float finalX = worldPos.x - (scissorWidth * _anchorPoint.x);
                    float finalY = worldPos.y - (scissorHeight * _anchorPoint.y);
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
                    _clippingRect.origin.x = worldPos.x - (scissorWidth * _anchorPoint.x);
                    _clippingRect.origin.y = worldPos.y - (scissorHeight * _anchorPoint.y);
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
            setStencilClippingSize(_contentSize);
            _doLayoutDirty = true;
            _clippingRectDirty = true;
            if (_backGroundImage)
            {
                _backGroundImage->setPosition(_contentSize.width/2.0f, _contentSize.height/2.0f);
                if (_backGroundScale9Enabled && _backGroundImage)
                {
                    _backGroundImage->setPreferredSize(_contentSize);
                }
            }
            if (_colorRender)
            {
                _colorRender->setContentSize(_contentSize);
            }
            if (_gradientRender)
            {
                _gradientRender->setContentSize(_contentSize);
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
                _backGroundImage->setPreferredSize(_contentSize);
            }

            _backGroundImageTextureSize = _backGroundImage->getContentSize();
            _backGroundImage->setPosition(_contentSize.width/2.0f, _contentSize.height/2.0f);
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

            _backGroundImage->setPosition(_contentSize.width/2.0f, _contentSize.height/2.0f);
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
                    _colorRender->setContentSize(_contentSize);
                    _colorRender->setOpacity(_cOpacity);
                    _colorRender->setColor(_cColor);
                    addProtectedChild(_colorRender, BCAKGROUNDCOLORRENDERER_Z, -1);
                    break;
                case BackGroundColorType::GRADIENT:
                    _gradientRender = LayerGradient::create();
                    _gradientRender->setContentSize(_contentSize);
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

        void Layout::setBackGroundColorOpacity(GLubyte opacity)
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

        GLubyte Layout::getBackGroundColorOpacity()const
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

        void Layout::setBackGroundImageOpacity(GLubyte opacity)
        {
            _backGroundImageOpacity = opacity;
            updateBackGroundImageOpacity();
        }

        const Color3B& Layout::getBackGroundImageColor()const
        {
            return _backGroundImageColor;
        }

        GLubyte Layout::getBackGroundImageOpacity()const
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

            for (auto& child : _children)
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

            for (Node* node : _children)
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

            for (Node* node : _children)
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
            ssize_t index = 0;
            ssize_t count = this->getChildren().size();
            while (index < count)
            {
                Widget* w =  dynamic_cast<Widget*>(_children.at(index));
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
            int index = 0;
            ssize_t count = this->getChildren().size();

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
            int index = 0;
            ssize_t count = this->getChildren().size();

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



        Widget* Layout::findFocusEnabledChildWidgetByIndex(ssize_t index)
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
            for(Node *node : _children)
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
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findNearestChildWidgetIndex, this);
                }
                else
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findFarthestChildWidgetIndex, this);
                }
            }
            else if(dir == FocusDirection::RIGHT)
            {
                if (previousWidgetPosition.x > widgetPosition.x)
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findFarthestChildWidgetIndex, this);
                }
                else
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findNearestChildWidgetIndex, this);
                }
            }
            else if(dir == FocusDirection::DOWN)
            {
                if (previousWidgetPosition.y > widgetPosition.y)
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findNearestChildWidgetIndex, this);
                }
                else
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findFarthestChildWidgetIndex, this);
                }
            }
            else if(dir == FocusDirection::UP)
            {
                if (previousWidgetPosition.y < widgetPosition.y)
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findNearestChildWidgetIndex, this);
                }
                else
                {
                    onPassFocusToChild = CC_CALLBACK_2(Layout::findFarthestChildWidgetIndex, this);
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
            for(Node* node : _children)
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

        Widget* Layout::getChildWidgetByIndex(ssize_t index)const
        {
            ssize_t size = _children.size();
            int count = 0;
            ssize_t oldIndex = index;
            Widget *widget = nullptr;
            while (index < size)
            {
                Widget* firstChild = dynamic_cast<Widget*>(_children.at(index));
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
                int begin = 0;
                while (begin < oldIndex)
                {
                    Widget* firstChild = dynamic_cast<Widget*>(_children.at(begin));
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
            ssize_t previousWidgetPos = _children.getIndex(current);
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
                        previousWidgetPos = _children.size()-1;
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
                            return _focusedWidget;
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
                            return _focusedWidget;
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
            ssize_t previousWidgetPos = _children.getIndex(current);
            previousWidgetPos = previousWidgetPos + 1;
            if (previousWidgetPos < _children.size())
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
                            return _focusedWidget;
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
                            return _focusedWidget;
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
            ssize_t index = container.getIndex(widget);
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
                _name = __LAYOUT_COMPONENT_NAME;
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
                Node* parent = _owner->getParent();
                return parent;
            }
            void LayoutComponent::refreshHorizontalMargin()
            {
                Node* parent = this->getOwnerParent();
                if (parent == nullptr)
                    return;

                const MATH::Vector2f& ownerPoint = _owner->getPosition();
                const MATH::Vector2f& ownerAnchor = _owner->getAnchorPoint();
                const MATH::Sizef& ownerSize = _owner->getContentSize();
                const MATH::Sizef& parentSize = parent->getContentSize();

                _leftMargin = ownerPoint.x - ownerAnchor.x * ownerSize.width;
                _rightMargin = parentSize.width - (ownerPoint.x + (1 - ownerAnchor.x) * ownerSize.width);
            }
            void LayoutComponent::refreshVerticalMargin()
            {
                Node* parent = this->getOwnerParent();
                if (parent == nullptr)
                    return;

                const MATH::Vector2f& ownerPoint = _owner->getPosition();
                const MATH::Vector2f& ownerAnchor = _owner->getAnchorPoint();
                const MATH::Sizef& ownerSize = _owner->getContentSize();
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
                return _owner->getAnchorPoint();
            }

            void LayoutComponent::setAnchorPosition(const MATH::Vector2f& point)
            {
                MATH::Rectf oldRect = _owner->getBoundingBox();
                _owner->setAnchorPoint(point);
                MATH::Rectf newRect = _owner->getBoundingBox();
                float offSetX = oldRect.origin.x - newRect.origin.x;
                float offSetY = oldRect.origin.y - newRect.origin.y;

                MATH::Vector2f ownerPosition = _owner->getPosition();
                ownerPosition.x += offSetX;
                ownerPosition.y += offSetY;

                this->setPosition(ownerPosition);
            }

            const MATH::Vector2f& LayoutComponent::getPosition()const
            {
                return _owner->getPosition();
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

                    _owner->setPosition(ownerPoint);

                    this->refreshHorizontalMargin();
                    this->refreshVerticalMargin();
                }
                else
                    _owner->setPosition(position);
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
                    _owner->setPositionX(parent->getContentSize().width * _positionPercentX);
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
                    _owner->setPositionY(parent->getContentSize().height * _positionPercentY);
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
                    MATH::Vector2f ownerPoint = _owner->getPosition();
                    const MATH::Sizef& parentSize = parent->getContentSize();
                    if (parentSize.width != 0)
                        _positionPercentX = ownerPoint.x / parentSize.width;
                    else
                    {
                        _positionPercentX = 0;
                        ownerPoint.x = 0;
                        if (_usingPositionPercentX)
                            _owner->setPosition(ownerPoint);
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
                    MATH::Vector2f ownerPoint = _owner->getPosition();
                    const MATH::Sizef& parentSize = parent->getContentSize();
                    if (parentSize.height != 0)
                        _positionPercentY = ownerPoint.y / parentSize.height;
                    else
                    {
                        _positionPercentY = 0;
                        ownerPoint.y = 0;
                        if (_usingPositionPercentY)
                            _owner->setPosition(ownerPoint);
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

                    _owner->setContentSize(ownerSize);

                    this->refreshHorizontalMargin();
                    this->refreshVerticalMargin();
                }
                else
                    _owner->setContentSize(size);
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
                return _owner->getContentSize().width;
            }
            void LayoutComponent::setSizeWidth(float width)
            {
                MATH::Sizef ownerSize = _owner->getContentSize();
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
                    _owner->setContentSize(ownerSize);
                    this->refreshHorizontalMargin();
                }
                else
                    _owner->setContentSize(ownerSize);
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
                    MATH::Sizef ownerSize = _owner->getContentSize();
                    ownerSize.width = parent->getContentSize().width * _percentWidth;
                    _owner->setContentSize(ownerSize);

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
                return _owner->getContentSize().height;
            }
            void LayoutComponent::setSizeHeight(float height)
            {
                MATH::Sizef ownerSize = _owner->getContentSize();
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
                    _owner->setContentSize(ownerSize);
                    this->refreshVerticalMargin();
                }
                else
                    _owner->setContentSize(ownerSize);
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
                    MATH::Sizef ownerSize = _owner->getContentSize();
                    ownerSize.height = parent->getContentSize().height * _percentHeight;
                    _owner->setContentSize(ownerSize);

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
                const MATH::Vector2f& ownerAnchor = _owner->getAnchorPoint();
                MATH::Sizef ownerSize = _owner->getContentSize();
                MATH::Vector2f ownerPosition = _owner->getPosition();

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

                _owner->setPosition(ownerPosition);
                _owner->setContentSize(ownerSize);

                Helper::doLayout(_owner);
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
