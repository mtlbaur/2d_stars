#include "rng.h"

RNG::RNG() {
    mt.seed(std::chrono::system_clock::now().time_since_epoch().count());
}
unsigned RNG::U(unsigned l, unsigned u) {
    return UDU(l, u)(mt);
}
int RNG::I(int l, int u) {
    return UDI(l, u)(mt);
}
float RNG::F(float l, float u) {
    return UDF(l, u)(mt);
}
double RNG::D(double l, double u) {
    return UDD(l, u)(mt);
}
int RNG::IN(int l, int u) {
    return N(UDI(l, u)(mt));
}
float RNG::FN(float l, float u) {
    return N(UDF(l, u)(mt));
}
double RNG::DN(double l, double u) {
    return N(UDD(l, u)(mt));
}