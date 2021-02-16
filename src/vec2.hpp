#pragma once

#include <cmath>

struct vec2 {
    float x, y;
};

inline vec2 operator+(vec2 const &lhs, vec2 const &rhs) {
    return { lhs.x + rhs.x, lhs.y + rhs.y };
}

inline vec2 operator-(vec2 const &lhs, vec2 const &rhs) {
    return { lhs.x - rhs.x, lhs.y - rhs.y };
}

inline vec2 operator*(float lhs, vec2 const &rhs) {
    return { lhs * rhs.x, lhs * rhs.y };
}

inline float dot(vec2 const &lhs, vec2 const &rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

inline float length_sq(vec2 const &v) {
    return dot(v, v);
}

inline float length(vec2 const &v) {
    return std::sqrt(length_sq(v));
}

inline float distance(vec2 const &lhs, vec2 const &rhs) {
    return length(rhs - lhs);
}

inline vec2 normalized(vec2 const &v) {
    auto len = length(v);
    return (1 / len) * v;
}

struct point_line_relation {
    float distance;
    float radians;
};

/*
 * Kiszamitja a pontnak az egyeneshez relativ helyzetet: az egyeneshez kepesti tavolsagat es iranyat.
 * @param origin Az egyenes egy pontja
 * @param direction Az egyenes iranya, lehetoleg normalizalva
 * @param x A kerdeses pont
 *
 * @return Egy Point_Line_Relation tipusu struct, amely az eredmenyeket tartalmazza
 *
 * @note Ha a pont az egyenes bal oldalan van, akkor pozitiv a szog!
 */
inline point_line_relation point_line_distance(vec2 const &origin, vec2 const &direction, vec2 const &x) {
    // Kiszamitjuk a kezdopontbol az x-be mutato vektort.
    auto vp = origin - x;
    // Ezt a vektort levetitjuk az egyenesre.
    auto vp_dot_n = dot(vp, direction);
    auto proj = vp_dot_n * direction;
    // Kivonjuk a vp vektorbol a vetitest, hogy megkapjuk a vp-nek az egyenesre
    // meroleges komponenset.
    // Megforditjuk a vektort, mivel a szog erteket szeretnenk az egyenes
    // szempontjabol megkapni.
    auto diff = -1 * (vp - proj);
    // Hosszt es szoget szamolunk.
    auto len = length(diff);
    auto ang = std::atan2(diff.y, diff.x);
    return { len, ang };
}

float signed_distance_from_line(vec2 const &lp0, vec2 const &lp1, vec2 const &x) {
    auto direction = normalized(lp1 - lp0);
    // Kiszamitjuk a kezdopontbol az x-be mutato vektort.
    auto vp = lp0 - x;
    // Ezt a vektort levetitjuk az egyenesre.
    auto vp_dot_n = dot(vp, direction);
    auto proj = vp_dot_n * direction;
    // Kivonjuk a vp vektorbol a vetitest, hogy megkapjuk a vp-nek az egyenesre
    // meroleges komponenset.
    // Megforditjuk a vektort, mivel a vektort szeretnenk az egyenes
    // szempontjabol megkapni.
    auto perpen = -1 * (vp - proj);
    // Kiszamitjuk az egyenes "befele" tekinto normalisat.
    // Lenyegeben az iranyvektor elforgatva 90 fokkal az orajarasaval
    // ellentetes iranyba
    vec2 n{ -direction.y, direction.x };
    // Levetitjuk a merolegest a normalisra
    return dot(perpen, n);
}
