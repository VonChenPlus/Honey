#ifndef GMRANDOM_H
#define GMRANDOM_H

#include "BASE/Honey.h"
#include <random>

namespace UTILS
{
    namespace RANDOM
    {
        #define RANDOM_MINUS1_1() rand_minus1_1()
        inline float rand_minus1_1() {
            // FIXME: using the new c++11 random engine generator
            // without a proper way to set a seed is not useful.
            // Resorting to the old random method since it can
            // be seeded using std::srand()
            return ((std::rand() / (float)RAND_MAX) * 2) -1;

        //    return cocos2d::random(-1.f, 1.f);
        }

        #define RANDOM_0_1() rand_0_1()
        inline float rand_0_1() {
            // FIXME: using the new c++11 random engine generator
            // without a proper way to set a seed is not useful.
            // Resorting to the old random method since it can
            // be seeded using std::srand()
            return std::rand() / (float)RAND_MAX;

        //    return cocos2d::random(0.f, 1.f);
        }

        // George Marsaglia-style random number generator.
        class GMRandom
        {
        public:
            GMRandom() {
                w_ = 0x23E866ED;
                z_ = 0x80FD5AF2;
            }
            void init(int seed) {
                w_ = seed ^ (seed << 16);
                if (!w_) w_ = 1337;
                z_ = ~seed;
                if (!z_) z_ = 31337;
            }
            uint32 rand32() {
                z_ = 36969 * (z_ & 65535) + (z_ >> 16);
                w_ = 18000 * (w_ & 65535) + (w_ >> 16);
                return (z_ << 16) + w_;
            }
            float randFloat() {
                return (float)rand32() / (float)(0xFFFFFFFF);
            }

            // public for easy save/load. Yes a bit ugly but better than moving DoState into native.
        private:
            uint32 w_;
            uint32 z_;
        };
    }
}
#endif // GMRANDOM_H

