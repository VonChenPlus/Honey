#ifndef UILABEL_H
#define UILABEL_H

#include "GRAPH/Node.h"
#include "GRAPH/Protocols.h"
#include "GRAPH/Fonts.h"
#include "GRAPH/UNITY3D/RenderCommand.h"
#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    class Sprite;
    class SpriteBatchNode;

    namespace UI
    {
        enum class LabelEffect
        {
            NORMAL,
            OUTLINE,
            SHADOW,
            GLOW,
            ALL
        };

        class Label : public Node, public LabelProtocol, public BlendProtocol
        {
        public:
            static Label* create();
            static Label* createWithCustomLoader(const char *string, U3DStringToTexture loader = nullptr, void *loaderOwner = nullptr);

            virtual void requestSystemFontRefresh() { systemFontDirty_ = true;}

            virtual void setString(const std::string& text) override;
            virtual const std::string& getString() const override {  return utf8Text_; }

            int getStringNumLines();
            int getStringLength();

            virtual void setTextColor(const Color4B &color);
            const Color4B& getTextColor() const { return textColor_;}

            virtual void enableShadow(const Color4B& shadowColor = Color4B::BLACK, const MATH::Sizef &offset = MATH::Sizef(2,-2));
            virtual void enableOutline(const Color4B& outlineColor,int outlineSize = -1);

            virtual void disableEffect();
            virtual void disableEffect(LabelEffect effect);

            void setLineBreakWithoutSpace(bool breakWithoutSpace);

            void setMaxLineWidth(float maxLineWidth);
            float getMaxLineWidth() { return maxLineWidth_; }

            void setWidth(float width) { setDimensions(width,labelHeight_);}
            float getWidth() const { return labelWidth_; }

            void setHeight(float height){ setDimensions(labelWidth_, height); }
            float getHeight() const { return labelHeight_; }

            void setDimensions(float width, float height);
            const MATH::Sizef& getDimensions() const{ return labelDimensions_;}

            virtual void updateContent();

            virtual Sprite * getLetter(int lettetIndex);

            void setClipMarginEnabled(bool clipEnabled) { clipEnabled_ = clipEnabled; }

            bool isClipMarginEnabled() const { return clipEnabled_; }

            void setLineHeight(float height);
            float getLineHeight() const;

            void setAdditionalKerning(float space);
            float getAdditionalKerning() const;

            FontAtlas* getFontAtlas() { return fontAtlas_; }

            virtual const BlendFunc& getBlendFunc() const override { return blendFunc_; }
            virtual void setBlendFunc(const BlendFunc &blendFunc) override;

            virtual bool isOpacityModifyRGB() const override { return isOpacityModifyRGB_; }
            virtual void setOpacityModifyRGB(bool isOpacityModifyRGB) override;
            virtual void updateDisplayedColor(const Color3B& parentColor) override;
            virtual void updateDisplayedOpacity(uint8 parentOpacity) override;

            virtual const MATH::Sizef& getContentSize() const override;
            virtual MATH::Rectf getBoundingBox() const override;

            virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;
            virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

            virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

            virtual void removeAllChildrenWithCleanup(bool cleanup) override;
            virtual void removeChild(Node* child, bool cleanup = true) override;
            virtual void setGlobalZOrder(float globalZOrder) override;

        public:
            Label();
            virtual ~Label();

        protected:
            struct LetterInfo
            {
                char16_t utf16Char;
                bool valid;
                float positionX;
                float positionY;
                int atlasIndex;
                int lineIndex;
            };

            virtual void setFontAtlas(FontAtlas* atlas, bool distanceFieldEnabled = false, bool useA8Shader = false);

            void computeStringNumLines();

            void onDraw(const MATH::Matrix4& transform, bool transformUpdated);
            void onDrawShadow(Unity3DShaderSet* u3dShader);
            void drawSelf(Renderer* renderer, uint32_t flags);

            bool multilineTextWrapByChar();
            bool multilineTextWrapByWord();

            void updateLabelLetters();
            virtual void alignText();

            void recordLetterInfo(const MATH::Vector2f& point, char16_t utf16Char, int letterIndex, int lineIndex);
            void recordPlaceholderInfo(int letterIndex, char16_t utf16Char);

            void updateQuads();

            virtual void updateShaderProgram();

            void reset();

            virtual void updateColor() override;

            bool contentDirty_;
            std::u16string utf16Text_;
            std::string utf8Text_;
            int numberOfLines_;

            std::string bmFontPath_;
            float outlineSize_;

            bool systemFontDirty_;
            std::string systemFont_;
            float systemFontSize_;
            Sprite* textSprite_;
            Sprite* shadowNode_;

            FontAtlas* fontAtlas_;
            HObjectVector<SpriteBatchNode*> batchNodes_;
            std::vector<LetterInfo> lettersInfo_;

            Sprite *reusedLetter_;
            MATH::Rectf reusedRect_;
            int lengthOfString_;

            float lineHeight_;
            float additionalKerning_;
            int* horizontalKernings_;
            bool lineBreakWithoutSpaces_;
            float maxLineWidth_;
            MATH::Sizef labelDimensions_;
            float labelWidth_;
            float labelHeight_;

            float textDesiredHeight_;
            std::vector<float> linesWidth_;
            std::vector<float> linesOffsetX_;
            float letterOffsetY_;
            float tailoredTopY_;
            float tailoredBottomY_;

            LabelEffect currLabelEffect_;
            Color4F effectColorF_;
            Color4B textColor_;
            Color4F textColorF_;

            QuadCommand quadCommand_;
            CustomCommand customCommand_;
            MATH::Matrix4  shadowTransform_;
            uint32 uniformEffectColor_;
            uint32 uniformTextColor_;
            bool useDistanceField_;
            bool useA8Shader_;

            bool shadowDirty_;
            bool shadowEnabled_;
            MATH::Sizef shadowOffset_;

            Color4F shadowColor4F_;
            Color3B shadowColor3B_;
            uint8 shadowOpacity_;
            float shadowBlurRadius_;

            bool clipEnabled_;
            bool blendFuncDirty_;
            BlendFunc blendFunc_;

            bool insideBounds_;

            bool isOpacityModifyRGB_;

            U3DStringToTexture stringToTextureLoader_;
            void *stringtoTextureOwner_;

            std::unordered_map<int, Sprite*> letters_;

        private:
            DISALLOW_COPY_AND_ASSIGN(Label)
        };
    }
}

#endif // UILABEL_H
