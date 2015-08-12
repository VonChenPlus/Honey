#include "DrawBuffer.h"

#include <cmath>
#include <algorithm>

using THIN3D::Thin3DContext;
using THIN3D::Thin3DVertexComponent;
using THIN3D::Thin3DShader;
using THIN3D::Thin3DShaderSet;
using THIN3D::PRIM_TRIANGLES;
using THIN3D::PRIM_LINES;
using THIN3D::SEM_POSITION;
using THIN3D::SEM_TEXCOORD0;
using THIN3D::SEM_COLOR0;
using THIN3D::FLOATx3;
using THIN3D::FLOATx2;
using THIN3D::UNORM8x4;
using THIN3D::VS_TEXTURE_COLOR_2D;
#include "MATH/MathDef.h"
#include "GRAPH/GFX/Atlas.h"

namespace GLOBAL
{
    extern float &pixelInDPS();
}

namespace GFX
{
    extern Atlas _UIAtlas;

    enum
    {
        // Enough?
        MAX_VERTS = 65536,
    };

    // #define USE_VBO

    DrawBuffer::DrawBuffer()
        : count_(0)
        , atlas(&_UIAtlas) {
        verts_ = new Vertex[MAX_VERTS];
        fontscalex = 1.0f;
        fontscaley = 1.0f;
        inited_ = false;
    }

    DrawBuffer::~DrawBuffer() {
        delete [] verts_;
    }

    void DrawBuffer::init(Thin3DContext *t3d) {
        if (inited_)
            return;

        t3d_ = t3d;
    #ifdef USE_VBO
        vbuf_ = t3d_->createBuffer(MAX_VERTS * sizeof(Vertex), T3DBufferUsage::DYNAMIC | T3DBufferUsage::VERTEXDATA);
    #else
        vbuf_ = NULLPTR;
    #endif
        inited_ = true;
        std::vector<Thin3DVertexComponent> components;
        components.push_back(Thin3DVertexComponent("Position", SEM_POSITION, FLOATx3, 0));
        components.push_back(Thin3DVertexComponent("TexCoord0", SEM_TEXCOORD0, FLOATx2, 12));
        components.push_back(Thin3DVertexComponent("Color0", SEM_COLOR0, UNORM8x4, 20));

        Thin3DShader *vshader = t3d_->getVshaderPreset(VS_TEXTURE_COLOR_2D);

        vformat_ = t3d_->createVertexFormat(components, 24, vshader);
    }

    void DrawBuffer::shutdown() {
        if (vbuf_) {
            vbuf_->release();
        }
        vformat_->release();

        inited_ = false;
    }

    void DrawBuffer::begin(Thin3DShaderSet *program, DrawBufferPrimitiveMode dbmode) {
        shaderSet_ = program;
        count_ = 0;
        mode_ = dbmode;
    }

    void DrawBuffer::end() {
        // Currently does nothing, but call it!
    }

    void DrawBuffer::flush(bool set_blend_state) {
        UNUSED(set_blend_state);

        if (!shaderSet_) {
            throw _HException_("No program set!", HException::GFX);
        }

        if (count_ == 0)
            return;

        shaderSet_->setMatrix4("WorldViewProj", drawMatrix_);
    #ifdef USE_VBO
        vbuf_->subData((const uint8 *)verts_, 0, sizeof(Vertex) * count_);
        int offset = 0;
        t3d_->draw(mode_ == DBMODE_NORMAL ? PRIM_TRIANGLES : PRIM_LINES, shaderSet_, vformat_, vbuf_, count_, offset);
    #else
        t3d_->drawUP(mode_ == DBMODE_NORMAL ? PRIM_TRIANGLES : PRIM_LINES, shaderSet_, vformat_, (const void *)verts_, count_);
    #endif
        count_ = 0;
    }

    void DrawBuffer::v(float x, float y, float z, GRAPH::Color4B color, float u, float v) {
        if (count_ >= MAX_VERTS) {
            //FLOG("Overflowed the DrawBuffer");
            return;
        }

        Vertex *vert = &verts_[count_++];
        vert->x = x;
        vert->y = y;
        vert->z = z;
        vert->rgba = color;
        vert->u = u;
        vert->v = v;
    }

    void DrawBuffer::rect(float x, float y, float w, float h, GRAPH::Color4B color, int align) {
        doAlign(align, &x, &y, &w, &h);
        rectVGradient(x, y, w, h, color, color);
    }

    void DrawBuffer::hLine(float x1, float y, float x2, GRAPH::Color4B color) {
        rect(x1, y, x2 - x1, GLOBAL::pixelInDPS(), color);
    }

    void DrawBuffer::vLine(float x, float y1, float y2, GRAPH::Color4B color) {
        rect(x, y1, GLOBAL::pixelInDPS(), y2 - y1, color);
    }

    void DrawBuffer::vLineAlpha50(float x, float y1, float y2, GRAPH::Color4B color) {
        rect(x, y1, GLOBAL::pixelInDPS(), y2 - y1, (color | 0xFF000000) & 0x7F000000);
    }

    void DrawBuffer::rectVGradient(float x, float y, float w, float h, GRAPH::Color4B colorTop, GRAPH::Color4B colorBottom) {
        v(x,		 y,     0, colorTop,    0, 0);
        v(x + w, y,		 0, colorTop,    1, 0);
        v(x + w, y + h, 0, colorBottom, 1, 1);
        v(x,		 y,     0, colorTop,    0, 0);
        v(x + w, y + h, 0, colorBottom, 1, 1);
        v(x,		 y + h, 0, colorBottom, 0, 1);
    }

    void DrawBuffer::rectOutline(float x, float y, float w, float h, GRAPH::Color4B color, int align) {
        UNUSED(align);

        hLine(x, y, x + w + GLOBAL::pixelInDPS(), color);
        hLine(x, y + h, x + w + GLOBAL::pixelInDPS(), color);

        vLine(x, y, y + h + GLOBAL::pixelInDPS(), color);
        vLine(x + w, y, y + h + GLOBAL::pixelInDPS(), color);
    }

    void DrawBuffer::multiVGradient(float x, float y, float w, float h, GradientStop *stops, int numStops) {
        for (int i = 0; i < numStops - 1; i++) {
            float t0 = stops[i].t, t1 = stops[i+1].t;
            uint32 c0 = stops[i].t, c1 = stops[i+1].t;
            rectVGradient(x, y + h * t0, w, h * (t1 - t0), c0, c1);
        }
    }

    void DrawBuffer::rect(float x, float y, float w, float h,
        float u, float _v, float uw, float uh,
        GRAPH::Color4B color) {
            v(x,	   y,     0, color, u, _v);
            v(x + w, y,	   0, color, u + uw, _v);
            v(x + w, y + h, 0, color, u + uw, _v + uh);
            v(x,	   y,     0, color, u, _v);
            v(x + w, y + h, 0, color, u + uw, _v + uh);
            v(x,	   y + h, 0, color, u, _v + uh);
    }

    void DrawBuffer::line(int atlas_image, float x1, float y1, float x2, float y2, float thickness, GRAPH::Color4B color) {
        const AtlasImage &image = atlas->images[atlas_image];

        // No caps yet!
        // Pre-rotated - we are making a thick line here
        float dx = -(y2 - y1);
        float dy = x2 - x1;
        float len = sqrtf(dx * dx + dy * dy) / thickness;
        if (len <= 0.0f)
            len = 1.0f;

        dx /= len;
        dy /= len;

        float x[4] = { x1 - dx, x2 - dx, x1 + dx, x2 + dx };
        float y[4] = { y1 - dy, y2 - dy, y1 + dy, y2 + dy };

        v(x[0],	y[0], color, image.u1, image.v1);
        v(x[1],	y[1], color, image.u2, image.v1);
        v(x[2],	y[2], color, image.u1, image.v2);
        v(x[2],	y[2], color, image.u1, image.v2);
        v(x[1],	y[1], color, image.u2, image.v1);
        v(x[3],	y[3], color, image.u2, image.v2);
    }

    void DrawBuffer::measureImage(ImageID atlas_image, float *w, float *h) {
        const AtlasImage &image = atlas->images[atlas_image];
        *w = (float)image.w;
        *h = (float)image.h;
    }

    void DrawBuffer::drawImage(ImageID atlas_image, float x, float y, float scale, GRAPH::Color4B color, int align) {
        const AtlasImage &image = atlas->images[atlas_image];
        float w = (float)image.w * scale;
        float h = (float)image.h * scale;
        if (align & ALIGN_HCENTER) x -= w / 2;
        if (align & ALIGN_RIGHT) x -= w;
        if (align & ALIGN_VCENTER) y -= h / 2;
        if (align & ALIGN_BOTTOM) y -= h;
        drawImageStretch(atlas_image, x, y, x + w, y + h, color);
    }

    void DrawBuffer::drawImageStretch(ImageID atlas_image, float x1, float y1, float x2, float y2, GRAPH::Color4B color) {
        const AtlasImage &image = atlas->images[atlas_image];
        v(x1,	y1, color, image.u1, image.v1);
        v(x2,	y1, color, image.u2, image.v1);
        v(x2,	y2, color, image.u2, image.v2);
        v(x1,	y1, color, image.u1, image.v1);
        v(x2,	y2, color, image.u2, image.v2);
        v(x1,	y2, color, image.u1, image.v2);
    }

    static inline void Rotated(float *v, float angle, float xc, float yc) {
        const float x = v[0] - xc;
        const float y = v[1] - yc;
        const float sa = sinf(angle);
        const float ca = cosf(angle);
        v[0] = x * ca + y * -sa + xc;
        v[1] = x * sa + y *  ca + yc;
    }

    void DrawBuffer::drawImageRotated(ImageID atlas_image, float x, float y, float scale, float angle, GRAPH::Color4B color, bool mirror_h) {
        const AtlasImage &image = atlas->images[atlas_image];
        float w = (float)image.w * scale;
        float h = (float)image.h * scale;
        float x1 = x - w / 2;
        float x2 = x + w / 2;
        float y1 = y - h / 2;
        float y2 = y + h / 2;
        float _v[6][2] = {
            {x1, y1},
            {x2, y1},
            {x2, y2},
            {x1, y1},
            {x2, y2},
            {x1, y2},
        };
        float u1 = image.u1;
        float u2 = image.u2;
        if (mirror_h) {
            float temp = u1;
            u1 = u2;
            u2 = temp;
        }
        const float uv[6][2] = {
            {u1, image.v1},
            {u2, image.v1},
            {u2, image.v2},
            {u1, image.v1},
            {u2, image.v2},
            {u1, image.v2},
        };
        for (int i = 0; i < 6; i++) {
            Rotated(_v[i], angle, x, y);
            v(_v[i][0], _v[i][1], 0, color, uv[i][0], uv[i][1]);
        }
    }

    // TODO: add arc support
    void DrawBuffer::circle(float xc, float yc, float radius, float thickness, int segments, float startAngle, GRAPH::Color4B color, float u_mul) {
        UNUSED(startAngle);

        float angleDelta = MATH_PI * 2 / segments;
        float uDelta = 1.0f / segments;
        float t2 = thickness / 2.0f;
        float r1 = radius + t2;
        float r2 = radius - t2;
        for (int i = 0; i < segments + 1; i++) {
            float angle1 = i * angleDelta;
            float angle2 = (i + 1) * angleDelta;
            float u1 = u_mul * i * uDelta;
            float u2 = u_mul * (i + 1) * uDelta;
            // TODO: get rid of one pair of cos/sin per loop, can reuse from last iteration
            float c1 = cosf(angle1), s1 = sinf(angle1), c2 = cosf(angle2), s2 = sinf(angle2);
            const float x[4] = {c1 * r1 + xc, c2 * r1 + xc, c1 * r2 + xc, c2 * r2 + xc};
            const float y[4] = {s1 * r1 + yc, s2 * r1 + yc, s1 * r2 + yc, s2 * r2 + yc};
            v(x[0],	y[0], color, u1, 0);
            v(x[1],	y[1], color, u2, 0);
            v(x[2],	y[2], color, u1, 1);
            v(x[1],	y[1], color, u2, 0);
            v(x[3],	y[3], color, u2, 1);
            v(x[2],	y[2], color, u1, 1);
        }
    }

    void DrawBuffer::drawTexRect(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, GRAPH::Color4B color) {
        v(x1,	y1, color, u1, v1);
        v(x2,	y1, color, u2, v1);
        v(x2,	y2, color, u2, v2);
        v(x1,	y1, color, u1, v1);
        v(x2,	y2, color, u2, v2);
        v(x1,	y2, color, u1, v2);
    }

    void DrawBuffer::drawImage4Grid(ImageID atlas_image, float x1, float y1, float x2, float y2, GRAPH::Color4B color, float corner_scale) {
        const AtlasImage &image = atlas->images[atlas_image];

        float u1 = image.u1, v1 = image.v1, u2 = image.u2, v2 = image.v2;
        float um = (u2 + u1) * 0.5f;
        float vm = (v2 + v1) * 0.5f;
        float iw2 = (image.w * 0.5f) * corner_scale;
        float ih2 = (image.h * 0.5f) * corner_scale;
        float xa = x1 + iw2;
        float xb = x2 - iw2;
        float ya = y1 + ih2;
        float yb = y2 - ih2;
        // Top row
        drawTexRect(x1, y1, xa, ya, u1, v1, um, vm, color);
        drawTexRect(xa, y1, xb, ya, um, v1, um, vm, color);
        drawTexRect(xb, y1, x2, ya, um, v1, u2, vm, color);
        // Middle row
        drawTexRect(x1, ya, xa, yb, u1, vm, um, vm, color);
        drawTexRect(xa, ya, xb, yb, um, vm, um, vm, color);
        drawTexRect(xb, ya, x2, yb, um, vm, u2, vm, color);
        // Bottom row
        drawTexRect(x1, yb, xa, y2, u1, vm, um, v2, color);
        drawTexRect(xa, yb, xb, y2, um, vm, um, v2, color);
        drawTexRect(xb, yb, x2, y2, um, vm, u2, v2, color);
    }

    void DrawBuffer::drawImage2GridH(ImageID atlas_image, float x1, float y1, float x2, GRAPH::Color4B color, float corner_scale) {
        const AtlasImage &image = atlas->images[atlas_image];
        float um = (image.u1 + image.u2) * 0.5f;
        float iw2 = (image.w * 0.5f) * corner_scale;
        float xa = x1 + iw2;
        float xb = x2 - iw2;
        float u1 = image.u1, v1 = image.v1, u2 = image.u2, v2 = image.v2;
        float y2 = y1 + image.h;
        drawTexRect(x1, y1, xa, y2, u1, v1, um, v2, color);
        drawTexRect(xa, y1, xb, y2, um, v1, um, v2, color);
        drawTexRect(xb, y1, x2, y2, um, v1, u2, v2, color);
    }

    void DrawBuffer::measureTextCount(int font, const char *text, int count, float *width, float *height) {
        const AtlasFont &atlasfont = *atlas->fonts[font];

        unsigned int cval;
        float wacc = 0;
        float maxX = 0.0f;
        int lines = 1;
        int index = 0;
        while (index < count) {
            cval = text[index++];
            // Translate non-breaking space to space.
            if (cval == 0xA0) {
                cval = ' ';
            }
            if (cval == '\n') {
                maxX = MATH::MATH_MAX(maxX, wacc);
                wacc = 0;
                lines++;
                continue;
            }
            else if (cval == '&' && index < count && text[index] != '&') {
                // Ignore lone ampersands
                continue;
            }
            const AtlasChar *c = atlasfont.getChar(cval);
            if (c) {
                wacc += c->wx * fontscalex;
            }
        }
        
        if (width) *width = MATH::MATH_MAX(wacc, maxX);
        if (height) *height = atlasfont.height * fontscaley * lines;
    }

    void DrawBuffer::measureText(int font, const char *text, float *w, float *h) {
        return measureTextCount(font, text, (int)strlen(text), w, h);
    }

    void DrawBuffer::drawTextShadow(int font, const char *text, float x, float y, GRAPH::Color4B color, int flags) {
        uint32 alpha = (color >> 1) & 0xFF000000;
        drawText(font, text, x + 2, y + 2, alpha, flags);
        drawText(font, text, x, y, color, flags);
    }

    void DrawBuffer::doAlign(int flags, float *x, float *y, float *w, float *h) {
        if (flags & ALIGN_HCENTER) *x -= *w / 2;
        if (flags & ALIGN_RIGHT) *x -= *w;
        if (flags & ALIGN_VCENTER) *y -= *h / 2;
        if (flags & ALIGN_BOTTOM) *y -= *h;
        if (flags & (ROTATE_90DEG_LEFT | ROTATE_90DEG_RIGHT))
        {
            std::swap(*w, *h);
            std::swap(*x, *y);
        }
    }


    // TODO: Actually use the rect properly, take bounds.
    void DrawBuffer::drawTextRect(int font, const char *text, float x, float y, float w, float h, GRAPH::Color4B color, int align) {
        if (align & ALIGN_HCENTER)
        {
            x += w / 2;
        }
        else if (align & ALIGN_RIGHT)
        {
            x += w;
        }
        if (align & ALIGN_VCENTER)
        {
            y += h / 2;
        }
        else if (align & ALIGN_BOTTOM)
        {
            y += h;
        }

        drawText(font, text, x, y, color, align);
    }

    // ROTATE_* doesn't yet work right.
    void DrawBuffer::drawText(int font, const char *text, float x, float y, GRAPH::Color4B color, int align) {
        // rough estimate
        if (count_ + strlen(text) * 6 > MAX_VERTS) {
            flush(true);
        }

        const AtlasFont &atlasfont = *atlas->fonts[font];
        unsigned int cval;
        float w, h;
        measureText(font, text, &w, &h);
        if (align) {
            doAlign(align, &x, &y, &w, &h);
        }

        if (align & ROTATE_90DEG_LEFT) {
            x -= atlasfont.ascend*fontscaley;
            // y += h;
        }
        else
            y += atlasfont.ascend*fontscaley;
        float sx = x;
        size_t count = strlen(text);
        size_t index = 0;
        while (index < count) {
            cval = text[index++];
            // Translate non-breaking space to space.
            if (cval == 0xA0) {
                cval = ' ';
            }
            if (cval == '\n') {
                y += atlasfont.height * fontscaley;
                x = sx;
                continue;
            }
            else if (cval == '&' && index < count && text[index] != '&') {
                // Ignore lone ampersands
                continue;
            }
            const AtlasChar *ch = atlasfont.getChar(cval);
            if (!ch)
                ch = atlasfont.getChar('?');
            if (ch) {
                const AtlasChar &c = *ch;
                float cx1, cy1, cx2, cy2;
                if (align & ROTATE_90DEG_LEFT) {
                    cy1 = y - c.ox * fontscalex;
                    cx1 = x + c.oy * fontscaley;
                    cy2 = y - (c.ox + c.pw) * fontscalex;
                    cx2 = x + (c.oy + c.ph) * fontscaley;
                } else {
                    cx1 = x + c.ox * fontscalex;
                    cy1 = y + c.oy * fontscaley;
                    cx2 = x + (c.ox + c.pw) * fontscalex;
                    cy2 = y + (c.oy + c.ph) * fontscaley;
                }
                v(cx1,	cy1, color, c.sx, c.sy);
                v(cx2,	cy1, color, c.ex, c.sy);
                v(cx2,	cy2, color, c.ex, c.ey);
                v(cx1,	cy1, color, c.sx, c.sy);
                v(cx2,	cy2, color, c.ex, c.ey);
                v(cx1,	cy2, color, c.sx, c.ey);
                if (align & ROTATE_90DEG_LEFT)
                    y -= c.wx * fontscalex;
                else
                    x += c.wx * fontscalex;
            }
        }
    }
}
