#include "GfxResourceHolder.h"

#include <vector>

#include "BASE/BasicTypes.h"
#include "UTILS/STRING/String.h"
using UTILS::STRING::StringFromFormat;

namespace GFX
{
    std::vector<GfxResourceHolder *> *holders;

    static bool inLost;

    void register_gl_resource_holder(GfxResourceHolder *holder) {
        if (inLost) {
            //FLOG("BAD: Should not call register_gl_resource_holder from lost path");
            return;
        }
        if (holders) {
            holders->push_back(holder);
        }
        else {
            //WLOG("GL resource holder not initialized, cannot register resource");
        }
    }

    void unregister_gl_resource_holder(GfxResourceHolder *holder) {
        if (inLost) {
            //FLOG("BAD: Should not call unregister_gl_resource_holder from lost path");
            return;
        }
        if (holders) {
            for (Size i = 0; i < holders->size(); i++) {
                if ((*holders)[i] == holder) {
                    holders->erase(holders->begin() + i);
                    return;
                }
            }
            //WLOG("unregister_gl_resource_holder: Resource not registered");
        }
        else {
            //WLOG("GL resource holder not initialized or already shutdown, cannot unregister resource");
        }
    }

    void gl_lost() {
        inLost = true;
        if (!holders) {
            //WLOG("GL resource holder not initialized, cannot process lost request");
            inLost = false;
            return;
        }

        // TODO: We should really do this when we get the context back, not during gl_lost...
        //ILOG("gl_lost() restoring %i items:", (int)holders->size());
        for (Size i = 0; i < holders->size(); i++) {
            //ILOG("GLLost(%i / %i, %p)", (int)(i + 1), (int) holders->size(), (*holders)[i]);
            (*holders)[i]->glLost();
        }
        //ILOG("gl_lost() completed restoring %i items:", (int)holders->size());
        inLost = false;
    }

    void gl_lost_manager_init() {
        if (holders) {
            //FLOG("Double GL lost manager init");
            // Dead here (FLOG), no need to delete holders
        }
        holders = new std::vector<GfxResourceHolder *>();
    }

    void gl_lost_manager_shutdown() {
        if (!holders) {
            //FLOG("Lost manager already shutdown");
        }
        else if (holders->size() > 0) {
            throw _NException_(StringFromFormat("Lost manager shutdown with %i objects still registered", (int)holders->size()), NException::GFX);
        }

        delete holders;
        holders = 0;
    }
}

