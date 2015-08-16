#ifndef UILAYOUTPARMETER_H
#define UILAYOUTPARMETER_H

#include <string>
#include "BASE/HObject.h"

namespace GRAPH
{
    namespace UI
    {
        class Margin
        {
        public:
            float left;
            float top;
            float right;
            float bottom;

        public:
            Margin();
            Margin(float l, float t, float r, float b);
            Margin(const Margin& other);
            Margin& operator= (const Margin& other);

            void setMargin(float l, float t, float r, float b);

            bool equals(const Margin& target) const;

            static const Margin ZERO;
        };

        class LayoutParameter : public HObject
        {
        public:
            enum class Type
            {
                NONE = 0,
                LINEAR,
                RELATIVE
            };
            LayoutParameter() : _margin(Margin())
            {
                _layoutParameterType = Type::NONE;
            }

            virtual ~LayoutParameter(){}

            static LayoutParameter* create();

            void setMargin(const Margin& margin);
            const Margin& getMargin() const;

            Type getLayoutType() const;

            LayoutParameter* clone();

            virtual LayoutParameter* createCloneInstance();

            virtual void copyProperties(LayoutParameter* model);

        protected:
            Margin _margin;
            Type _layoutParameterType;
        };

        class LayoutParameterProtocol
        {
        public:
            virtual ~LayoutParameterProtocol(){}
            virtual LayoutParameter* getLayoutParameter() const= 0;
        };

        class LinearLayoutParameter : public LayoutParameter
        {
        public:
            enum class LinearGravity
            {
                NONE,
                LEFT,
                TOP,
                RIGHT,
                BOTTOM,
                CENTER_VERTICAL,
                CENTER_HORIZONTAL
            };

            LinearLayoutParameter()
            : _linearGravity(LinearGravity::NONE)
            {
                _layoutParameterType = Type::LINEAR;
            }

            virtual ~LinearLayoutParameter(){}

            static LinearLayoutParameter* create();

            void setGravity(LinearGravity gravity);
            LinearGravity getGravity() const;

            virtual LayoutParameter* createCloneInstance() override;
            virtual void copyProperties(LayoutParameter* model) override;

        protected:
            LinearGravity _linearGravity;
            int i;
        };

        class RelativeLayoutParameter : public LayoutParameter
        {
        public:
            enum class RelativeAlign
            {
                NONE,
                PARENT_TOP_LEFT,
                PARENT_TOP_CENTER_HORIZONTAL,
                PARENT_TOP_RIGHT,
                PARENT_LEFT_CENTER_VERTICAL,

                CENTER_IN_PARENT,

                PARENT_RIGHT_CENTER_VERTICAL,
                PARENT_LEFT_BOTTOM,
                PARENT_BOTTOM_CENTER_HORIZONTAL,
                PARENT_RIGHT_BOTTOM,

                LOCATION_ABOVE_LEFTALIGN,
                LOCATION_ABOVE_CENTER,
                LOCATION_ABOVE_RIGHTALIGN,
                LOCATION_LEFT_OF_TOPALIGN,
                LOCATION_LEFT_OF_CENTER,
                LOCATION_LEFT_OF_BOTTOMALIGN,
                LOCATION_RIGHT_OF_TOPALIGN,
                LOCATION_RIGHT_OF_CENTER,
                LOCATION_RIGHT_OF_BOTTOMALIGN,
                LOCATION_BELOW_LEFTALIGN,
                LOCATION_BELOW_CENTER,
                LOCATION_BELOW_RIGHTALIGN
            };

            RelativeLayoutParameter()
            : _relativeAlign(RelativeAlign::NONE),
            _relativeWidgetName(""),
            _relativeLayoutName(""),
            _put(false)
            {
                _layoutParameterType = Type::RELATIVE;
            }

            virtual ~RelativeLayoutParameter(){}

            static RelativeLayoutParameter* create();

            void setAlign(RelativeAlign align);
            RelativeAlign getAlign() const;

            void setRelativeToWidgetName(const std::string& name);
            const std::string& getRelativeToWidgetName() const;

            void setRelativeName(const std::string& name);
            const std::string& getRelativeName() const;

            //override functions.
            virtual LayoutParameter* createCloneInstance() override;
            virtual void copyProperties(LayoutParameter* model) override;

        protected:
            RelativeAlign _relativeAlign;
            std::string _relativeWidgetName;
            std::string _relativeLayoutName;
            bool _put;
            friend class RelativeLayoutManager;
        };
    }
}

#endif // UILAYOUTPARMETER_H
