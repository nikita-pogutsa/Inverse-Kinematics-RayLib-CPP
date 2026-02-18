#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ik2d
{
struct Vec2
{
    float x{};
    float y{};
};

inline Vec2 operator+(const Vec2 a, const Vec2 b) { return Vec2{a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(const Vec2 a, const Vec2 b) { return Vec2{a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(const Vec2 v, const float s) { return Vec2{v.x * s, v.y * s}; }

inline float Length(const Vec2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

struct TwoBoneResult
{
    Vec2 knee{};
    Vec2 end{};

    // Angles are in radians and consistent with the input coordinate system.
    float phi{};    // direction root -> target
    float psi{};    // hip offset angle
    float gamma{};  // internal knee angle
    float theta1{}; // first segment absolute angle
    float theta2{}; // second segment relative angle (added to theta1)

    float radius{};      // clamped |target-root|
    std::int8_t bend{};  // +1 or -1 solution used
    bool wasClamped{};   // whether radius had to be clamped to reachable range
};

class TwoBoneIK2D
{
public:
    // Solves a 2D two-segment IK chain.
    // - bendDir: +1 or -1, selects the mirrored knee solution when autoBend=false.
    // - autoBend: when true, bendDir is picked from the sign of (target-root).x.
    static TwoBoneResult Solve(const Vec2 root,
                               const float L1,
                               const float L2,
                               const Vec2 target,
                               const int bendDir = 1,
                               const bool autoBend = false)
    {
        static constexpr float k_pi = 3.14159265358979323846f;

        TwoBoneResult out{};

        const Vec2 localTarget = target - root;

        const float maxReach = L1 + L2;
        const float minReach = std::fabs(L1 - L2);

        const float rawRadius = Length(localTarget);
        const float clampedRadius =
            std::clamp(rawRadius, std::max(0.001f, minReach), maxReach);

        const std::int8_t heading = static_cast<std::int8_t>(
            autoBend ? (localTarget.x >= 0.0f ? 1 : -1) : (bendDir >= 0 ? 1 : -1));

        out.bend = heading;
        out.radius = clampedRadius;
        out.wasClamped = (std::fabs(clampedRadius - rawRadius) > 1e-6f);

        out.phi = std::atan2(localTarget.y, localTarget.x);

        // Angle between L1 and r (root to target).
        const float cosPsi = (clampedRadius * clampedRadius + L1 * L1 - L2 * L2) /
                             (2.0f * L1 * clampedRadius);
        out.psi = std::acos(std::clamp(cosPsi, -1.0f, 1.0f));

        // Internal knee angle between L1 and L2.
        const float cosGamma =
            (L1 * L1 + L2 * L2 - clampedRadius * clampedRadius) / (2.0f * L1 * L2);
        out.gamma = std::acos(std::clamp(cosGamma, -1.0f, 1.0f));

        out.theta1 = out.phi - static_cast<float>(heading) * out.psi;
        out.theta2 = static_cast<float>(heading) * (k_pi - out.gamma);

        const float c1 = std::cos(out.theta1);
        const float s1 = std::sin(out.theta1);
        const float c2 = std::cos(out.theta1 + out.theta2);
        const float s2 = std::sin(out.theta1 + out.theta2);

        out.knee = root + Vec2{c1, s1} * L1;
        out.end = out.knee + Vec2{c2, s2} * L2;

        return out;
    }
};
} // namespace ik2d

