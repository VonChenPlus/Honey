#include "GLExtensions.h"

#include <string>
#include <string.h>

#include "BASE/NativeApp.h"

extern std::string System_GetProperty(SystemProperty prop);

namespace GLOBAL
{
    extern GFX::GLExtensions &glExtensions();
}

namespace GFX
{
    bool GLExtensions::versionGEThan(int major, int minor, int sub) {
        if (GLOBAL::glExtensions().ver[0] > major)
            return true;
        if (GLOBAL::glExtensions().ver[0] < major)
            return false;
        if (GLOBAL::glExtensions().ver[1] > minor)
            return true;
        if (GLOBAL::glExtensions().ver[1] < minor)
            return false;
        return GLOBAL::glExtensions().ver[2] >= sub;
    }

    void ProcessGPUFeatures() {
        GLOBAL::glExtensions().bugs = 0;

        //ILOG("Checking for GL driver bugs... vendor=%i model='%s'", (int)gl_extensions.gpuVendor, gl_extensions.model);
        // Should be table driven instead, this is a quick hack for Galaxy Y
        if (System_GetProperty(SYSPROP_NAME) == "samsung:GT-S5360") {
            GLOBAL::glExtensions().bugs |= BUG_FBO_UNUSABLE;
        }

        if (GLOBAL::glExtensions().gpuVendor == GPU_VENDOR_POWERVR) {
            if (!strcmp(GLOBAL::glExtensions().model, "PowerVR SGX 543") ||
                  !strcmp(GLOBAL::glExtensions().model, "PowerVR SGX 540") ||
                  !strcmp(GLOBAL::glExtensions().model, "PowerVR SGX 530") ||
                    !strcmp(GLOBAL::glExtensions().model, "PowerVR SGX 520") ) {
                //WLOG("GL DRIVER BUG: PVR with bad and terrible precision");
                GLOBAL::glExtensions().bugs |= BUG_PVR_SHADER_PRECISION_TERRIBLE | BUG_PVR_SHADER_PRECISION_BAD;
            }
            else {
                //WLOG("GL DRIVER BUG: PVR with bad precision");
                GLOBAL::glExtensions().bugs |= BUG_PVR_SHADER_PRECISION_BAD;
            }
        }
    }
}

