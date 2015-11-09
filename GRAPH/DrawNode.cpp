#include "GRAPH/DrawNode.h"
#include "GRAPH/UNITY3D/Unity3D.h"
#include "GRAPH/UNITY3D/ShaderCache.h"
#include "GRAPH/UNITY3D/ShaderState.h"
#include "GRAPH/UNITY3D/Unity3DGLState.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "GRAPH/UNITY3D/Renderer.h"

namespace GRAPH
{
    DrawNode::DrawNode() {
        blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;
        u3dContext_ = (Unity3DCreator::CreateContext());
        memset(vboArray_, 0, sizeof(VertexBufferObject<V2F_C4B_T2F>) * 3);
    }

    DrawNode::~DrawNode() {
        for (auto object : vboArray_) {
            free(object.u1.bufferData);
        }
        SAFE_DELETE_PTRARRAY(u3dVertexBuffer_, 3);
        SAFE_DELETE_PTRARRAY(u3dVertexFormat_, 3);
        SAFE_DELETE(u3dContext_);
    }

    DrawNode* DrawNode::create() {
        DrawNode* ret = new (std::nothrow) DrawNode();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }

        return ret;
    }

    void DrawNode::ensureCapacity(int type, uint64 count) {
        if(vboArray_[type].u1.bufferCount + count > vboArray_[type].u1.bufferCapacity) {
            vboArray_[type].u1.bufferCapacity += MATH::MATH_MAX(vboArray_[type].u1.bufferCapacity, count);
            vboArray_[type].u1.bufferData = (V2F_C4B_T2F*)realloc(vboArray_[type].u1.bufferData, vboArray_[type].u1.bufferCapacity*sizeof(V2F_C4B_T2F));
        }
    }

    bool DrawNode::init() {
        blendFunc_ = BlendFunc::ALPHA_PREMULTIPLIED;

        setU3DShaderState(ShaderState::getOrCreateWithShaderName(Unity3DShader::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR));

        for (int index = 0; index < 3; ++index) {
            ensureCapacity(0, 512);

            u3dVertexBuffer_[index] = Unity3DCreator::CreateBuffer(VERTEXDATA | STREAM);
            u3dVertexBuffer_[index]->setData((const uint8 *) vboArray_[index].u1.bufferData, sizeof(V2F_C4B_T2F) * vboArray_[index].u1.bufferCapacity);

            std::vector<U3DVertexComponent> vertexFormat = {
                U3DVertexComponent(SEM_POSITION, FLOATx3, sizeof(V2F_C4B_T2F), offsetof(V2F_C4B_T2F, vertices)),
                U3DVertexComponent(SEM_COLOR0, UNORM8x4, sizeof(V2F_C4B_T2F), offsetof(V2F_C4B_T2F, colors)),
                U3DVertexComponent(SEM_TEXCOORD0, FLOATx2, sizeof(V2F_C4B_T2F), offsetof(V2F_C4B_T2F, texCoords)) };
            u3dVertexFormat_[index] = Unity3DCreator::CreateVertexFormat(vertexFormat);
        }

        memset(dirty_, 0, sizeof(bool) * 3);

        return true;
    }

    void DrawNode::draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) {
        if(vboArray_[0].u1.bufferCount) {
            customCommand_.init(globalZOrder_, transform, flags);
            customCommand_.func = std::bind(&DrawNode::onDraw, this, transform, flags);
            renderer->addCommand(&customCommand_);
        }

        if(vboArray_[1].u1.bufferCount) {
            customCommandGLPoint_.init(globalZOrder_, transform, flags);
            customCommandGLPoint_.func = std::bind(&DrawNode::onDrawGLPoint, this, transform, flags);
            renderer->addCommand(&customCommandGLPoint_);
        }

        if(vboArray_[2].u1.bufferCount) {
            customCommandGLLine_.init(globalZOrder_, transform, flags);
            customCommandGLLine_.func = std::bind(&DrawNode::onDrawGLLine, this, transform, flags);
            renderer->addCommand(&customCommandGLLine_);
        }
    }

    void DrawNode::onDraw(const MATH::Matrix4 &transform, uint32_t) {
        auto u3dShader = getU3DShader();
        u3dShader->apply();
        u3dShader->setUniformsForBuiltins(transform);

        Unity3DGLState::OpenGLState().blendFunc.set(blendFunc_.src, blendFunc_.dst);

        if (dirty_[0]) {
            u3dVertexBuffer_[DEFAULT]->bind();
            u3dVertexBuffer_[DEFAULT]->setData((const uint8 *) vboArray_[0].u1.bufferData, sizeof(V2F_C4B_T2F)*vboArray_[0].u1.bufferCapacity);
            dirty_[0] = false;
        }

        u3dContext_->draw(PRIM_TRIANGLES, u3dVertexFormat_[DEFAULT], u3dVertexBuffer_[DEFAULT], vboArray_[DEFAULT].u1.bufferCount, 0);
    }

    void DrawNode::onDrawGLLine(const MATH::Matrix4 &transform, uint32_t) {
        auto u3dShader = ShaderCache::getInstance().getU3DShader(Unity3DShader::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR);
        u3dShader->apply();
        u3dShader->setUniformsForBuiltins(transform);

        if (dirty_[0]) {
            u3dVertexBuffer_[LINE]->bind();
            u3dVertexBuffer_[LINE]->setData((const uint8 *) vboArray_[LINE].u1.bufferData, sizeof(V2F_C4B_T2F)*vboArray_[LINE].u1.bufferCapacity);
            dirty_[0] = false;
        }

        u3dContext_->draw(PRIM_LINES, u3dVertexFormat_[LINE], u3dVertexBuffer_[LINE], vboArray_[LINE].u1.bufferCount, 0);
    }

    void DrawNode::onDrawGLPoint(const MATH::Matrix4 &transform, uint32_t) {
        auto u3dShader = ShaderCache::getInstance().getU3DShader(Unity3DShader::SHADER_NAME_POSITION_COLOR_TEXASPOINTSIZE);
        u3dShader->apply();
        u3dShader->setUniformsForBuiltins(transform);

        if (dirty_[0]) {
            u3dVertexBuffer_[POINT]->bind();
            u3dVertexBuffer_[POINT]->setData((const uint8 *) vboArray_[POINT].u1.bufferData, sizeof(V2F_C4B_T2F)*vboArray_[POINT].u1.bufferCapacity);
            dirty_[0] = false;
        }

        u3dContext_->draw(PRIM_POINTS, u3dVertexFormat_[POINT], u3dVertexBuffer_[POINT], vboArray_[POINT].u1.bufferCount, 0);
    }

    void DrawNode::drawPoint(const MATH::Vector2f& position, const float pointSize, const Color4F &color) {
        ensureCapacity(1, 1);

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(vboArray_[1].u1.bufferData + vboArray_[1].u1.bufferCount);
        V2F_C4B_T2F a = {position, Color4B(color), Tex2F(pointSize,0)};
        *point = a;

        vboArray_[1].u1.bufferCount += 1;
        dirty_[1] = true;
    }

    void DrawNode::drawPoints(const MATH::Vector2f *position, unsigned int numberOfPoints, const Color4F &color) {
        drawPoints(position, numberOfPoints, 1.0, color);
    }

    void DrawNode::drawPoints(const MATH::Vector2f *position, unsigned int numberOfPoints, const float pointSize, const Color4F &color) {
        ensureCapacity(1, numberOfPoints);

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(vboArray_[1].u1.bufferData + vboArray_[1].u1.bufferCount);

        for(unsigned int i=0; i < numberOfPoints; i++,point++) {
            V2F_C4B_T2F a = {position[i], Color4B(color), Tex2F(pointSize,0)};
            *point = a;
        }

        vboArray_[1].u1.bufferCount += numberOfPoints;
        dirty_[1] = true;
    }

    void DrawNode::drawLine(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color) {
        ensureCapacity(2, 2);

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(vboArray_[2].u1.bufferCount + vboArray_[2].u1.bufferCount);

        V2F_C4B_T2F a = {origin, Color4B(color), Tex2F(0.0, 0.0)};
        V2F_C4B_T2F b = {destination, Color4B(color), Tex2F(0.0, 0.0)};

        *point = a;
        *(point+1) = b;

        vboArray_[2].u1.bufferCount += 2;
        dirty_[2] = true;
    }

    void DrawNode::drawRect(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color) {
        drawLine(MATH::Vector2f(origin.x, origin.y), MATH::Vector2f(destination.x, origin.y), color);
        drawLine(MATH::Vector2f(destination.x, origin.y), MATH::Vector2f(destination.x, destination.y), color);
        drawLine(MATH::Vector2f(destination.x, destination.y), MATH::Vector2f(origin.x, destination.y), color);
        drawLine(MATH::Vector2f(origin.x, destination.y), MATH::Vector2f(origin.x, origin.y), color);
    }

    void DrawNode::drawPoly(const MATH::Vector2f *poli, unsigned int numberOfPoints, bool closePolygon, const Color4F &color) {
        unsigned int vertext_count;
        if(closePolygon) {
            vertext_count = 2 * numberOfPoints;
            ensureCapacity(2, vertext_count);
        }
        else {
            vertext_count = 2 * (numberOfPoints - 1);
            ensureCapacity(2, vertext_count);
        }

        V2F_C4B_T2F *point = (V2F_C4B_T2F*)(vboArray_[2].u1.bufferCount + vboArray_[2].u1.bufferCount);

        unsigned int i = 0;
        for(; i<numberOfPoints-1; i++) {
            V2F_C4B_T2F a = {poli[i], Color4B(color), Tex2F(0.0, 0.0)};
            V2F_C4B_T2F b = {poli[i+1], Color4B(color), Tex2F(0.0, 0.0)};

            *point = a;
            *(point+1) = b;
            point += 2;
        }

        if(closePolygon) {
            V2F_C4B_T2F a = {poli[i], Color4B(color), Tex2F(0.0, 0.0)};
            V2F_C4B_T2F b = {poli[0], Color4B(color), Tex2F(0.0, 0.0)};
            *point = a;
            *(point+1) = b;
        }

        vboArray_[2].u1.bufferCount += vertext_count;
    }

    void DrawNode::drawCircle(const MATH::Vector2f& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, float scaleX, float scaleY, const Color4F &color) {
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

        if(drawLineToCenter) {
            vertices[segments+1].x = center.x;
            vertices[segments+1].y = center.y;
            drawPoly(vertices, segments+2, true, color);
        } else
            drawPoly(vertices, segments+1, true, color);

        SAFE_DELETE_ARRAY(vertices);
    }

    void DrawNode::drawCircle(const MATH::Vector2f &center, float radius, float angle, unsigned int segments, bool drawLineToCenter, const Color4F &color) {
        drawCircle(center, radius, angle, segments, drawLineToCenter, 1.0f, 1.0f, color);
    }

    void DrawNode::drawQuadBezier(const MATH::Vector2f &origin, const MATH::Vector2f &control, const MATH::Vector2f &destination, unsigned int segments, const Color4F &color) {
        MATH::Vector2f* vertices = new (std::nothrow) MATH::Vector2f[segments + 1];
        if( ! vertices )
            return;

        float t = 0.0f;
        for(unsigned int i = 0; i < segments; i++) {
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
        for (unsigned int i = 0; i < segments; i++) {
            vertices[i].x = powf(1 - t, 3) * origin.x + 3.0f * powf(1 - t, 2) * t * control1.x + 3.0f * (1 - t) * t * t * control2.x + t * t * t * destination.x;
            vertices[i].y = powf(1 - t, 3) * origin.y + 3.0f * powf(1 - t, 2) * t * control1.y + 3.0f * (1 - t) * t * t * control2.y + t * t * t * destination.y;
            t += 1.0f / segments;
        }
        vertices[segments].x = destination.x;
        vertices[segments].y = destination.y;

        drawPoly(vertices, segments+1, false, color);

        SAFE_DELETE_ARRAY(vertices);
    }

    void DrawNode::drawDot(const MATH::Vector2f &pos, float radius, const Color4F &color) {
        unsigned int vertex_count = 2*3;
        ensureCapacity(0, vertex_count);

        V2F_C4B_T2F a = {MATH::Vector2f(pos.x - radius, pos.y - radius), Color4B(color), Tex2F(-1.0, -1.0) };
        V2F_C4B_T2F b = {MATH::Vector2f(pos.x - radius, pos.y + radius), Color4B(color), Tex2F(-1.0,  1.0) };
        V2F_C4B_T2F c = {MATH::Vector2f(pos.x + radius, pos.y + radius), Color4B(color), Tex2F( 1.0,  1.0) };
        V2F_C4B_T2F d = {MATH::Vector2f(pos.x + radius, pos.y - radius), Color4B(color), Tex2F( 1.0, -1.0) };

        V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(vboArray_[0].u1.bufferData + vboArray_[0].u1.bufferCount);
        V2F_C4B_T2F_Triangle triangle0 = {a, b, c};
        V2F_C4B_T2F_Triangle triangle1 = {a, c, d};
        triangles[0] = triangle0;
        triangles[1] = triangle1;

        vboArray_[0].u1.bufferCount += vertex_count;
        dirty_[0] = true;
    }

    void DrawNode::drawRect(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const MATH::Vector2f& p4, const Color4F &color) {
        drawLine(MATH::Vector2f(p1.x, p1.y), MATH::Vector2f(p2.x, p2.y), color);
        drawLine(MATH::Vector2f(p2.x, p2.y), MATH::Vector2f(p3.x, p3.y), color);
        drawLine(MATH::Vector2f(p3.x, p3.y), MATH::Vector2f(p4.x, p4.y), color);
        drawLine(MATH::Vector2f(p4.x, p4.y), MATH::Vector2f(p1.x, p1.y), color);
    }

    void DrawNode::drawSolidRect(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color) {
        MATH::Vector2f vertices[] = {
            origin,
            MATH::Vector2f(destination.x, origin.y),
            destination,
            MATH::Vector2f(origin.x, destination.y)
        };

        drawSolidPoly(vertices, 4, color );
    }

    void DrawNode::drawSolidPoly(const MATH::Vector2f *poli, unsigned int numberOfPoints, const Color4F &color) {
        drawPolygon(poli, numberOfPoints, color, 0.0, Color4F(0.0, 0.0, 0.0, 0.0));
    }

    void DrawNode::drawSolidCircle(const MATH::Vector2f& center, float radius, float angle, unsigned int segments, float scaleX, float scaleY, const Color4F &color) {
        const float coef = 2.0f * (float)MATH_PI/segments;

        MATH::Vector2f *vertices = new (std::nothrow) MATH::Vector2f[segments];
        if( ! vertices )
            return;

        for(unsigned int i = 0;i < segments; i++)
        {
            float rads = i*coef;
            GLfloat j = radius * cosf(rads + angle) * scaleX + center.x;
            GLfloat k = radius * sinf(rads + angle) * scaleY + center.y;

            vertices[i].x = j;
            vertices[i].y = k;
        }

        drawSolidPoly(vertices, segments, color);

        SAFE_DELETE_ARRAY(vertices);
    }

    void DrawNode::drawSolidCircle(const MATH::Vector2f& center, float radius, float angle, unsigned int segments, const Color4F& color) {
        drawSolidCircle(center, radius, angle, segments, 1.0f, 1.0f, color);
    }

    void DrawNode::drawSegment(const MATH::Vector2f &from, const MATH::Vector2f &to, float radius, const Color4F &color) {
        unsigned int vertex_count = 6*3;

        MATH::Vector2f a = from;
        MATH::Vector2f b = to;
        MATH::Vector2f n = MATH::Vector2f::normalize(MATH::Vector2f::perp(MATH::Vector2f::subtract(b, a)));
        MATH::Vector2f t = MATH::Vector2f::perp(n);
        MATH::Vector2f nw = MATH::Vector2f::scale(n, radius);
        MATH::Vector2f tw = MATH::Vector2f::scale(t, radius);
        MATH::Vector2f v0 = MATH::Vector2f::subtract(b, MATH::Vector2f::add(nw, tw));
        MATH::Vector2f v1 = MATH::Vector2f::add(b, MATH::Vector2f::subtract(nw, tw));
        MATH::Vector2f v2 = MATH::Vector2f::subtract(b, nw);
        MATH::Vector2f v3 = MATH::Vector2f::add(b, nw);
        MATH::Vector2f v4 = MATH::Vector2f::subtract(a, nw);
        MATH::Vector2f v5 = MATH::Vector2f::add(a, nw);
        MATH::Vector2f v6 = MATH::Vector2f::subtract(a, MATH::Vector2f::subtract(nw, tw));
        MATH::Vector2f v7 = MATH::Vector2f::add(a, MATH::Vector2f::add(nw, tw));

        V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(vboArray_[0].u1.bufferData + vboArray_[0].u1.bufferCount);

        V2F_C4B_T2F_Triangle triangles0 = {
            {v0, Color4B(color), MATH::Vector2f::negate(MATH::Vector2f::add(n, t))},
            {v1, Color4B(color), MATH::Vector2f::subtract(n, t)},
            {v2, Color4B(color), MATH::Vector2f::negate(n)},
        };
        triangles[0] = triangles0;

        V2F_C4B_T2F_Triangle triangles1 = {
            {v3, Color4B(color), n},
            {v1, Color4B(color), MATH::Vector2f::subtract(n, t)},
            {v2, Color4B(color), MATH::Vector2f::negate(n)},
        };
        triangles[1] = triangles1;

        V2F_C4B_T2F_Triangle triangles2 = {
            {v3, Color4B(color), n},
            {v4, Color4B(color), MATH::Vector2f::negate(n)},
            {v2, Color4B(color), MATH::Vector2f::negate(n)},
        };
        triangles[2] = triangles2;

        V2F_C4B_T2F_Triangle triangles3 = {
            {v3, Color4B(color), n},
            {v4, Color4B(color), MATH::Vector2f::negate(n)},
            {v5, Color4B(color), n },
        };
        triangles[3] = triangles3;

        V2F_C4B_T2F_Triangle triangles4 = {
            {v6, Color4B(color), MATH::Vector2f::subtract(t, n)},
            {v4, Color4B(color), MATH::Vector2f::negate(n) },
            {v5, Color4B(color), n},
        };
        triangles[4] = triangles4;

        V2F_C4B_T2F_Triangle triangles5 = {
            {v6, Color4B(color), MATH::Vector2f::subtract(t, n)},
            {v7, Color4B(color), MATH::Vector2f::add(n, t)},
            {v5, Color4B(color), n},
        };
        triangles[5] = triangles5;

        vboArray_[0].u1.bufferCount += vertex_count;

        dirty_[0] = true;
    }

    void DrawNode::drawPolygon(const MATH::Vector2f *verts, int count, const Color4F &fillColor, float borderWidth, const Color4F &borderColor)
    {
        bool outline = (borderColor.alpha > 0.0 && borderWidth > 0.0);

        auto  triangle_count = outline ? (3*count - 2) : (count - 2);
        auto vertex_count = 3*triangle_count;
        ensureCapacity(0, vertex_count);

        V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(vboArray_[0].u1.bufferData + vboArray_[0].u1.bufferCount);
        V2F_C4B_T2F_Triangle *cursor = triangles;

        for (int i = 0; i < count-2; i++) {
            V2F_C4B_T2F_Triangle tmp = {
                {verts[0], Color4B(fillColor), MATH::Vec2fZERO},
                {verts[i+1], Color4B(fillColor), MATH::Vec2fZERO},
                {verts[i+2], Color4B(fillColor), MATH::Vec2fZERO},
            };

            *cursor++ = tmp;
        }

        if(outline) {
            struct ExtrudeVerts {MATH::Vector2f offset, n;};
            struct ExtrudeVerts* extrude = (struct ExtrudeVerts*)malloc(sizeof(struct ExtrudeVerts)*count);
            memset(extrude, 0, sizeof(struct ExtrudeVerts)*count);

            for (int i = 0; i < count; i++) {
                MATH::Vector2f v0 = verts[(i-1+count)%count];
                MATH::Vector2f v1 = verts[i];
                MATH::Vector2f v2 = verts[(i+1)%count];

                MATH::Vector2f n1 = MATH::Vector2f::normalize(MATH::Vector2f::perp(MATH::Vector2f::subtract(v1, v0)));
                MATH::Vector2f n2 = MATH::Vector2f::normalize(MATH::Vector2f::perp(MATH::Vector2f::subtract(v2, v1)));

                MATH::Vector2f offset = MATH::Vector2f::scale(MATH::Vector2f::add(n1, n2), 1.0/(MATH::Vector2f::dot(n1, n2) + 1.0));
                struct ExtrudeVerts tmp = {offset, n2};
                extrude[i] = tmp;
            }

            for(int i = 0; i < count; i++) {
                int j = (i+1)%count;
                MATH::Vector2f v0 = verts[i];
                MATH::Vector2f v1 = verts[j];

                MATH::Vector2f n0 = extrude[i].n;

                MATH::Vector2f offset0 = extrude[i].offset;
                MATH::Vector2f offset1 = extrude[j].offset;

                MATH::Vector2f inner0 = MATH::Vector2f::subtract(v0, MATH::Vector2f::scale(offset0, borderWidth));
                MATH::Vector2f inner1 = MATH::Vector2f::subtract(v1, MATH::Vector2f::scale(offset1, borderWidth));
                MATH::Vector2f outer0 = MATH::Vector2f::subtract(v0, MATH::Vector2f::scale(offset0, borderWidth));
                MATH::Vector2f outer1 = MATH::Vector2f::subtract(v1, MATH::Vector2f::scale(offset1, borderWidth));

                V2F_C4B_T2F_Triangle tmp1 = {
                    {inner0, Color4B(borderColor), MATH::Vector2f::negate(n0)},
                    {inner1, Color4B(borderColor), MATH::Vector2f::negate(n0)},
                    {outer1, Color4B(borderColor), n0}
                };
                *cursor++ = tmp1;

                V2F_C4B_T2F_Triangle tmp2 = {
                    {inner0, Color4B(borderColor), MATH::Vector2f::negate(n0)},
                    {outer0, Color4B(borderColor), n0},
                    {outer1, Color4B(borderColor), n0}
                };
                *cursor++ = tmp2;
            }

            free(extrude);
        }

        vboArray_[0].u1.bufferCount += vertex_count;
        dirty_[0] = true;
    }

    void DrawNode::drawTriangle(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const Color4F &color) {
        unsigned int vertex_count = 3;
        ensureCapacity(0, vertex_count);

        Color4B col = Color4B(color);
        V2F_C4B_T2F a = {MATH::Vector2f(p1.x, p1.y), col, Tex2F(0.0, 0.0) };
        V2F_C4B_T2F b = {MATH::Vector2f(p2.x, p2.y), col, Tex2F(0.0,  0.0) };
        V2F_C4B_T2F c = {MATH::Vector2f(p3.x, p3.y), col, Tex2F(0.0,  0.0) };

        V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(vboArray_[0].u1.bufferData + vboArray_[0].u1.bufferCount);
        V2F_C4B_T2F_Triangle triangle = {a, b, c};
        triangles[0] = triangle;

        vboArray_[0].u1.bufferCount += vertex_count;
        dirty_[0] = true;
    }

    void DrawNode::clear() {
        for (auto &object : vboArray_) {
            object.u1.bufferCount = 0;
        }
        memset(dirty_, 0, sizeof(bool) * 3);
    }

    const BlendFunc& DrawNode::getBlendFunc() const {
        return blendFunc_;
    }

    void DrawNode::setBlendFunc(const BlendFunc &blendFunc) {
        blendFunc_ = blendFunc;
    }
}
