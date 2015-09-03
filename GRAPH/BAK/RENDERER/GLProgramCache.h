#ifndef GLPROGRAMCACHE_H
#define GLPROGRAMCACHE_H

#include <string>
#include <unordered_map>

#include "BASE/HObject.h"

namespace GRAPH
{
    class GLProgram;
    class GLProgramCache : public HObject
    {
    public:
        /**
        Constructor.
         * @js ctor
         */
        GLProgramCache();
        /**
        Destructor.
         * @js NA
         * @lua NA
         */
        ~GLProgramCache();

        /** returns the shared instance */
        static GLProgramCache* getInstance();

        /** purges the cache. It releases the retained instance. */
        static void destroyInstance();

        /** loads the default shaders */
        void loadDefaultGLPrograms();

        /** reload the default shaders */
        void reloadDefaultGLPrograms();

        /** returns a GL program for a given key
         */
        GLProgram * getGLProgram(const std::string &key);

        /** adds a GLProgram to the cache for a given name */
        void addGLProgram(GLProgram* program, const std::string &key);

    private:
        /**
        @{
            Init and load predefined shaders.
        */
        bool init();
        void loadDefaultGLProgram(GLProgram *program, int type);
        /**
        @}
        */

        /**Get macro define for lights in current openGL driver.*/
        std::string getShaderMacrosForLight() const;

        /**Predefined shaders.*/
        std::unordered_map<std::string, GLProgram*> _programs;
    };
}

#endif // GLPROGRAMCACHE_H
