#ifndef RNG_H_GUARD
#define RNG_H_GUARD

#include <chrono>
#include <random>

struct RNG {
    using UDU = std::uniform_int_distribution<unsigned>;
    using UDI = std::uniform_int_distribution<int>;
    using UDD = std::uniform_real_distribution<double>;
    using UDF = std::uniform_real_distribution<float>;

    std::mt19937_64 mt;

    RNG();

    unsigned U(unsigned, unsigned);
    int I(int, int);
    float F(float, float);
    double D(double, double);
    int IN(int, int);
    float FN(float, float);
    double DN(double, double);

    template <typename T>
    T N(T v) {
        if (UDI(0, 1)(mt)) return -v;
        return v;
    }
};

#endif