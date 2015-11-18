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
            static Widget* seekWidgetByTag(Widget* root, int tag);

            static Widget* seekWidgetByName(Widget* root, const std::string& name);

            static Widget* seekActionWidgetByActionTag(Widget* root, int tag);

            static void doLayout(Node *rootNode);

            static void changeLayoutSystemActiveState(bool active);

            static MATH::Rectf restrictCapInsetRect(const MATH::Rectf& capInsets, const MATH::Sizef& textureSize);
        };
    }
}

#endif // UIHELPER_H
