#ifndef DRAWNODE_H
#define DRAWNODE_H

#include "GRAPH/Node.h"
#include "GRAPH/Types.h"
#include "GRAPH/RENDERER/RenderCommand.h"

namespace GRAPH
{
    class DrawNode : public Node
    {
    public:
        static DrawNode* create();

        void drawPoint(const MATH::Vector2f& point, const float pointSize, const Color4F &color);
        void drawPoints(const MATH::Vector2f *position, unsigned int numberOfPoints, const Color4F &color);
        void drawPoints(const MATH::Vector2f *position, unsigned int numberOfPoints, const float pointSize, const Color4F &color);
        void drawLine(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color);
        void drawRect(const MATH::Vector2f &origin, const MATH::Vector2f &destination, const Color4F &color);
        void drawPoly(const MATH::Vector2f *poli, unsigned int numberOfPoints, bool closePolygon, const Color4F &color);
        void drawCircle( const MATH::Vector2f& center, float radius, float angle, unsigned int segments, bool drawLineToCenter, float scaleX, float scaleY, const Color4F &color);
        void drawCircle(const MATH::Vector2f &center, float radius, float angle, unsigned int segments, bool drawLineToCenter, const Color4F &color);
        void drawQuadBezier(const MATH::Vector2f &origin, const MATH::Vector2f &control, const MATH::Vector2f &destination, unsigned int segments, const Color4F &color);
        void drawCubicBezier(const MATH::Vector2f &origin, const MATH::Vector2f &control1, const MATH::Vector2f &control2, const MATH::Vector2f &destination, unsigned int segments, const Color4F &color);
        void drawDot(const MATH::Vector2f &pos, float radius, const Color4F &color);
        void drawRect(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const MATH::Vector2f& p4, const Color4F &color);
        void drawPolygon(const MATH::Vector2f *verts, int count, const Color4F &fillColor, float borderWidth, const Color4F &borderColor);
        void drawTriangle(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const Color4F &color);

        void clear();

        const BlendFunc& getBlendFunc() const;
        void setBlendFunc(const BlendFunc &blendFunc);

        void onDraw(const MATH::Matrix4 &transform, uint32_t flags);
        void onDrawGLLine(const MATH::Matrix4 &transform, uint32_t flags);
        void onDrawGLPoint(const MATH::Matrix4 &transform, uint32_t flags);

        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

    public:
        DrawNode();
        virtual ~DrawNode();
        virtual bool init() override;

    protected:
        void ensureCapacity(int count);
        void ensureCapacityGLPoint(int count);
        void ensureCapacityGLLine(int count);

    private:
        GLuint      vbo_;
        GLuint      vboGLPoint_;
        GLuint      vboGLLine_;

        int         bufferCapacity_;
        GLsizei     bufferCount_;
        V2F_C4B_T2F *buffer_;

        int         bufferCapacityGLPoint_;
        GLsizei     bufferCountGLPoint_;
        V2F_C4B_T2F *bufferGLPoint_;
        Color4F     pointColor_;

        int         bufferCapacityGLLine_;
        GLsizei     bufferCountGLLine_;
        V2F_C4B_T2F *bufferGLLine_;

        BlendFunc   blendFunc_;
        CustomCommand customCommand_;
        CustomCommand customCommandGLPoint_;
        CustomCommand customCommandGLLine_;

        bool        dirty_;
        bool        dirtyGLPoint_;
        bool        dirtyGLLine_;

    private:
        DISALLOW_COPY_AND_ASSIGN(DrawNode)
    };
}

#endif // DRAWNODE_H
