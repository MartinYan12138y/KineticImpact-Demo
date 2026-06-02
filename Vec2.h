// COMP710 - KineticImpact
// Vec2.h - 2D vector utility

#pragma once
#include <cmath>

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;

    // ── Construction ──────────────────────────────────────────────────────────
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    // ── Arithmetic operators ──────────────────────────────────────────────────
    Vec2  operator+ (const Vec2& o) const { return { x + o.x, y + o.y }; }
    Vec2  operator- (const Vec2& o) const { return { x - o.x, y - o.y }; }
    Vec2  operator* (float s)       const { return { x * s,   y * s }; }
    Vec2  operator/ (float s)       const { return { x / s,   y / s }; }

    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s;   y *= s;   return *this; }

    bool  operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool  operator!=(const Vec2& o) const { return !(*this == o); }

    // ── Vector math ───────────────────────────────────────────────────────────
    float length()    const { return std::sqrt(x * x + y * y); }
    float lengthSq()  const { return x * x + y * y; }

    // Returns a unit vector; returns zero vector if length is zero
    Vec2 normalised() const
    {
        float len = length();
        if (len < 1e-6f) return { 0.0f, 0.0f };
        return { x / len, y / len };
    }

    float dot(const Vec2& o)   const { return x * o.x + y * o.y; }
    float distance(const Vec2& o) const { return (*this - o).length(); }

    // ── Rotation helpers ──────────────────────────────────────────────────────
    // Angle in DEGREES from positive X axis (right), clockwise = positive
    float angleDeg() const
    {
        return std::atan2(y, x) * (180.0f / 3.14159265f);
    }

    // Build a direction vector from an angle in degrees
    static Vec2 fromAngleDeg(float deg)
    {
        float rad = deg * (3.14159265f / 180.0f);
        return { std::cos(rad), std::sin(rad) };
    }

    // ── Convenience statics ───────────────────────────────────────────────────
    static Vec2 zero() { return { 0.0f,  0.0f }; }
    static Vec2 up() { return { 0.0f, -1.0f }; }
    static Vec2 down() { return { 0.0f,  1.0f }; }
    static Vec2 left() { return { -1.0f,  0.0f }; }
    static Vec2 right() { return { 1.0f,  0.0f }; }
};

// Scalar on left side: 2.0f * vec
inline Vec2 operator*(float s, const Vec2& v) { return v * s; }