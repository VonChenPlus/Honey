#ifndef UIHELPER_H
#define UIHELPER_H

#include <string>
#include "GRAPH/Node.h"

namespace GRAPH
{
    namespace UI
    {
        class Widget;

        class Helper
        {
        public:
            /**
             * Find a widget with a specific tag from root widget.
             * This search will be recursive throught all child widgets.
             * @param root      The be seached root widget.
             * @param tag       The widget tag.
             * @return Widget instance pointer.
             */
            static Widget* seekWidgetByTag(Widget* root, int tag);

            /**
             * Find a widget with a specific name from root widget.
             * This search will be recursive throught all child widgets.
             *
             * @param root      The be searched root widget.
             * @param name      The widget name.
             * @return Widget isntance pointer.
             */
            static Widget* seekWidgetByName(Widget* root, const std::string& name);

            /**
             * Find a widget with a specific action tag from root widget
             * This search will be recursive throught all child widgets.
             *@param root The be searched root widget.
             *@param tag The widget action's tag.
             *@return Widget instance pointer.
             */
            static Widget* seekActionWidgetByActionTag(Widget* root, int tag);

            /**
             * @brief Get a UTF8 substring from a std::string with a given start position and length
             *  Sample:  std::string str = "中国中国中国";  substr = getSubStringOfUTF8String(str,0,2) will = "中国"
             *
             * @param str The source string.
             * @param start The start position of the substring.
             * @param length The length of the substring in UTF8 count
             * @return a UTF8 substring
             * @js NA
             */
            static std::string getSubStringOfUTF8String(const std::string& str,
                                           std::string::size_type start,
                                           std::string::size_type length);

            /**
             * Refresh object and it's children layout state
             *
             *@param rootNode   A Node* or Node* descendant instance pointer.
             *
             */
            static void doLayout(Node *rootNode);

            /**
             *  Change the active property of Layout's @see `LayoutComponent`
             *@param active A boolean value.
             */
            static void changeLayoutSystemActiveState(bool active);

            /**
             *@brief  restrict capInsetSize, when the capInsets's width is larger than the textureSize, it will restrict to 0,
             *        the height goes the same way as width.
             *@param  capInsets A user defined capInsets.
             *@param  textureSize  The size of a scale9enabled texture
             *@return a restricted capInset.
             */
            static MATH::Rectf restrictCapInsetRect(const MATH::Rectf& capInsets, const MATH::Sizef& textureSize);
        };
    }
}

#endif // UIHELPER_H
