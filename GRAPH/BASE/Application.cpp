#include <algorithm>
#include "GRAPH/BASE/Application.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/BASE/GLView.h"
#include "IO/FileUtils.h"

namespace GRAPH
{
    // sharedApplication pointer
    Application * Application::sm_pSharedApplication = 0;

    Application::Application()
    {
        sm_pSharedApplication = this;
    }

    Application::~Application()
    {
        sm_pSharedApplication = nullptr;
    }

    int Application::run()
    {
        initGLContextAttrs();

        // Initialize instance and cocos2d.
        if (!applicationDidFinishLaunching())
        {
            return 1;
        }

        auto director = Director::getInstance();
        auto glview = director->getOpenGLView();

        // Retain glview to avoid glview being released in the while loop
        glview->retain();

        while(!glview->windowShouldClose())
        {
            director->mainLoop();
            glview->pollEvents();
        }

        // Director should still do a cleanup if the window was closed manually.
        if (glview->isOpenGLReady())
        {
            director->end();
            director->mainLoop();
            director = nullptr;
        }
        glview->release();
        return 0;
    }

    Application* Application::getInstance()
    {
        return sm_pSharedApplication;
    }

    void Application::setStartupScriptFilename(const std::string& startupScriptFile)
    {
        _startupScriptFilename = startupScriptFile;
        std::replace(_startupScriptFilename.begin(), _startupScriptFilename.end(), '\\', '/');
    }
}
