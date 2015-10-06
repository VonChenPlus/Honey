#ifndef LABEL_H
#define LABEL_H

#include "GRAPH/Node.h"
#include "GRAPH/Protocols.h"
#include "GRAPH/Fonts.h"
#include "GRAPH/RENDERER/RenderCommand.h"

namespace GRAPH
{
    class Sprite;
    class SpriteBatchNode;

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
        static Label* createWithSystemFont(const std::string& text, const std::string& font, float fontSize,
            const MATH::Sizef& dimensions = MATH::SizefZERO, TextHAlignment hAlignment = TextHAlignment::LEFT,
            TextVAlignment vAlignment = TextVAlignment::TOP);

        virtual void setSystemFontName(const std::string& font);
        virtual const std::string& getSystemFontName() const { return _systemFont;}

        virtual void setSystemFontSize(float fontSize);
        virtual float getSystemFontSize() const { return _systemFontSize;}

        virtual void requestSystemFontRefresh() { _systemFontDirty = true;}

        virtual void setString(const std::string& text) override;
        virtual const std::string& getString() const override {  return _utf8Text; }

        int getStringNumLines();
        int getStringLength();

        virtual void setTextColor(const Color4B &color);
        const Color4B& getTextColor() const { return _textColor;}

        virtual void enableShadow(const Color4B& shadowColor = Color4B::BLACK, const MATH::Sizef &offset = MATH::Sizef(2,-2));
        virtual void enableOutline(const Color4B& outlineColor,int outlineSize = -1);

        virtual void disableEffect();
        virtual void disableEffect(LabelEffect effect);

        void setAlignment(TextHAlignment hAlignment) { setAlignment(hAlignment,_vAlignment);}

        TextHAlignment getTextAlignment() const { return _hAlignment;}
        void setAlignment(TextHAlignment hAlignment,TextVAlignment vAlignment);

        void setHorizontalAlignment(TextHAlignment hAlignment) { setAlignment(hAlignment,_vAlignment); }
        TextHAlignment getHorizontalAlignment() const { return _hAlignment; }

        void setVerticalAlignment(TextVAlignment vAlignment) { setAlignment(_hAlignment,vAlignment); }
        TextVAlignment getVerticalAlignment() const { return _vAlignment; }

        void setLineBreakWithoutSpace(bool breakWithoutSpace);

        void setMaxLineWidth(float maxLineWidth);
        float getMaxLineWidth() { return _maxLineWidth; }

        void setWidth(float width) { setDimensions(width,_labelHeight);}
        float getWidth() const { return _labelWidth; }

        void setHeight(float height){ setDimensions(_labelWidth, height); }
        float getHeight() const { return _labelHeight; }

        void setDimensions(float width, float height);
        const MATH::Sizef& getDimensions() const{ return _labelDimensions;}

        virtual void updateContent();

        virtual Sprite * getLetter(int lettetIndex);

        void setClipMarginEnabled(bool clipEnabled) { _clipEnabled = clipEnabled; }

        bool isClipMarginEnabled() const { return _clipEnabled; }

        void setLineHeight(float height);
        float getLineHeight() const;

        void setAdditionalKerning(float space);
        float getAdditionalKerning() const;

        FontAtlas* getFontAtlas() { return _fontAtlas; }

        virtual const BlendFunc& getBlendFunc() const override { return _blendFunc; }
        virtual void setBlendFunc(const BlendFunc &blendFunc) override;

        virtual bool isOpacityModifyRGB() const override { return _isOpacityModifyRGB; }
        virtual void setOpacityModifyRGB(bool isOpacityModifyRGB) override;
        virtual void updateDisplayedColor(const Color3B& parentColor) override;
        virtual void updateDisplayedOpacity(GLubyte parentOpacity) override;

        virtual const MATH::Sizef& getContentSize() const override;
        virtual MATH::Rectf getBoundingBox() const override;

        virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;
        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

        virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

        virtual void removeAllChildrenWithCleanup(bool cleanup) override;
        virtual void removeChild(Node* child, bool cleanup = true) override;
        virtual void setGlobalZOrder(float globalZOrder) override;

    public:
        Label(TextHAlignment hAlignment = TextHAlignment::LEFT,
          TextVAlignment vAlignment = TextVAlignment::TOP);

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
        void onDrawShadow(GLProgram* glProgram);
        void drawSelf(Renderer* renderer, uint32_t flags);

        bool multilineTextWrapByChar();
        bool multilineTextWrapByWord();

        void updateLabelLetters();
        virtual void alignText();
        void computeAlignmentOffset();
        bool computeHorizontalKernings(const std::u16string& stringToRender);

        void recordLetterInfo(const MATH::Vector2f& point, char16_t utf16Char, int letterIndex, int lineIndex);
        void recordPlaceholderInfo(int letterIndex, char16_t utf16Char);

        void updateQuads();

        void createSpriteForSystemFont(const FontDefinition& fontDef);
        void createShadowSpriteForSystemFont(const FontDefinition& fontDef);

        virtual void updateShaderProgram();

        void reset();

        FontDefinition _getFontDefinition() const;

        virtual void updateColor() override;

        bool _contentDirty;
        std::u16string _utf16Text;
        std::string _utf8Text;
        int _numberOfLines;

        std::string _bmFontPath;
        float _outlineSize;

        bool _systemFontDirty;
        std::string _systemFont;
        float _systemFontSize;
        Sprite* _textSprite;
        Sprite* _shadowNode;

        FontAtlas* _fontAtlas;
        HObjectVector<SpriteBatchNode*> _batchNodes;
        std::vector<LetterInfo> _lettersInfo;

        Sprite *_reusedLetter;
        MATH::Rectf _reusedRect;
        int _lengthOfString;

        float _lineHeight;
        float _additionalKerning;
        int* _horizontalKernings;
        bool _lineBreakWithoutSpaces;
        float _maxLineWidth;
        MATH::Sizef _labelDimensions;
        float _labelWidth;
        float _labelHeight;
        TextHAlignment _hAlignment;
        TextVAlignment _vAlignment;

        float _textDesiredHeight;
        std::vector<float> _linesWidth;
        std::vector<float> _linesOffsetX;
        float _letterOffsetY;
        float _tailoredTopY;
        float _tailoredBottomY;

        LabelEffect _currLabelEffect;
        Color4F _effectColorF;
        Color4B _textColor;
        Color4F _textColorF;

        QuadCommand _quadCommand;
        CustomCommand _customCommand;
        MATH::Matrix4  _shadowTransform;
        GLuint _uniformEffectColor;
        GLuint _uniformTextColor;
        bool _useDistanceField;
        bool _useA8Shader;

        bool _shadowDirty;
        bool _shadowEnabled;
        MATH::Sizef _shadowOffset;

        Color4F _shadowColor4F;
        Color3B _shadowColor3B;
        GLubyte _shadowOpacity;
        float _shadowBlurRadius;

        bool _clipEnabled;
        bool _blendFuncDirty;
        BlendFunc _blendFunc;

        bool _insideBounds;

        bool _isOpacityModifyRGB;

        std::unordered_map<int, Sprite*> _letters;

    private:
        DISALLOW_COPY_AND_ASSIGN(Label)
    };
}

#endif // LABEL_H
