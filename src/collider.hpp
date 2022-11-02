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

    Collider() : x(0), y(0), height(0), width(0) {};

    template<typename T,
        typename = typename std::enable_if<std::is_arithmetic<T>::value,T>::type>
    Collider(T x, T y, T h, T w) :
        x(x), y(y), height(h), width(w)
    {};

    bool is_colliding(Collider& c) {
        return (x          < c.x + c.width
             && x + width  > c.x
             && y          < c.y + c.height
             && y + height > c.y
        );
    }
};

#endif // COLLIDER_HPP
