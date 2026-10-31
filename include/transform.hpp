#pragma once

#include <math/aliases.hpp>

struct Transform {
    math::vector3f position {0.0f, 0.0f, 0.0f};
    math::vector3f rotation {0.0f, 0.0f, 0.0f};
    math::vector3f scale    {1.0f, 1.0f, 1.0f};

    constexpr Transform() = default;

    constexpr Transform(
        math::vector3f const &p,
        math::vector3f const &r,
        math::vector3f const &s
    ) : position(p), rotation(r), scale(s) {}

    constexpr Transform(
        math::vector3f const &p,
        math::vector3f const &r
    ) : position(p), rotation(r) {}

    explicit constexpr Transform(
        math::vector3f const &p
    ) : position(p) {}

    static constexpr Transform identity() {
        return {};
    }
};
