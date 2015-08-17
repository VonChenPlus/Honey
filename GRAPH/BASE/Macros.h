#ifndef MACROS_H
#define MACROS_H

#include "MATH/MathDef.h"
#include <functional>

namespace GRAPH
{
    #define REPEAT_FOREVER (MATH::MATH_UINT32_MAX() - 1)

    #define CC_CONTENT_SCALE_FACTOR() Director::getInstance()->getContentScaleFactor()

    #define CC_RECT_POINTS_TO_PIXELS(__rect_in_points_points__)                                                                        \
        MATH::Rectf( (__rect_in_points_points__).origin.x * CC_CONTENT_SCALE_FACTOR(), (__rect_in_points_points__).origin.y * CC_CONTENT_SCALE_FACTOR(),    \
                (__rect_in_points_points__).size.width * CC_CONTENT_SCALE_FACTOR(), (__rect_in_points_points__).size.height * CC_CONTENT_SCALE_FACTOR() )
    #define CC_INCREMENT_GL_DRAWS(__n__) Director::getInstance()->getRenderer()->addDrawnBatches(__n__)
    #define CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(__drawcalls__, __vertices__) \
        do {                                                                \
            auto __renderer__ = Director::getInstance()->getRenderer();     \
            __renderer__->addDrawnBatches(__drawcalls__);                   \
            __renderer__->addDrawnVertices(__vertices__);                   \
        } while(0)

    #define CC_CALLBACK_0(__selector__,__target__, ...) std::bind(&__selector__,__target__, ##__VA_ARGS__)
    #define CC_CALLBACK_1(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, ##__VA_ARGS__)
    #define CC_CALLBACK_2(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)
    #define CC_CALLBACK_3(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)

    #define CREATE_FUNC(__TYPE__) \
    static __TYPE__* create() \
    { \
        __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
        if (pRet && pRet->init()) \
        { \
            pRet->autorelease(); \
            return pRet; \
        } \
        else \
        { \
            delete pRet; \
            pRet = NULL; \
            return NULL; \
        } \
    }
}

#endif // MACROS_H

