#ifndef LABEL_H
#define LABEL_H

#include "GRAPH/BASE/Node.h"
#include "GRAPH/RENDERER/RenderCommand.h"
#include "GRAPH/BASE/Protocols.h"

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
        static const int DistanceFieldFontSize;

        static Label* create();
        static Label* createWithSystemFont(const std::string& text, const std::string& font, float fontSize,
            const MATH::Sizef& dimensions = MATH::SizefZERO, TextHAlignment hAlignment = TextHAlignment::LEFT,
            TextVAlignment vAlignment = TextVAlignment::TOP);
        static Label * createWithTTF(const std::string& text, const std::string& fontFilePath, float fontSize,
            const MATH::Sizef& dimensions = MATH::SizefZERO, TextHAlignment hAlignment = TextHAlignment::LEFT,
            TextVAlignment vAlignment = TextVAlignment::TOP);
        static Label* createWithTTF(const TTFConfig& ttfConfig, const std::string& text, TextHAlignment hAlignment = TextHAlignment::LEFT, int maxLineWidth = 0);
        static Label* createWithBMFont(const std::string& bmfontPath, const std::string& text,
            const TextHAlignment& hAlignment = TextHAlignment::LEFT, int maxLineWidth = 0,
            const MATH::Vector2f& imageOffset = MATH::Vec2fZERO);
        static Label * createWithCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap);
        static Label * createWithCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap);
        static Label * createWithCharMap(const std::string& plistFile);

        virtual bool setTTFConfig(const TTFConfig& ttfConfig);
        virtual const TTFConfig& getTTFConfig() const { return _fontConfig;}

        virtual bool setBMFontFilePath(const std::string& bmfontFilePath, const MATH::Vector2f& imageOffset = MATH::Vec2fZERO);
        const std::string& getBMFontFilePath() const { return _bmFontPath;}

        virtual bool setCharMap(const std::string& charMapFile, int itemWidth, int itemHeight, int startCharMap);
        virtual bool setCharMap(Texture2D* texture, int itemWidth, int itemHeight, int startCharMap);
        virtual bool setCharMap(const std::string& plistFile);

        virtual void setSystemFontName(const std::string& font);
        virtual const std::string& getSystemFontName() const { return _systemFont;}
        virtual void setSystemFontSize(float fontSize);
        virtual float getSystemFontSize() const { return _systemFontSize;}

        virtual void requestSystemFontRefresh() { _systemFontDirty = true;}

        virtual void setString(const std::string& text) override;
        virtual const std::string& getString() const override {  return _originalUTF8String; }

        int getStringNumLines() const;

        int getStringLength() const;

        virtual void setTextColor(const Color4B &color);
        const Color4B& getTextColor() const { return _textColor;}

        virtual void enableShadow(const Color4B& shadowColor = Color4B::BLACK,const MATH::Sizef &offset = MATH::Sizef(2,-2), int blurRadius = 0);
        virtual void enableOutline(const Color4B& outlineColor,int outlineSize = -1);
        virtual void enableGlow(const Color4B& glowColor);
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

        /**
         * Returns the line height of this Label.
         * @warning Not support system font.
         * @since v3.2.0
         */
        float getLineHeight() const;

        /**
         * Sets the additional kerning of the Label.
         *
         * @warning Not support system font.
         * @since v3.2.0
         */
        void setAdditionalKerning(float space);

        /**
         * Returns the additional kerning of the Label.
         *
         * @warning Not support system font.
         * @since v3.2.0
         */
        float getAdditionalKerning() const;

        FontAtlas* getFontAtlas() { return _fontAtlas; }

        virtual const BlendFunc& getBlendFunc() const override { return _blendFunc; }
        virtual void setBlendFunc(const BlendFunc &blendFunc) override;

        virtual bool isOpacityModifyRGB() const override;
        virtual void setOpacityModifyRGB(bool isOpacityModifyRGB) override;
        virtual void updateDisplayedColor(const Color3B& parentColor) override;
        virtual void updateDisplayedOpacity(GLubyte parentOpacity) override;

        virtual void setScale(float scale) override;
        virtual void setScaleX(float scaleX) override;
        virtual void setScaleY(float scaleY) override;
        virtual float getScaleX() const override;
        virtual float getScaleY() const override;

        virtual const MATH::Sizef& getContentSize() const override;

        virtual MATH::Rectf getBoundingBox() const override;

        virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;
        virtual void draw(Renderer *renderer, const MATH::Matrix4 &transform, uint32_t flags) override;

        virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

        virtual void removeAllChildrenWithCleanup(bool cleanup) override;
        virtual void removeChild(Node* child, bool cleanup = true) override;

    public:
        /**
         * Constructor of Label.
         * @js NA
         */
        Label(FontAtlas *atlas = nullptr, TextHAlignment hAlignment = TextHAlignment::LEFT,
          TextVAlignment vAlignment = TextVAlignment::TOP,bool useDistanceField = false,bool useA8Shader = false);

        /**
         * Destructor of Label.
         * @js NA
         * @lua NA
         */
        virtual ~Label();

    protected:
        void onDraw(const MATH::Matrix4& transform, bool transformUpdated);
        void onDrawShadow(GLProgram* glProgram);

        struct LetterInfo
        {
            FontLetterDefinition def;

            MATH::Vector2f position;
            MATH::Sizef  contentSize;
            int   atlasIndex;
        };
        enum class LabelType {

            TTF,
            BMFONT,
            CHARMAP,
            STRING_TEXTURE
        };

        virtual void setFontAtlas(FontAtlas* atlas,bool distanceFieldEnabled = false, bool useA8Shader = false);

        bool recordLetterInfo(const MATH::Vector2f& point,const FontLetterDefinition& letterDef, int spriteIndex);
        bool recordPlaceholderInfo(int spriteIndex);

        void setFontScale(float fontScale);

        virtual void alignText();

        bool computeHorizontalKernings(const std::u16string& stringToRender);

        void computeStringNumLines();

        void updateQuads();

        virtual void updateColor() override;

        virtual void updateShaderProgram();

        void createSpriteForSystemFont();

        void createShadowSpriteForSystemFont();

        void reset();

        void drawSelf(Renderer* renderer, uint32_t flags);

        std::string _bmFontPath;

        bool _isOpacityModifyRGB;
        bool _contentDirty;

        bool _systemFontDirty;
        std::string _systemFont;
        float         _systemFontSize;
        LabelType _currentLabelType;

        HObjectVector<SpriteBatchNode*> _batchNodes;
        FontAtlas *                   _fontAtlas;
        std::vector<LetterInfo>       _lettersInfo;
        EventListenerCustom* _purgeTextureListener;
        EventListenerCustom* _resetTextureListener;

        TTFConfig _fontConfig;

        //compatibility with older LabelTTF
        Sprite* _textSprite;
        FontDefinition _fontDefinition;
        bool  _compatibleMode;

        //! used for optimization
        Sprite *_reusedLetter;
        MATH::Rectf _reusedRect;
        int _limitShowCount;

        float _additionalKerning;
        float _commonLineHeight;
        bool  _lineBreakWithoutSpaces;
        int * _horizontalKernings;

        float _maxLineWidth;
        MATH::Sizef  _labelDimensions;
        float _labelWidth;
        float _labelHeight;
        TextHAlignment _hAlignment;
        TextVAlignment _vAlignment;

        int           _currNumLines;
        std::u16string _currentUTF16String;
        std::string          _originalUTF8String;

        float _fontScale;

        bool _useDistanceField;
        bool _useA8Shader;

        LabelEffect _currLabelEffect;
        Color4B _effectColor;
        Color4F _effectColorF;

        GLuint _uniformEffectColor;
        GLuint _uniformTextColor;
        CustomCommand _customCommand;

        bool    _shadowDirty;
        bool    _shadowEnabled;
        MATH::Sizef    _shadowOffset;
        int     _shadowBlurRadius;
        MATH::Matrix4  _shadowTransform;
        Color4F _shadowColor4F;
        Color3B _shadowColor3B;
        GLubyte _shadowOpacity;
        Sprite*   _shadowNode;

        int     _outlineSize;

        Color4B _textColor;
        Color4F _textColorF;

        bool _clipEnabled;
        bool _blendFuncDirty;
        BlendFunc _blendFunc;
        /// whether or not the sprite was inside bounds the previous frame
        bool _insideBounds;

        std::unordered_map<int, Sprite*> _letters;

    private:
        DISALLOW_COPY_AND_ASSIGN(Label)

        friend class LabelTextFormatter;
    };
}

#endif // LABEL_H
