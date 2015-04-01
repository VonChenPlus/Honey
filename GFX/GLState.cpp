#include "GLState.h"

#ifdef _WIN32
#include "GL/wglew.h"
#endif

#include "GFX/GLExtensions.h"

namespace GFX
{
    OpenGLState glstate;
    GLExtensions gl_extensions;
    std::string g_all_gl_extensions;
    std::string g_all_egl_extensions;

    int OpenGLState::state_count = 0;

    void OpenGLState::initialize()
    {
        if (initialized)
            return;
        initialized = true;

        restore();
    }

    void OpenGLState::restore()
    {
        int count = 0;

        blend.restore(); count++;
        blendEquationSeparate.restore(); count++;
        blendFuncSeparate.restore(); count++;
        blendColor.restore(); count++;

        scissorTest.restore(); count++;
        scissorRect.restore(); count++;

        cullFace.restore(); count++;
        cullFaceMode.restore(); count++;
        frontFace.restore(); count++;

        depthTest.restore(); count++;
        depthRange.restore(); count++;
        depthFunc.restore(); count++;
        depthWrite.restore(); count++;

        colorMask.restore(); count++;
        viewport.restore(); count++;

        stencilTest.restore(); count++;
        stencilOp.restore(); count++;
        stencilFunc.restore(); count++;
        stencilMask.restore(); count++;

        dither.restore(); count++;

        colorLogicOp.restore(); count++;
        logicOp.restore(); count++;

        arrayBuffer.restore(); count++;
        elementArrayBuffer.restore(); count++;

        if (count != state_count)
        {
            //FLOG("OpenGLState::Restore is missing some states");
        }
    }

    void OpenGLState::setVSyncInterval(int interval)
    {
    #ifdef _WIN32
        if (wglSwapIntervalEXT)
            wglSwapIntervalEXT(interval);
    #endif
    }

    // http://stackoverflow.com/questions/16147700/opengl-es-using-tegra-specific-extensions-gl-ext-texture-array

    void CheckGLExtensions()
    {
        // Make sure to only do this once. It's okay to call CheckGLExtensions from wherever.
        static bool done = false;
        if (done)
            return;
        done = true;
        memset(&gl_extensions, 0, sizeof(gl_extensions));

        const char *renderer = (const char *)glGetString(GL_RENDERER);
        const char *versionStr = (const char *)glGetString(GL_VERSION);
        //const char *glslVersionStr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

        // Check vendor string to try and guess GPU
        const char *cvendor = (char *)glGetString(GL_VENDOR);
        // TODO: move this stuff to gpu_features.cpp
        if (cvendor)
        {
            const std::string vendor(cvendor);
            if (vendor == "NVIDIA Corporation"
                || vendor == "Nouveau"
                || vendor == "nouveau")
            {
                gl_extensions.gpuVendor = GPU_VENDOR_NVIDIA;
            }
            else if (vendor == "Advanced Micro Devices, Inc."
                || vendor == "ATI Technologies Inc.")
            {
                gl_extensions.gpuVendor = GPU_VENDOR_AMD;
            }
            else if (vendor == "Intel"
                || vendor == "Intel Inc."
                || vendor == "Intel Corporation"
                || vendor == "Tungsten Graphics, Inc")
            { // We'll assume this last one means Intel
                gl_extensions.gpuVendor = GPU_VENDOR_INTEL;
            }
            else if (vendor == "ARM")
            {
                gl_extensions.gpuVendor = GPU_VENDOR_ARM;
            }
            else if (vendor == "Imagination Technologies")
            {
                gl_extensions.gpuVendor = GPU_VENDOR_POWERVR;
            }
            else if (vendor == "Qualcomm")
            {
                gl_extensions.gpuVendor = GPU_VENDOR_ADRENO;
            }
            else if (vendor == "Broadcom")
            {
                gl_extensions.gpuVendor = GPU_VENDOR_BROADCOM;
                // Just for reference: Galaxy Y has renderer == "VideoCore IV HW"
            }
            else
            {
                gl_extensions.gpuVendor = GPU_VENDOR_UNKNOWN;
            }
        }
        else
        {
            gl_extensions.gpuVendor = GPU_VENDOR_UNKNOWN;
        }

        //ILOG("GPU Vendor : %s ; renderer: %s version str: %s ; GLSL version str: %s", cvendor, renderer ? renderer : "N/A", versionStr ? versionStr : "N/A", glslVersionStr ? glslVersionStr : "N/A");

        if (renderer)
        {
            strncpy(gl_extensions.model, renderer, sizeof(gl_extensions.model));
            gl_extensions.model[sizeof(gl_extensions.model) - 1] = 0;
        }

        char buffer[64] = { 0 };
        if (versionStr)
        {
            strncpy(buffer, versionStr, 63);
        }
        const char *lastNumStart = buffer;
        int numVer = 0;
        int len = (int)strlen(buffer);
        for (int i = 0; i < len && numVer < 3; i++)
        {
            if (buffer[i] == '.')
            {
                buffer[i] = 0;
                gl_extensions.ver[numVer++] = strtol(lastNumStart, NULL, 10);
                i++;
                lastNumStart = buffer + i;
            }
        }
        if (numVer < 3)
            gl_extensions.ver[numVer++] = strtol(lastNumStart, NULL, 10);

        // If the GL version >= 4.3, we know it's a true superset of OpenGL ES 3.0 and can thus enable
        // all the same modern paths.
        // Most of it could be enabled on lower GPUs as well, but let's start this way.
        if (gl_extensions.versionGEThan(4, 3, 0))
        {
            gl_extensions.GLES3 = true;
        }

        const char *extString = (const char *)glGetString(GL_EXTENSIONS);
        if (extString)
        {
            g_all_gl_extensions = extString;
        }
        else
        {
            g_all_gl_extensions = "";
            extString = "";
        }

    #ifdef WIN32
        const char *wglString = 0;
        if (wglGetExtensionsStringEXT)
            wglString = wglGetExtensionsStringEXT();
        if (wglString)
        {
            gl_extensions.EXT_swap_control_tear = strstr(wglString, "WGL_EXT_swap_control_tear") != 0;
            g_all_egl_extensions = wglString;
        }
        else
        {
            g_all_egl_extensions = "";
        }
    #endif

        // Check the desktop extension instead of the OES one. They are very similar.
        // Also explicitly check those ATI devices that claims to support npot
        gl_extensions.OES_texture_npot = strstr(extString, "GL_ARB_texture_non_power_of_two") != 0
            && !(((strncmp(renderer, "ATI RADEON X", 12) == 0) || (strncmp(renderer, "ATI MOBILITY RADEON X", 21) == 0)));

        gl_extensions.NV_draw_texture = strstr(extString, "GL_NV_draw_texture") != 0;
        gl_extensions.ARB_blend_func_extended = strstr(extString, "GL_ARB_blend_func_extended") != 0;
        gl_extensions.ARB_shader_image_load_store = (strstr(extString, "GL_ARB_shader_image_load_store") != 0) || (strstr(extString, "GL_EXT_shader_image_load_store") != 0);
        gl_extensions.EXT_bgra = strstr(extString, "GL_EXT_bgra") != 0;
        gl_extensions.EXT_gpu_shader4 = strstr(extString, "GL_EXT_gpu_shader4") != 0;
        gl_extensions.NV_framebuffer_blit = strstr(extString, "GL_NV_framebuffer_blit") != 0;
        if (gl_extensions.gpuVendor == GPU_VENDOR_INTEL || !gl_extensions.versionGEThan(3, 0, 0)) {
            // Force this extension to off on sub 3.0 OpenGL versions as it does not seem reliable
            // Also on Intel, see https://github.com/hrydgard/ppsspp/issues/4867
            gl_extensions.ARB_blend_func_extended = false;
        }

        // Desktops support minmax and subimage unpack (GL_UNPACK_ROW_LENGTH etc)
        gl_extensions.EXT_blend_minmax = true;
        gl_extensions.EXT_unpack_subimage = true;

        // GLES 3 subsumes many ES2 extensions.
        if (gl_extensions.GLES3)
        {
            gl_extensions.EXT_unpack_subimage = true;
        }

        if (strstr(extString, "GL_ARB_ES2_compatibility"))
        {
            const GLint precisions[6] =
            {
                GL_LOW_FLOAT, GL_MEDIUM_FLOAT, GL_HIGH_FLOAT,
                GL_LOW_INT, GL_MEDIUM_INT, GL_HIGH_INT
            };
            GLint shaderTypes[2] =
            {
                GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
            };
            for (int st = 0; st < 2; st++)
            {
                for (int p = 0; p < 6; p++)
                {
                    glGetShaderPrecisionFormat(shaderTypes[st], precisions[p], gl_extensions.range[st][p], &gl_extensions.precision[st][p]);
                }
            }
        }

        gl_extensions.FBO_ARB = false;
        gl_extensions.FBO_EXT = false;
        gl_extensions.PBO_ARB = true;
        gl_extensions.PBO_NV = true;
        if (strlen(extString) != 0)
        {
            gl_extensions.FBO_ARB = strstr(extString, "GL_ARB_framebuffer_object") != 0;
            gl_extensions.FBO_EXT = strstr(extString, "GL_EXT_framebuffer_object") != 0;
            gl_extensions.PBO_ARB = strstr(extString, "GL_ARB_pixel_buffer_object") != 0;
            gl_extensions.PBO_NV = strstr(extString, "GL_NV_pixel_buffer_object") != 0;
        }

        ProcessGPUFeatures();

        //int error = glGetError();
        //if (error)
            //ELOG("GL error in init: %i", error);
    }
}
