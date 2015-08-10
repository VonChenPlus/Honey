#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

#include "MATH/Matrix.h"
#include "MATH/Bounds.h"
#include "GRAPH/THIN3D/Thin3D.h"
#include "GRAPH/GFX/Texture.h"
#include "GRAPH/GFX/Atlas.h"
#include "GRAPH/BASE/Color.h"

namespace GFX
{
    enum
    {
        ALIGN_LEFT = 0,
        ALIGN_RIGHT = 16,
        ALIGN_TOP = 0,
        ALIGN_BOTTOM = 1,
        ALIGN_HCENTER = 4,
        ALIGN_VCENTER = 8,
        ALIGN_VBASELINE = 32,	// text only, possibly not yet working

        ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER,
        ALIGN_TOPLEFT = ALIGN_TOP | ALIGN_LEFT,
        ALIGN_TOPRIGHT = ALIGN_TOP | ALIGN_RIGHT,
        ALIGN_BOTTOMLEFT = ALIGN_BOTTOM | ALIGN_LEFT,
        ALIGN_BOTTOMRIGHT = ALIGN_BOTTOM | ALIGN_RIGHT,

        // Only for text drawing
        ROTATE_90DEG_LEFT = 256,
        ROTATE_90DEG_RIGHT = 512,
        ROTATE_180DEG = 1024,

        // For "uncachable" text like debug log.
        // Avoids using system font drawing as it's too slow.
        // Not actually used here but is reserved for whatever system wraps DrawBuffer.
        FLAG_DYNAMIC_ASCII = 2048,
        FLAG_NO_PREFIX = 4096  // means to not process ampersands
    };

    enum DrawBufferPrimitiveMode
    {
        DBMODE_NORMAL = 0,
        DBMODE_LINES = 1
    };

    struct GradientStop
    {
        float t;
        GRAPH::Color4B color;
    };

    class DrawBuffer
    {
    public:
        DrawBuffer();
        ~DrawBuffer();

        void begin(THIN3D::Thin3DShaderSet *shaders, DrawBufferPrimitiveMode mode = DBMODE_NORMAL);
        void end();

        // TODO: Enforce these. Now Init is autocalled and shutdown not called.
        void init(THIN3D::Thin3DContext *t3d);
        void shutdown();

        int count() const { return count_; }

        void flush(bool set_blend_state = true);

        void rect(float x, float y, float w, float h, GRAPH::Color4B color, int align = ALIGN_TOPLEFT);
        void hLine(float x1, float y, float x2, GRAPH::Color4B color);
        void vLine(float x, float y1, float y2, GRAPH::Color4B color);
        void vLineAlpha50(float x, float y1, float y2, GRAPH::Color4B color);

        void line(int atlas_image, float x1, float y1, float x2, float y2, float thickness, GRAPH::Color4B color);

        void rectOutline(float x, float y, float w, float h, GRAPH::Color4B color, int align = ALIGN_TOPLEFT);

        void rectVGradient(float x, float y, float w, float h, GRAPH::Color4B colorTop, GRAPH::Color4B colorBottom);
        void rectVDarkFaded(float x, float y, float w, float h, GRAPH::Color4B colorTop) {
            rectVGradient(x, y, w, h, colorTop, GRAPH::Color4B::DarkenColor(colorTop));
        }

        void multiVGradient(float x, float y, float w, float h, GradientStop *stops, int numStops);

        void rectCenter(float x, float y, float w, float h, GRAPH::Color4B color) {
            rect(x - w/2, y - h/2, w, h, color);
        }
        void rect(float x, float y, float w, float h, float u, float v, float uw, float uh, GRAPH::Color4B color);

        void v(float x, float y, float z, GRAPH::Color4B color, float u, float v);
        void v(float x, float y, GRAPH::Color4B color, float u, float _v) {
            v(x, y, 0.0f, color, u, _v);
        }

        void circle(float x, float y, float radius, float thickness, int segments, float startAngle, GRAPH::Color4B color, float u_mul);

        // New drawing APIs

        // Must call this before you use any functions with atlas_image etc.
        void setAtlas(const Atlas *_atlas) {
            atlas = _atlas;
        }
        const Atlas *getAtlas() const { return atlas; }
        void measureImage(ImageID atlas_image, float *w, float *h);
        void drawImage(ImageID atlas_image, float x, float y, float scale, GRAPH::Color4B = GRAPH::Color4B::WHITE, int align = ALIGN_TOPLEFT);
        void drawImageStretch(ImageID atlas_image, float x1, float y1, float x2, float y2, GRAPH::Color4B = GRAPH::Color4B::WHITE);
        void drawImageStretch(int atlas_image, const MATH::Boundsf &bounds, GRAPH::Color4B color = GRAPH::Color4B::WHITE) {
            drawImageStretch(atlas_image, bounds.left, bounds.top, bounds.right(), bounds.bottom(), color);
        }
        void drawImageRotated(ImageID atlas_image, float x, float y, float scale, float angle, GRAPH::Color4B = GRAPH::Color4B::WHITE, bool mirror_h = false);	// Always centers
        void drawTexRect(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, GRAPH::Color4B color);
        void drawTexRect(const MATH::Boundsf &bounds, float u1, float v1, float u2, float v2, GRAPH::Color4B color) {
            drawTexRect(bounds.left, bounds.top, bounds.right(), bounds.bottom(), u1, v1, u2, v2, color);
        }
        // Results in 18 triangles. Kind of expensive for a button.
        void drawImage4Grid(ImageID atlas_image, float x1, float y1, float x2, float y2, GRAPH::Color4B = GRAPH::Color4B::WHITE, float corner_scale = 1.0);
        // This is only 6 triangles, much cheaper.
        void drawImage2GridH(ImageID atlas_image, float x1, float y1, float x2, GRAPH::Color4B = GRAPH::Color4B::WHITE, float scale = 1.0);

        void measureText(int font, const char *text, float *w, float *h);

        // NOTE: Count is in plain chars not utf-8 chars!
        void measureTextCount(int font, const char *text, int count, float *w, float *h);

        void drawTextRect(int font, const char *text, float x, float y, float w, float h, GRAPH::Color4B = GRAPH::Color4B::WHITE, int align = 0);
        void drawText(int font, const char *text, float x, float y, GRAPH::Color4B = GRAPH::Color4B::WHITE, int align = 0);
        void drawTextShadow(int font, const char *text, float x, float y, GRAPH::Color4B = GRAPH::Color4B::WHITE, int align = 0);

        void rotateSprite(ImageID atlas_image, float x, float y, float angle, float scale, GRAPH::Color4B);

        void setFontScale(float xs, float ys) {
            fontscalex = xs;
            fontscaley = ys;
        }

        static void doAlign(int flags, float *x, float *y, float *w, float *h);

        void setDrawMatrix(const MATH::Matrix4 &m) {
            drawMatrix_ = m;
        }

    private:
        struct Vertex
        {
            float x, y, z;
            float u, v;
            GRAPH::Color4B rgba;
        };

        MATH::Matrix4 drawMatrix_;

        THIN3D::Thin3DContext *t3d_;
        THIN3D::Thin3DBuffer *vbuf_;
        THIN3D::Thin3DVertexFormat *vformat_;
        THIN3D::Thin3DShaderSet *shaderSet_;

        Vertex *verts_;
        int count_;
        DrawBufferPrimitiveMode mode_;
        const Atlas *atlas;

        bool inited_;
        float fontscalex;
        float fontscaley;
    };
}

#endif // DRAWBUFFER_H
