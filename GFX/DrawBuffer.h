#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

#include "MATH/Matrix.h"
#include "THIN3D/Thin3D.h"

namespace GFX
{
    class DrawBuffer
    {
    public:
        DrawBuffer();
        ~DrawBuffer();

        void begin(Thin3DShaderSet *shaders, DrawBufferPrimitiveMode mode = DBMODE_NORMAL);
        void end();

        // TODO: Enforce these. Now Init is autocalled and shutdown not called.
        void init(Thin3DContext *t3d);
        void shutdown();

        int count() const { return count_; }

        void flush(bool set_blend_state = true);

        void rect(float x, float y, float w, float h, uint32 color, int align = ALIGN_TOPLEFT);
        void hLine(float x1, float y, float x2, uint32 color);
        void vLine(float x, float y1, float y2, uint32 color);
        void vLineAlpha50(float x, float y1, float y2, uint32 color);

        void line(int atlas_image, float x1, float y1, float x2, float y2, float thickness, uint32 color);

        void tectOutline(float x, float y, float w, float h, uint32 color, int align = ALIGN_TOPLEFT);

        void rectVGradient(float x, float y, float w, float h, uint32 colorTop, uint32 colorBottom);
        void rectVDarkFaded(float x, float y, float w, float h, uint32 colorTop) {
            RectVGradient(x, y, w, h, colorTop, darkenColor(colorTop));
        }

        void multiVGradient(float x, float y, float w, float h, GradientStop *stops, int numStops);

        void rectCenter(float x, float y, float w, float h, uint32 color) {
            rect(x - w/2, y - h/2, w, h, color);
        }
        void rect(float x, float y, float w, float h, float u, float v, float uw, float uh, uint32 color);

        void v(float x, float y, float z, uint32 color, float u, float v);
        void v(float x, float y, uint32 color, float u, float v) {
            v(x, y, 0.0f, color, u, v);
        }

        void circle(float x, float y, float radius, float thickness, int segments, float startAngle, uint32 color, float u_mul);

        // New drawing APIs

        // Must call this before you use any functions with atlas_image etc.
        void setAtlas(const Atlas *_atlas) {
            atlas = _atlas;
        }
        const Atlas *getAtlas() const { return atlas; }
        void measureImage(ImageID atlas_image, float *w, float *h);
        void drawImage(ImageID atlas_image, float x, float y, float scale, Color color = COLOR(0xFFFFFF), int align = ALIGN_TOPLEFT);
        void drawImageStretch(ImageID atlas_image, float x1, float y1, float x2, float y2, Color color = COLOR(0xFFFFFF));
        void drawImageStretch(int atlas_image, const Bounds &bounds, Color color = COLOR(0xFFFFFF)) {
            drawImageStretch(atlas_image, bounds.x, bounds.y, bounds.x2(), bounds.y2(), color);
        }
        void drawImageRotated(ImageID atlas_image, float x, float y, float scale, float angle, Color color = COLOR(0xFFFFFF), bool mirror_h = false);	// Always centers
        void drawTexRect(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, Color color);
        void drawTexRect(const Bounds &bounds, float u1, float v1, float u2, float v2, Color color) {
            drawTexRect(bounds.x, bounds.y, bounds.x2(), bounds.y2(), u1, v1, u2, v2, color);
        }
        // Results in 18 triangles. Kind of expensive for a button.
        void drawImage4Grid(ImageID atlas_image, float x1, float y1, float x2, float y2, Color color = COLOR(0xFFFFFF), float corner_scale = 1.0);
        // This is only 6 triangles, much cheaper.
        void drawImage2GridH(ImageID atlas_image, float x1, float y1, float x2, Color color = COLOR(0xFFFFFF), float scale = 1.0);

        void measureText(int font, const char *text, float *w, float *h);

        // NOTE: Count is in plain chars not utf-8 chars!
        void measureTextCount(int font, const char *text, int count, float *w, float *h);

        void drawTextRect(int font, const char *text, float x, float y, float w, float h, Color color = 0xFFFFFFFF, int align = 0);
        void drawText(int font, const char *text, float x, float y, Color color = 0xFFFFFFFF, int align = 0);
        void drawTextShadow(int font, const char *text, float x, float y, Color color = 0xFFFFFFFF, int align = 0);

        void rotateSprite(ImageID atlas_image, float x, float y, float angle, float scale, Color color);

        void setFontScale(float xs, float ys)
        {
            fontscalex = xs;
            fontscaley = ys;
        }

        static void doAlign(int flags, float *x, float *y, float *w, float *h);

        void setDrawMatrix(const MATH::Matrix4x4 &m)
        {
            drawMatrix_ = m;
        }

    private:
        struct Vertex {
            float x, y, z;
            float u, v;
            uint32_t rgba;
        };

        MATH::Matrix4x4 drawMatrix_;

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
