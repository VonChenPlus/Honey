#ifndef DRAWNODE_H
#define DRAWNODE_H

#include "GRAPH/BASE/Node.h"
#include "GRAPH/BASE/Types.h"
#include "GRAPH/RENDERER/RenderCommand.h"

namespace GRAPH
{
    class PointArray : public HObject
    {
    public:
        static PointArray* create(ssize_t capacity);

        virtual ~PointArray();
        PointArray();

        bool initWithCapacity(ssize_t capacity);

        void addControlPoint(MATH::Vector2f controlPoint);

        void insertControlPoint(MATH::Vector2f &controlPoint, ssize_t index);

        void replaceControlPoint(MATH::Vector2f &controlPoint, ssize_t index);

        MATH::Vector2f getControlPointAtIndex(ssize_t index);

        void removeControlPointAtIndex(ssize_t index);

        ssize_t count() const;

        PointArray* reverse() const;

        void reverseInline();

        virtual PointArray* clone() const;

        const std::vector<MATH::Vector2f*>* getControlPoints() const;

        void setControlPoints(std::vector<MATH::Vector2f*> *controlPoints);
    private:
        /** Array that contains the control points. */
        std::vector<MATH::Vector2f*> *_controlPoints;
    };

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
        void drawTriangle(const MATH::Vector2f &p1, const MATH::Vector2f &p2, const MATH::Vector2f &p3, const Color4F &color);

        void clear();

        const BlendFunc& getBlendFunc() const;
        void setBlendFunc(const BlendFunc &blendFunc);

        void onDraw(const MATH::Matrix4 &transform, uint32_t flags);
        void onDrawGLLine(const MATH::Matrix4 &transform, uint32_t flags);
        void onDrawGLPoint(const MATH::Matrix4 &transform, uint32_t flags);

        // Overrides
        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

    public:
        DrawNode();
        virtual ~DrawNode();
        virtual bool init() override;

    protected:
        void ensureCapacity(int count);
        void ensureCapacityGLPoint(int count);
        void ensureCapacityGLLine(int count);

        GLuint      _vao;
        GLuint      _vbo;
        GLuint      _vaoGLPoint;
        GLuint      _vboGLPoint;
        GLuint      _vaoGLLine;
        GLuint      _vboGLLine;

        int         _bufferCapacity;
        GLsizei     _bufferCount;
        V2F_C4B_T2F *_buffer;

        int         _bufferCapacityGLPoint;
        GLsizei     _bufferCountGLPoint;
        V2F_C4B_T2F *_bufferGLPoint;
        Color4F     _pointColor;
        int         _pointSize;

        int         _bufferCapacityGLLine;
        GLsizei     _bufferCountGLLine;
        V2F_C4B_T2F *_bufferGLLine;

        BlendFunc   _blendFunc;
        CustomCommand _customCommand;
        CustomCommand _customCommandGLPoint;
        CustomCommand _customCommandGLLine;

        bool        _dirty;
        bool        _dirtyGLPoint;
        bool        _dirtyGLLine;

    private:
        DISALLOW_COPY_AND_ASSIGN(DrawNode)
    };
}

#endif // DRAWNODE_H
