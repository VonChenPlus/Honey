#include "SGF/GFX/TextDrawer.h"
using THIN3D::Thin3DContext;
using THIN3D::LINEAR2D;
using THIN3D::RGBA4444;
#include "UTILS/HASH/Hash.h"
using UTILS::HASH::Fletcher;
using MATH::Bounds;
using THIN3D::Thin3DTexture;

namespace GFX
{
    TextDrawer::TextDrawer(Thin3DContext *thin3d) : thin3d_(thin3d), ctx_(NULLPTR) {
        fontScaleX_ = 1.0f;
        fontScaleY_ = 1.0f;
        frameCount_ = 0;
        UNUSED(ctx_);
    }

    TextDrawer::~TextDrawer() {
        for (auto iter = cache_.begin(); iter != cache_.end(); ++iter)
        {
            iter->second->texture->release();
            delete iter->second;
        }
        cache_.clear();
    }

    uint32 TextDrawer::setFont(const char *fontName, int size, int flags) {
        // We will only use the default font
        uint32 fontHash = Fletcher((const uint8 *)fontName, strlen(fontName));
        fontHash ^= size;
        fontHash ^= flags << 10;

        auto iter = fontMap_.find(fontHash);
        if (iter != fontMap_.end()) {
            fontHash_ = fontHash;
            return fontHash;
        }

        fontMap_[fontHash] = setFont(size);
        fontHash_ = fontHash;
        return fontHash;
    }

    void TextDrawer::setFontScale(float xscale, float yscale){
        fontScaleX_ = xscale;
        fontScaleY_ = yscale;
    }

    void TextDrawer::measureString(const char *str, float *w, float *h) {
        measureString(fontMap_.find(fontHash_)->second, str, w, h);
        *w = (float)*w * fontScaleX_;
        *h = (float)*h * fontScaleY_;
    }

    void TextDrawer::drawString(DrawBuffer &target, const char *str, float x, float y, uint32 color, int align) {
        if (!strlen(str))
            return;

        uint32 stringHash = Fletcher((const uint8 *)str, strlen(str));
        uint32 entryHash = stringHash ^ fontHash_;

        target.flush(true);

        TextStringEntry *entry;

        auto iter = cache_.find(entryHash);
        if (iter != cache_.end()) {
            entry = iter->second;
            entry->lastUsedFrame = frameCount_;
            thin3d_->setTexture(0, entry->texture);
        }
        else {
            drawString(fontMap_.find(fontHash_)->second, str, color, &entry);
            entry->lastUsedFrame = frameCount_;
            cache_[entryHash] = entry;
        }
        float w = entry->bmWidth * fontScaleX_;
        float h = entry->bmHeight * fontScaleY_;
        DrawBuffer::doAlign(align, &x, &y, &w, &h);
        target.drawTexRect(x, y, x + w, y + h, 0.0f, 0.0f, 1.0f, 1.0f, color);
        target.flush(true);
    }

    void TextDrawer::drawStringRect(DrawBuffer &target, const char *str, const Bounds &bounds, uint32 color, int align) {
        float x = bounds.x;
        float y = bounds.y;
        if (align & ALIGN_HCENTER) {
            x = bounds.centerX();
        }
        else if (align & ALIGN_RIGHT) {
            x = bounds.x2();
        }
        if (align & ALIGN_VCENTER) {
            y = bounds.centerY();
        }
        else if (align & ALIGN_BOTTOM) {
            y = bounds.y2();
        }

        drawString(target, str, x, y, color, align);
    }

    void TextDrawer::oncePerFrame()
    {
        frameCount_++;
        // Use a prime number to reduce clashing with other rhythms
        if (frameCount_ % 23 == 0) {
            for (auto iter = cache_.begin(); iter != cache_.end();) {
                if (frameCount_ - iter->second->lastUsedFrame > 100) {
                    if (iter->second->texture)
                        iter->second->texture->release();
                    delete iter->second;
                    cache_.erase(iter++);
                }
                else {
                    iter++;
                }
            }
        }
    }

    Thin3DTexture *TextDrawer::createTexture(uint16 *bitmapData, int width, int height) {
        Thin3DTexture *texture = thin3d_->createTexture(LINEAR2D, RGBA4444, width, height, 1, 0);
        texture->setImageData(0, 0, 0, width, height, 1, 0, width * 2, (const uint8 *)bitmapData);
        texture->finalize(0);
        return texture;
    }
}

