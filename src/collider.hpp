#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include <limits>

using f16 = fpm::fixed_16_16;

struct Collider {
    f16 x;
    f16 y;
    f16 height;
    f16 width;

    bool is_colliding(Collider& c) {
        return (x          < c.x + c.width
             && x + width  > c.x
             && y          < c.y + c.height
             && y + height > c.y
        );
    }
};

#endif // COLLIDER_HPP
