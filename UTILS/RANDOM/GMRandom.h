#ifndef GMRANDOM_H
#define GMRANDOM_H

#include "BASE/Native.h"

namespace UTILS
{
    namespace RANDOM
    {
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

