#ifndef OVERLAP_CORRECTION_H_GUARD
#define OVERLAP_CORRECTION_H_GUARD

#include <cmath>

/*
Circle-circle overlap correction.

The elastic collision response code (found in elastic_collision_response.h) used in this program does not handle overlapping star collision circles. Since, in many collisions, stars end up overlapping to some degree, we have to correct that overlap somehow before we compute the collision response. The method used here takes the overlap distance ("overlap"), divides it by 2, and displaces each star away from each other by that distance.

This is possible by considering the vector from one colliding star's center to the other ("collision vector"), scaling that vector down to the magnitude of half the overlap, and then adding that vector to the x, y position of one star and substracting from the other.

A visualization of the mathematical steps can be found here: https://www.desmos.com/calculator/zcg3yrwuj4

Definitions:
    a and b:
        The two star objects that are colliding.
        a_x, a_y, b_x, b_y refer to the x, y positional values of the stars.

    distance:
        The magnitude of the vector from the center point of one colliding star to the other.

    radii:
        The sum of the radii of the two colliding stars.

    overlap:
        overlap = (radii - distance)
        The length of the line segment that lies on the vector from one star center to the other AND resides within the overlapping area of the circles.

    half_overlap:
        half_overlap = (overlap / 2)
        Each star will be displaced by half the overlap.

    scalar:
        scalar = (half_overlap / distance)
        Distance is defined as the magnitude of the vector from one colliding star center to the other. So, if we want a "scaled down" version of this vector that has magnitude equal to half_overlap, we have to obtain a scalar with the ratio of the half_overlap to the distance. This is obtained via the above computation. Now that we have this scalar, we can multiply the x, y components of the collision vector by that scalar and get the desired scaled collision vector.

    x_component and y_component:
        x_component = (a_x - b_x)
        y_component = (a_y - b_y)
        These are the x, y values (components) of the vector from one colliding star's center to the other. In other words, these are the values that can be added to b_x, b_y to move b's position exactly onto a's.

    x_component_scaled and y_component_scaled:
        x_component_scaled = (x_component * scalar)
        y_component_scaled = (y_component * scalar)
        The components of the scaled down collision vector. This vector is then added/substract to/from the x, y positions of the colliding stars to eliminate overlap.

Mathematical steps:
    Given:
        a_x, a_y, b_x, b_y
        radii, distance

    overlap = (radii - distance)
    half_overlap = (overlap / 2)

    scalar = (half_overlap / distance)

    x_component = (a_x - b_x)
    y_component = (a_y - b_y)

    x_component_scaled = (x_component * scalar)
    y_component_scaled = (y_component * scalar)

    a_x += x_component_scaled;
    a_y += y_component_scaled;
    b_x -= x_component_scaled;
    b_y -= y_component_scaled;

The code in the following overlapCorrection() function is a condensed version of the above mathematical steps.
*/
inline void overlapCorrection(double& a_x, double& a_y, double& b_x, double& b_y,
                              double radii, double distance) {
    double scalar             = (radii - distance) / 2 / distance;
    double x_component_scaled = (a_x - b_x) * scalar;
    double y_component_scaled = (a_y - b_y) * scalar;

    a_x += x_component_scaled;
    a_y += y_component_scaled;
    b_x -= x_component_scaled;
    b_y -= y_component_scaled;
}

#endif