#include "GRAPH/RENDERER/RenderCommand.h"
#include "GRAPH/BASE/Node.h"

namespace GRAPH
{
    RenderCommand::RenderCommand()
    : _type(RenderCommand::Type::UNKNOWN_COMMAND)
    , _globalOrder(0)
    , _isTransparent(true)
    , _skipBatching(false)
    , _is3D(false)
    , _depth(0)
    {
    }

    RenderCommand::~RenderCommand()
    {
    }

    void RenderCommand::init(float globalZOrder, const MATH::Matrix4 &transform, uint32_t flags)
    {
        _globalOrder = globalZOrder;
        if (flags & Node::FLAGS_RENDER_AS_3D)
        {
            if (Camera::getVisitingCamera())
                _depth = Camera::getVisitingCamera()->getDepthInView(transform);

            set3D(true);
        }
        else
        {
            set3D(false);
            _depth = 0;
        }
    }

    void printBits(ssize_t const size, void const * const ptr)
    {
        unsigned char *b = (unsigned char*) ptr;
        unsigned char byte;
        ssize_t i, j;

        for (i=size-1;i>=0;i--)
        {
            for (j=7;j>=0;j--)
            {
                byte = b[i] & (1<<j);
                byte >>= j;
                printf("%u", byte);
            }
        }
        puts("");
    }

    void RenderCommand::printID()
    {
        printf("Command Depth: %f\n", _globalOrder);
    }
}
