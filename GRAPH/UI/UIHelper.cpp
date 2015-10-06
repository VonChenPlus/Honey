#include "GRAPH/UI/UIHelper.h"
#include "GRAPH/UI/UIWidget.h"
#include "GRAPH/UI/UILayout.h"

namespace GRAPH
{
    namespace UI
    {
        static bool _activeLayout = true;

        Widget* Helper::seekWidgetByTag(Widget* root, int tag)
        {
            if (!root)
            {
                return nullptr;
            }
            if (root->getTag() == tag)
            {
                return root;
            }
            const auto& arrayRootChildren = root->getChildren();
            int64 length = arrayRootChildren.size();
            for (int64 i=0;i<length;i++)
            {
                Widget* child = dynamic_cast<Widget*>(arrayRootChildren.at(i));
                if (child)
                {
                    Widget* res = seekWidgetByTag(child,tag);
                    if (res != nullptr)
                    {
                        return res;
                    }
                }
            }
            return nullptr;
        }

        Widget* Helper::seekWidgetByName(Widget* root, const std::string& name)
        {
            if (!root)
            {
                return nullptr;
            }
            if (root->getName() == name)
            {
                return root;
            }
            const auto& arrayRootChildren = root->getChildren();
            for (auto& subWidget : arrayRootChildren)
            {
                Widget* child = dynamic_cast<Widget*>(subWidget);
                if (child)
                {
                    Widget* res = seekWidgetByName(child,name);
                    if (res != nullptr)
                    {
                        return res;
                    }
                }
            }
            return nullptr;
        }

        /*temp action*/
        Widget* Helper::seekActionWidgetByActionTag(Widget* root, int tag)
        {
            if (!root)
            {
                return nullptr;
            }
            if (root->getActionTag() == tag)
            {
                return root;
            }
            const auto& arrayRootChildren = root->getChildren();
            for (auto& subWidget : arrayRootChildren)
            {
                Widget* child = dynamic_cast<Widget*>(subWidget);
                if (child)
                {
                    Widget* res = seekActionWidgetByActionTag(child,tag);
                    if (res != nullptr)
                    {
                        return res;
                    }
                }
            }
            return nullptr;
        }

        std::string Helper::getSubStringOfUTF8String(const std::string& str, std::string::size_type start, std::string::size_type length)
        {
            if (length==0)
            {
                return "";
            }
            std::string::size_type c, i, ix, q, min=std::string::npos, max=std::string::npos;
            for (q=0, i=0, ix=str.length(); i < ix; i++, q++)
            {
                if (q==start)
                {
                    min = i;
                }
                if (q <= start+length || length==std::string::npos)
                {
                    max = i;
                }

                c = (unsigned char) str[i];

                if      (c<=127) i+=0;
                else if ((c & 0xE0) == 0xC0) i+=1;
                else if ((c & 0xF0) == 0xE0) i+=2;
                else if ((c & 0xF8) == 0xF0) i+=3;
                else return "";//invalid utf8
            }
            if (q <= start+length || length == std::string::npos)
            {
                max = i;
            }
            if (min==std::string::npos || max==std::string::npos)
            {
                return "";
            }
            return str.substr(min,max);
        }

        void Helper::changeLayoutSystemActiveState(bool bActive)
        {
            _activeLayout = bActive;
        }

        void Helper::doLayout(Node *rootNode)
        {
            if(!_activeLayout)
            {
                return;
            }

            for(auto& node : rootNode->getChildren())
            {
                auto com = node->getComponent(__LAYOUT_COMPONENT_NAME);
                Node *parent = node->getParent();
                if (nullptr != com && nullptr != parent) {
                    LayoutComponent* layoutComponent = (LayoutComponent*)com;

                    layoutComponent->refreshLayout();
                }
            }
        }

        MATH::Rectf Helper::restrictCapInsetRect(const MATH::Rectf &capInsets, const MATH::Sizef& textureSize )
        {
            float x = capInsets.origin.x;
            float y = capInsets.origin.y;
            float width = capInsets.size.width;
            float height = capInsets.size.height;

            if (textureSize.width < width)
            {
                x = 0.0f;
                width = 0.0f;
            }
            if (textureSize.height < height)
            {
                y = 0.0f;
                height = 0.0f;
            }
            return MATH::Rectf(x, y, width, height);
        }
    }
}
