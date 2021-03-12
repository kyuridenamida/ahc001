//
// Created by kyuridenamida on 2020/02/17.
//

#ifndef MARATHON_HELPERS_XORSHIFT_HPP
#define MARATHON_HELPERS_XORSHIFT_HPP

class XorShift {
public:
    XorShift() {
        initialize();
    }

    void initialize() {
        x = 123456789;
        y = 362436069;
        z = 521288629;
        w = 88675123;
    }

    // [0, ub)
    inline unsigned int next_uint32(unsigned int ub) {
        assert(ub > 0);
        return next() % ub;
    }

    // [lb, ub)
    unsigned next_uint32(unsigned int lb, unsigned int ub) {
        return lb + next_uint32(ub - lb);
    }

    // [lb, ub)
    unsigned next_int32(int lb, int ub) {
        // Need verification
        return lb + next_uint32(ub - lb);
    }

    // [0, ub)
    inline unsigned long long next_uint64(unsigned long long ub) {
        return ((1ull * next() << 32) | next()) % ub;
    }

    // [lb, ub)
    unsigned long next_uint64(unsigned long long lb, unsigned long long ub) {
        return lb + next_uint64(ub - lb);
    }

    // [0, 1.0)
    inline double next_prob() {
        return (double) next() / ((long long) 1 << 32);
    }

private:
    unsigned int x, y, z, w;

    inline unsigned int next() {
        unsigned int t = x ^x << 11;
        x = y;
        y = z;
        z = w;
        return w = w ^ w >> 19 ^ t ^ t >> 8;
    }

};

#endif //MARATHON_HELPERS_XORSHIFT_HPP