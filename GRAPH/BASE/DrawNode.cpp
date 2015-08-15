#include "GRAPH/BASE/DrawNode.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/BASE/Configuration.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/RENDERER/GLProgramCache.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "GRAPH/BASE/Director.h"
#include <vector>

namespace GRAPH
{
    PointArray* PointArray::create(ssize_t capacity)
    {
        PointArray* pointArray = new (std::nothrow) PointArray();
        if (pointArray)
        {
            if (pointArray->initWithCapacity(capacity))
            {
                pointArray->autorelease();
            }
            else
            {
                delete pointArray;
                pointArray = nullptr;
            }
        }

        return pointArray;
    }


    bool PointArray::initWithCapacity(ssize_t capacity)
    {
        _controlPoints = new std::vector<MATH::Vector2f*>();

        return true;
    }

    PointArray* PointArray::clone() const
    {
        std::vector<MATH::Vector2f*> *newArray = new std::vector<MATH::Vector2f*>();
        std::vector<MATH::Vector2f*>::iterator iter;
        for (iter = _controlPoints->begin(); iter != _controlPoints->end(); ++iter)
        {
            newArray->push_back(new MATH::Vector2f((*iter)->x, (*iter)->y));
        }

        PointArray *points = new (std::nothrow) PointArray();
        points->initWithCapacity(10);
        points->setControlPoints(newArray);

        points->autorelease();
        return points;
    }

    PointArray::~PointArray()
    {
        std::vector<MATH::Vector2f*>::iterator iter;
        for (iter = _controlPoints->begin(); iter != _controlPoints->end(); ++iter)
        {
            delete *iter;
        }
        delete _controlPoints;
    }

    PointArray::PointArray() :_controlPoints(nullptr){}

    const std::vector<MATH::Vector2f*>* PointArray::getControlPoints() const
    {
        return _controlPoints;
    }

    void PointArray::setControlPoints(std::vector<MATH::Vector2f*> *controlPoints)
    {
        // delete old points
        std::vector<MATH::Vector2f*>::iterator iter;
        for (iter = _controlPoints->begin(); iter != _controlPoints->end(); ++iter)
        {
            delete *iter;
        }
        delete _controlPoints;

        _controlPoints = controlPoints;
    }

    void PointArray::addControlPoint(MATH::Vector2f controlPoint)
    {
        _controlPoints->push_back(new MATH::Vector2f(controlPoint.x, controlPoint.y));
    }

    void PointArray::insertControlPoint(MATH::Vector2f &controlPoint, ssize_t index)
    {
        MATH::Vector2f *temp = new (std::nothrow) MATH::Vector2f(controlPoint.x, controlPoint.y);
        _controlPoints->insert(_controlPoints->begin() + index, temp);
    }

    MATH::Vector2f PointArray::getControlPointAtIndex(ssize_t index)
    {
        index = MATH::MATH_MIN(static_cast<ssize_t>(_controlPoints->size())-1, MATH::MATH_MAX(index, ssize_t(0)));
        return *(_controlPoints->at(index));
    }

    void PointArray::replaceControlPoint(MATH::Vector2f &controlPoint, ssize_t index)
    {
        MATH::Vector2f *temp = _controlPoints->at(index);
        temp->x = controlPoint.x;
        temp->y = controlPoint.y;
    }

    void PointArray::removeControlPointAtIndex(ssize_t index)
    {
        std::vector<MATH::Vector2f*>::iterator iter = _controlPoints->begin() + index;
        MATH::Vector2f* removedPoint = *iter;
        _controlPoints->erase(iter);
        delete removedPoint;
    }

    ssize_t PointArray::count() const
    {
        return _controlPoints->size();
    }

    PointArray* PointArray::reverse() const
    {
        std::vector<MATH::Vector2f*> *newArray = new std::vector<MATH::Vector2f*>();
        std::vector<MATH::Vector2f*>::reverse_iterator iter;
        MATH::Vector2f *point = nullptr;
        for (iter = _controlPoints->rbegin(); iter != _controlPoints->rend(); ++iter)
        {
            point = *iter;
            newArray->push_back(new MATH::Vector2f(point->x, point->y));
        }
        PointArray *config = PointArray::create(0);
        config->setControlPoints(newArray);

        return config;
    }

    void PointArray::reverseInline()
    {
        size_t l = _controlPoints->size();
        MATH::Vector2f *p1 = nullptr;
        MATH::Vector2f *p2 = nullptr;
        float x, y;
        for (size_t i = 0; i < l/2; ++i)
        {
            p1 = _controlPoints->at(i);
            p2 = _controlPoints->at(l-i-1);

            x = p1->x;
            y = p1->y;

            p1->x = p2->x;
            p1->y = p2->y;

            p2->x = x;
            p2->y = y;
        }
    }

    DrawNode::DrawNode()
    : _vao(0)
    , _vbo(0)
    , _vaoGLPoint(0)
    , _vboGLPoint(0)
    , _vaoGLLine(0)
    , _vboGLLine(0)
    , _bufferCapacity(0)
    , _bufferCount(0)
    , _buffer(nullptr)
    , _bufferCapacityGLPoint(0)
    , _bufferCountGLPoint(0)
    , _bufferGLPoint(nullptr)
    , _bufferCapacityGLLine(0)
    , _bufferCountGLLine(0)
    , _bufferGLLine(nullptr)
    , _dirty(false)
    , _dirtyGLPoint(false)
    , _dirtyGLLine(false)
    {
        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
    }

    DrawNode::~DrawNode()
    {
        free(_buffer);
        _buffer = nullptr;
        free(_bufferGLPoint);
        _bufferGLPoint = nullptr;
        free(_bufferGLLine);
        _bufferGLLine = nullptr;

        glDeleteBuffers(1, &_vbo);
        glDeleteBuffers(1, &_vboGLLine);
        glDeleteBuffers(1, &_vboGLPoint);
        _vbo = 0;
        _vboGLPoint = 0;
        _vboGLLine = 0;

        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(0);
            glDeleteVertexArrays(1, &_vao);
            glDeleteVertexArrays(1, &_vaoGLLine);
            glDeleteVertexArrays(1, &_vaoGLPoint);
            _vao = _vaoGLLine = _vaoGLPoint = 0;
        }
    }

    DrawNode* DrawNode::create()
    {
        DrawNode* ret = new (std::nothrow) DrawNode();
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

    void DrawNode::ensureCapacity(int count)
    {
        if(_bufferCount + count > _bufferCapacity)
        {
            _bufferCapacity += MATH::MATH_MAX(_bufferCapacity, count);
            _buffer = (V2F_C4B_T2F*)realloc(_buffer, _bufferCapacity*sizeof(V2F_C4B_T2F));
        }
    }

    void DrawNode::ensureCapacityGLPoint(int count)
    {
        if(_bufferCountGLPoint + count > _bufferCapacityGLPoint)
        {
            _bufferCapacityGLPoint += MATH::MATH_MAX(_bufferCapacityGLPoint, count);
            _bufferGLPoint = (V2F_C4B_T2F*)realloc(_bufferGLPoint, _bufferCapacityGLPoint*sizeof(V2F_C4B_T2F));
        }
    }

    void DrawNode::ensureCapacityGLLine(int count)
    {
        if(_bufferCountGLLine + count > _bufferCapacityGLLine)
        {
            _bufferCapacityGLLine += MATH::MATH_MAX(_bufferCapacityGLLine, count);
            _bufferGLLine = (V2F_C4B_T2F*)realloc(_bufferGLLine, _bufferCapacityGLLine*sizeof(V2F_C4B_T2F));
        }
    }

    bool DrawNode::init()
    {
        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

        setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR));

        ensureCapacity(512);
        ensureCapacityGLPoint(64);
        ensureCapacityGLLine(256);

        if (Configuration::getInstance()->supportsShareableVAO())
        {
            glGenVertexArrays(1, &_vao);
            bindVAO(_vao);
            glGenBuffers(1, &_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)* _bufferCapacity, _buffer, GL_STREAM_DRAW);
            // vertex
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
            // color
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
            // texcood
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));

            glGenVertexArrays(1, &_vaoGLLine);
            bindVAO(_vaoGLLine);
            glGenBuffers(1, &_vboGLLine);
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);
            // vertex
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
            // color
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
            // texcood
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));

            glGenVertexArrays(1, &_vaoGLPoint);
            bindVAO(_vaoGLPoint);
            glGenBuffers(1, &_vboGLPoint);
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);
            // vertex
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
            // color
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
            // Texture coord as pointsize
            glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));

            bindVAO(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

        }
        else
        {
            glGenBuffers(1, &_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)* _bufferCapacity, _buffer, GL_STREAM_DRAW);

            glGenBuffers(1, &_vboGLLine);
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);

            glGenBuffers(1, &_vboGLPoint);
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        _dirty = true;
        _dirtyGLLine = true;
        _dirtyGLPoint = true;

        return true;
    }

    void DrawNode::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags)
    {
        if(_bufferCount)
        {
            _customCommand.init(_globalZOrder, transform, flags);
            _customCommand.func = CC_CALLBACK_0(DrawNode::onDraw, this, transform, flags);
            renderer->addCommand(&_customCommand);
        }

        if(_bufferCountGLPoint)
        {
            _customCommandGLPoint.init(_globalZOrder, transform, flags);
            _customCommandGLPoint.func = CC_CALLBACK_0(DrawNode::onDrawGLPoint, this, transform, flags);
            renderer->addCommand(&_customCommandGLPoint);
        }

        if(_bufferCountGLLine)
        {
            _customCommandGLLine.init(_globalZOrder, transform, flags);
            _customCommandGLLine.func = CC_CALLBACK_0(DrawNode::onDrawGLLine, this, transform, flags);
            renderer->addCommand(&_customCommandGLLine);
        }
    }

    void DrawNode::onDraw(const MATH::Matrix4 &transform, uint32_t flags)
    {
        auto glProgram = getGLProgram();
        glProgram->use();
        glProgram->setUniformsForBuiltins(transform);

        blendFunc(_blendFunc.src, _blendFunc.dst);

        if (_dirty)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacity, _buffer, GL_STREAM_DRAW);

            _dirty = false;
        }
        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(_vao);
        }
        else
        {
            enableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            // vertex
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
            // color
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
            // texcood
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));
        }

        glDrawArrays(GL_TRIANGLES, 0, _bufferCount);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(0);
        }

        CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, _bufferCount);
    }

    void DrawNode::onDrawGLLine(const MATH::Matrix4 &transform, uint32_t flags)
    {
        auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR);
        glProgram->use();
        glProgram->setUniformsForBuiltins(transform);

        if (_dirtyGLLine)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLLine, _bufferGLLine, GL_STREAM_DRAW);
            _dirtyGLLine = false;
        }
        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(_vaoGLLine);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLLine);
            enableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
            // vertex
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
            // color
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
            // texcood
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));
        }
        glLineWidth(2);
        glDrawArrays(GL_LINES, 0, _bufferCountGLLine);

        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_bufferCountGLLine);
    }

    void DrawNode::onDrawGLPoint(const MATH::Matrix4 &transform, uint32_t flags)
    {
        auto glProgram = GLProgramCache::getInstance()->getGLProgram(GLProgram::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE);
        glProgram->use();
        glProgram->setUniformsForBuiltins(transform);

        if (_dirtyGLPoint)
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacityGLPoint, _bufferGLPoint, GL_STREAM_DRAW);

            _dirtyGLPoint = false;
        }

        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(_vaoGLPoint);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER, _vboGLPoint);
            enableVertexAttribs( VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
            glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));
        }

        glDrawArrays(GL_POINTS, 0, _bufferCountGLPoint);

        if (Configuration::getInstance()->supportsShareableVAO())
        {
            bindVAO(0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_bufferCountGLPoint);
    }

    void DrawNode::drawPoint(const MATH::Vector2f& position, const float pointSize, const Color4F &color)
    {
        ensureCapacityGLPoint(1);

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(_bufferGLPoint + _bufferCountGLPoint);
        V2F_C4B_T2F a = {position, Color4B(color), Tex2F(pointSize,0)};
        *point = a;

        _bufferCountGLPoint += 1;
        _dirtyGLPoint = true;
    }

    void DrawNode::drawPoints(const MATH::Vector2f *position, unsigned int numberOfPoints, const Color4F &color)
    {
        drawPoints(position, numberOfPoints, 1.0, color);
    }

    void DrawNode::drawPoints(const MATH::Vector2f *position, unsigned int numberOfPoints, const float pointSize, const Color4F &color)
    {
        ensureCapacityGLPoint(numberOfPoints);

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(_bufferGLPoint + _bufferCountGLPoint);

        for(unsigned int i=0; i < numberOfPoints; i++,point++)
        {
            V2F_C4B_T2F a = {position[i], Color4B(color), Tex2F(pointSize,0)};
            *point = a;
        }

        _bufferCountGLPoint += numberOfPoints;
        _dirtyGLPoint = true;
    }

    void DrawNode::drawLine(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color)
    {
        ensureCapacityGLLine(2);

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(_bufferGLLine + _bufferCountGLLine);

        V2F_C4B_T2F a = {origin, Color4B(color), Tex2F(0.0, 0.0)};
        V2F_C4B_T2F b = {destination, Color4B(color), Tex2F(0.0, 0.0)};

        *point = a;
        *(point+1) = b;

        _bufferCountGLLine += 2;
        _dirtyGLLine = true;
    }

    void DrawNode::drawRect(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color)
    {
        drawLine(MATH::Vector2f(origin.x, origin.y), MATH::Vector2f(destination.x, origin.y), color);
        drawLine(MATH::Vector2f(destination.x, origin.y), MATH::Vector2f(destination.x, destination.y), color);
        drawLine(MATH::Vector2f(destination.x, destination.y), MATH::Vector2f(origin.x, destination.y), color);
        drawLine(MATH::Vector2f(origin.x, destination.y), MATH::Vector2f(origin.x, origin.y), color);
    }

    void DrawNode::drawPoly(const MATH::Vector2f *poli, unsigned int numberOfPoints, bool closePolygon, const Color4F &color)
    {
        unsigned int vertext_count;
        if(closePolygon)
        {
            vertext_count = 2 * numberOfPoints;
            ensureCapacityGLLine(vertext_count);
        }
        else
        {
            vertext_count = 2 * (numberOfPoints - 1);
            ensureCapacityGLLine(vertext_count);
        }

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(_bufferGLLine + _bufferCountGLLine);

        unsigned int i = 0;
        for(; i<numberOfPoints-1; i++)
        {
            V2F_C4B_T2F a = {poli[i], Color4B(color), Tex2F(0.0, 0.0)};
            V2F_C4B_T2F b = {poli[i+1], Color4B(color), Tex2F(0.0, 0.0)};

            *point = a;
            *(point+1) = b;
            point += 2;
        }
        if(closePolygon)
        {
            V2F_C4B_T2F a = {poli[i], Color4B(color), Tex2F(0.0, 0.0)};
            V2F_C4B_T2F b = {poli[0], Color4B(color), Tex2F(0.0, 0.0)};
            *point = a;
            *(point+1) = b;
        }

        _bufferCountGLLine += vertext_count;
    }

    void DrawNode::drawCircle(const MATH::Vector2f& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, float scaleX, float scaleY, const Color4F &color)
    {
        const float coef = 2.0f * (float)MATH_PI/segments;

        MATH::Vector2f *vertices = new (std::nothrow) MATH::Vector2f[segments+2];
        if( ! vertices )
            return;

        for(unsigned int i = 0;i <= segments; i++) {
            float rads = i*coef;
            GLfloat j = radius * cosf(rads + angle) * scaleX + center.x;
            GLfloat k = radius * sinf(rads + angle) * scaleY + center.y;

            vertices[i].x = j;
            vertices[i].y = k;
        }
        if(drawLineToCenter)
        {
            vertices[segments+1].x = center.x;
            vertices[segments+1].y = center.y;
            drawPoly(vertices, segments+2, true, color);
        }
        else
            drawPoly(vertices, segments+1, true, color);

        SAFE_DELETE_ARRAY(vertices);
    }

    void DrawNode::drawCircle(const MATH::Vector2f &center, float radius, float angle, unsigned int segments, bool drawLineToCenter, const Color4F &color)
    {
        drawCircle(center, radius, angle, segments, drawLineToCenter, 1.0f, 1.0f, color);
    }

    void DrawNode::drawQuadBezier(const MATH::Vector2f &origin, const MATH::Vector2f &control, const MATH::Vector2f &destination, unsigned int segments, const Color4F &color)
    {
        MATH::Vector2f* vertices = new (std::nothrow) MATH::Vector2f[segments + 1];
        if( ! vertices )
            return;

        float t = 0.0f;
        for(unsigned int i = 0; i < segments; i++)
        {
            vertices[i].x = powf(1 - t, 2) * origin.x + 2.0f * (1 - t) * t * control.x + t * t * destination.x;
            vertices[i].y = powf(1 - t, 2) * origin.y + 2.0f * (1 - t) * t * control.y + t * t * destination.y;
            t += 1.0f / segments;
        }
        vertices[segments].x = destination.x;
        vertices[segments].y = destination.y;

        drawPoly(vertices, segments+1, false, color);

        SAFE_DELETE_ARRAY(vertices);
    }

    void DrawNode::drawCubicBezier(const MATH::Vector2f &origin, const MATH::Vector2f &control1, const MATH::Vector2f &control2, const MATH::Vector2f &destination, unsigned int segments, const Color4F &color)
    {
        MATH::Vector2f* vertices = new (std::nothrow) MATH::Vector2f[segments + 1];
        if( ! vertices )
            return;

        float t = 0;
        for (unsigned int i = 0; i < segments; i++)
        {
            vertices[i].x = powf(1 - t, 3) * origin.x + 3.0f * powf(1 - t, 2) * t * control1.x + 3.0f * (1 - t) * t * t * control2.x + t * t * t * destination.x;
            vertices[i].y = powf(1 - t, 3) * origin.y + 3.0f * powf(1 - t, 2) * t * control1.y + 3.0f * (1 - t) * t * t * control2.y + t * t * t * destination.y;
            t += 1.0f / segments;
        }
        vertices[segments].x = destination.x;
        vertices[segments].y = destination.y;

        drawPoly(vertices, segments+1, false, color);

        SAFE_DELETE_ARRAY(vertices);
    }

    void DrawNode::drawDot(const MATH::Vector2f &pos, float radius, const Color4F &color)
    {
        unsigned int vertex_count = 2*3;
        ensureCapacity(vertex_count);

        V2F_C4B_T2F a = {MATH::Vector2f(pos.x - radius, pos.y - radius), Color4B(color), Tex2F(-1.0, -1.0) };
        V2F_C4B_T2F b = {MATH::Vector2f(pos.x - radius, pos.y + radius), Color4B(color), Tex2F(-1.0,  1.0) };
        V2F_C4B_T2F c = {MATH::Vector2f(pos.x + radius, pos.y + radius), Color4B(color), Tex2F( 1.0,  1.0) };
        V2F_C4B_T2F d = {MATH::Vector2f(pos.x + radius, pos.y - radius), Color4B(color), Tex2F( 1.0, -1.0) };

        V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
        V2F_C4B_T2F_Triangle triangle0 = {a, b, c};
        V2F_C4B_T2F_Triangle triangle1 = {a, c, d};
        triangles[0] = triangle0;
        triangles[1] = triangle1;

        _bufferCount += vertex_count;

        _dirty = true;
    }

    void DrawNode::drawRect(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const MATH::Vector2f& p4, const Color4F &color)
    {
        drawLine(MATH::Vector2f(p1.x, p1.y), MATH::Vector2f(p2.x, p2.y), color);
        drawLine(MATH::Vector2f(p2.x, p2.y), MATH::Vector2f(p3.x, p3.y), color);
        drawLine(MATH::Vector2f(p3.x, p3.y), MATH::Vector2f(p4.x, p4.y), color);
        drawLine(MATH::Vector2f(p4.x, p4.y), MATH::Vector2f(p1.x, p1.y), color);
    }

    void DrawNode::drawTriangle(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const Color4F &color)
    {
        unsigned int vertex_count = 3;
        ensureCapacity(vertex_count);

        Color4B col = Color4B(color);
        V2F_C4B_T2F a = {MATH::Vector2f(p1.x, p1.y), col, Tex2F(0.0, 0.0) };
        V2F_C4B_T2F b = {MATH::Vector2f(p2.x, p2.y), col, Tex2F(0.0,  0.0) };
        V2F_C4B_T2F c = {MATH::Vector2f(p3.x, p3.y), col, Tex2F(0.0,  0.0) };

        V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
        V2F_C4B_T2F_Triangle triangle = {a, b, c};
        triangles[0] = triangle;

        _bufferCount += vertex_count;
        _dirty = true;
    }

    void DrawNode::clear()
    {
        _bufferCount = 0;
        _dirty = true;
        _bufferCountGLLine = 0;
        _dirtyGLLine = true;
        _bufferCountGLPoint = 0;
        _dirtyGLPoint = true;
    }

    const BlendFunc& DrawNode::getBlendFunc() const
    {
        return _blendFunc;
    }

    void DrawNode::setBlendFunc(const BlendFunc &blendFunc)
    {
        _blendFunc = blendFunc;
    }
}
