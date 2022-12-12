#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <glm/gtc/constants.hpp>

namespace Constants {
    inline constexpr double PI      = glm::pi<double>();
    inline constexpr double TWO_PI  = glm::two_pi<double>();
    inline constexpr double GRAVITY = 9.80665;
} // namespace Constants

#endif