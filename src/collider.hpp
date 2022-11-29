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
    bool is_active = true;

    Collider() : x(0), y(0), height(0), width(0) {};

    template<typename T,
        typename = typename std::enable_if<std::is_arithmetic<T>::value,T>::type>
    Collider(T x, T y, T h, T w) :
        x(x), y(y), height(h), width(w)
    {};

    bool static is_colliding(Collider& l, Collider& r){
        // Make sure both of the colliders are active
        // Then check using AABB
        return ((l.is_active && r.is_active) &&
            (  l.x            < r.x + r.width
            && l.x + l.width  > r.x
            && l.y            < r.y + r.height
            && l.y + l.height > r.y
            )
        );
    }
};

#endif // COLLIDER_HPP
