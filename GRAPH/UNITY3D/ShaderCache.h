#ifndef SHADERCACHE_H
#define SHADERCACHE_H

#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    class ShaderCache : public HObject
    {
    public:
        ShaderCache();
        ~ShaderCache();

        static ShaderCache& getInstance();

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

#endif // SHADERCACHE_H
