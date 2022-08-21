#ifndef ELASTICCOLLISIONRESPONSE_H
#define ELASTICCOLLISIONRESPONSE_H

#include <cmath>

/*
Circle-circle elastic collision response.

The code within the following elasticCollisionResponse() function is an adaptation of the "angle-free representation" versions of the circle-circle elastic collision response formulas found in this Wikipedia article:

    https://en.wikipedia.org/wiki/Elastic_collision#Two-dimensional_collision_with_two_moving_objects

The text within this Wikipedia article is licensed under Creative Commons Attribution-ShareAlike License 3.0:

    https://creativecommons.org/licenses/by-sa/3.0/

One requirement listed by the "human-readable summary" of this license states:

    "If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original."

Consequently, the code in this elasticCollisionResponse() function is also licensed under Creative Commons Attribution-ShareAlike License 3.0.
*/
inline void elasticCollisionResponse(double &a_x, double &a_y, double &b_x, double &b_y,
                                     double &a_x_vel, double &a_y_vel, double &b_x_vel, double &b_y_vel,
                                     double &a_mass, double &b_mass) {
    double x_temp = a_x - b_x;
    double y_temp = a_y - b_y;
    double temp = 2.0 / (a_mass + b_mass) *
                  (((a_x_vel - b_x_vel) * x_temp + (a_y_vel - b_y_vel) * y_temp) /
                   (pow(x_temp, 2) + pow(y_temp, 2)));

    x_temp *= temp;
    y_temp *= temp;

    a_x_vel -= b_mass * x_temp;
    a_y_vel -= b_mass * y_temp;
    b_x_vel += a_mass * x_temp;
    b_y_vel += a_mass * y_temp;
}

#endif