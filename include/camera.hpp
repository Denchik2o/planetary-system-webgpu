#pragma once

#include <math/aliases.hpp>

struct Camera {
    math::vector3f position{0.f, 0.f, 5.f};
    float yaw   = 0.f;
    float pitch = 0.f;
    float mouseSensivity = 0.003f;
    float moveSpeed = 5.f;

    static constexpr float maxPitch = M_PI_2;

    void onMouseMove(float dx, float dy) {
        yaw  -= dx * mouseSensivity;
        pitch = math::clamp(pitch - dy * mouseSensivity, -maxPitch, maxPitch);
    }

    math::vector3f forward() const {
        float const cp = std::cos(pitch);
        return {
            -std::sin(yaw) * cp,
             std::sin(pitch),
            -std::cos(yaw) * cp
        };
    }

    math::vector3f right() const {
        return {
             std::cos(yaw), 
             0.f, 
            -std::sin(yaw)
        };
    }

    math::matrix4f view() const {
        math::vector3f const fw = forward();
        math::vector3f const rt = right();
        math::vector3f const up = math::cross(rt, fw);

        math::matrix4f m;
        m[0][0] = rt.x(); m[0][1] = rt.y(); m[0][2] = rt.z();
        m[0][3] = -math::dot(rt, position);
        m[1][0] = up.x(); m[1][1] = up.y(); m[1][2] = up.z();
        m[1][3] = -math::dot(up, position);
        m[2][0] = -fw.x(); m[2][1] = -fw.y(); m[2][2] = -fw.z();
        m[2][3] = math::dot(fw, position);
        m[3][0] = 0.f; m[3][1] = 0.f; m[3][2] = 0.f; m[3][3] = 1.f;

        return m;
    }
};
