#ifndef UNITYSHADERCACHE_H
#define UNITYSHADERCACHE_H

#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    class Unity3DShaderCache : public HObject
    {
    public:
        Unity3DShaderCache();
        ~Unity3DShaderCache();

        static Unity3DShaderCache& getInstance();

        void loadDefaultShaders();

        Unity3DShaderSet *getU3DShader(const std::string &key);
        void addU3DShader(Unity3DShaderSet* program, const std::string &key);

    private:
        bool init();
        void loadDefaultGLShader(Unity3DShaderSet *program, int type);

    private:
        std::unordered_map<std::string, Unity3DShaderSet*> programs_;
    };
}

#endif // UNITYSHADERCACHE_H
