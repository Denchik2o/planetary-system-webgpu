#include "mesh_generators.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <vector>


inline std::uint32_t nextSegm(std::uint32_t m, std::uint32_t n) {
    return (m + 1u == n) ? 0u : (m + 1u); 
}

ObjMesh generateUvSphere(uint32_t segmentCount, uint32_t ringCount) {
    if (segmentCount < 3) {
        throw std::invalid_argument("UV sphere requires at least 3 segments");
    }
    if (ringCount < 2) {
        throw std::invalid_argument("UV sphere requires at least 2 rings");
    }

    ObjMesh mesh;
    mesh.vertices.reserve(static_cast<std::size_t>(segmentCount) * ringCount + 2);
    mesh.indices.reserve(static_cast<std::size_t>(segmentCount) * ringCount * 6);

    {
        math::vector3f const n{0.0f, 1.0f, 0.0f};
        mesh.vertices.push_back({n, n, math::vector2f{0.5f, 1.0f}});
    }

    float const dTheta = M_PI / static_cast<float>(ringCount + 1);
    float const dPhi = 2.0f * M_PI / static_cast<float>(segmentCount);

    for (std::uint32_t ring = 1u; ring <= ringCount; ++ring) {
        float const theta = dTheta * static_cast<float>(ring);
        float const sinT  = std::sin(theta);
        float const cosT  = std::cos(theta);
        float const v     = 1.0f - static_cast<float>(ring) / static_cast<float>(ringCount + 1);
 
        for (std::uint32_t seg = 1; seg <= segmentCount; ++seg) {
            float const phi = dPhi * static_cast<float>(seg);
            float const u   = static_cast<float>(seg) / static_cast<float>(segmentCount);

            math::vector3f const p{sinT * std::sin(phi), cosT, sinT * std::cos(phi)};
            mesh.vertices.push_back({p, p, math::vector2f{u, v}});
        }
    }

    std::uint32_t const southIdx = static_cast<std::uint32_t>(mesh.vertices.size());
    {
        math::vector3f const n{0.0f, -1.0f, 0.0f};
        mesh.vertices.push_back({n, n, math::vector2f{0.5f, 0.0f}});
    }

    std::uint32_t const northIdx = 0u;

    auto const ringBase = [&](std::uint32_t ring) -> uint32_t {
        return 1u + (ring - 1u) * segmentCount;
    };

    for (std::uint32_t m = 0u; m < segmentCount; ++m) {
        std::uint32_t const a = ringBase(1u) + m;
        std::uint32_t const b = ringBase(1u) + nextSegm(m, segmentCount);
        mesh.indices.push_back(northIdx);
        mesh.indices.push_back(a);
        mesh.indices.push_back(b);
    }

    for (std::uint32_t r = 1u; r < ringCount; ++r) {
        std::uint32_t const top    = ringBase(r);
        std::uint32_t const bottom = ringBase(r + 1u);

        for (std::uint32_t m = 0; m < segmentCount; ++m) {
            std::uint32_t const m1 = nextSegm(m, segmentCount);

            std::uint32_t const a = top    + m;
            std::uint32_t const b = top    + m1;
            std::uint32_t const c = bottom + m;
            std::uint32_t const d = bottom + m1;

            mesh.indices.push_back(a); mesh.indices.push_back(c); mesh.indices.push_back(b);
            mesh.indices.push_back(b); mesh.indices.push_back(c); mesh.indices.push_back(d);
        }
    }

    for (std::uint32_t m = 0u; m < segmentCount; ++m) {
        std::uint32_t const a = ringBase(ringCount) + m;
        std::uint32_t const b = ringBase(ringCount) + nextSegm(m, segmentCount);
        mesh.indices.push_back(southIdx);
        mesh.indices.push_back(b);
        mesh.indices.push_back(a);
    }

    return mesh;
}

namespace {

constexpr float kGolden = 1.6180339887498948482f;

std::array<math::vector3f, 12> const kIcosahedronVerts = {{
    {-1.0f,  kGolden,  0.0f},
    { 1.0f,  kGolden,  0.0f},
    {-1.0f, -kGolden,  0.0f},
    { 1.0f, -kGolden,  0.0f},
    { 0.0f, -1.0f,  kGolden},
    { 0.0f,  1.0f,  kGolden},
    { 0.0f, -1.0f, -kGolden},
    { 0.0f,  1.0f, -kGolden},
    { kGolden,  0.0f, -1.0f},
    { kGolden,  0.0f,  1.0f},
    {-kGolden,  0.0f, -1.0f},
    {-kGolden,  0.0f,  1.0f},
}};

std::array<std::array<std::uint32_t, 3>, 20> const kIcosahedronFaces = {{
    { 0, 11,  5}, { 0,  5,  1}, { 0,  1,  7}, { 0,  7, 10}, { 0, 10, 11},
    { 1,  5,  9}, { 5, 11,  4}, {11, 10,  2}, {10,  7,  6}, { 7,  1,  8},
    { 3,  9,  4}, { 3,  4,  2}, { 3,  2,  6}, { 3,  6,  8}, { 3,  8,  9},
    { 4,  9,  5}, { 2,  4, 11}, { 6,  2, 10}, { 8,  6,  7}, { 9,  8,  1},
}};

struct EdgeKey {
    std::uint32_t a = 0;
    std::uint32_t b = 0;
    friend bool operator == (EdgeKey const &, EdgeKey const &) = default;
};

struct EdgeKeyHash {
    std::size_t operator () (EdgeKey const & k) const {
        std::size_t seed = 0;
        seed ^= static_cast<std::size_t>(k.a) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        seed ^= static_cast<std::size_t>(k.b) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        return seed;
    }
};

std::uint32_t midpoint(
    std::uint32_t a, std::uint32_t b,
    std::vector<math::vector3f> & positions,
    std::unordered_map<EdgeKey, std::uint32_t, EdgeKeyHash> & cache
) {
    EdgeKey const key{a < b ? a : b, a < b ? b : a};
    if (auto it = cache.find(key); it != cache.end()) {
        return it->second;
    }
    math::vector3f const mid = normalized(positions[a] + positions[b]);
    std::uint32_t const idx = static_cast<std::uint32_t>(positions.size());
    positions.push_back(mid);
    cache.emplace(key, idx);
    return idx;
}

}

ObjMesh generateIcoSphere(uint32_t subdivisionLevel) {
    if (subdivisionLevel < 1 || subdivisionLevel > 8) {
        throw std::invalid_argument("icosphere subdivision level must be in [1, 8]");
    }

    std::uint32_t const iterations = subdivisionLevel - 1;

    std::vector<math::vector3f> positions;
    positions.reserve(kIcosahedronVerts.size());
    for (auto const & v : kIcosahedronVerts) {
        positions.push_back(normalized(v));
    }

    std::vector<std::array<std::uint32_t, 3>> faces(
        kIcosahedronFaces.begin(), kIcosahedronFaces.end()
    );

    for (std::uint32_t it = 0; it < iterations; ++it) {
        std::unordered_map<EdgeKey, std::uint32_t, EdgeKeyHash> cache;
        cache.reserve(faces.size() * 2);

        std::vector<std::array<std::uint32_t, 3>> next;
        next.reserve(faces.size() * 4);

        for (auto const & f : faces) {
            std::uint32_t const a = f[0];
            std::uint32_t const b = f[1];
            std::uint32_t const c = f[2];
            
            std::uint32_t const ab = midpoint(a, b, positions, cache);
            std::uint32_t const bc = midpoint(b, c, positions, cache);
            std::uint32_t const ca = midpoint(c, a, positions, cache);

            next.push_back({a,  ab, ca});
            next.push_back({b,  bc, ab});
            next.push_back({c,  ca, bc});
            next.push_back({ab, bc, ca});
        }

        faces = std::move(next);
    }

    ObjMesh mesh;
    mesh.vertices.reserve(positions.size());
    for (auto const & p : positions) {
        mesh.vertices.push_back({p, p, math::vector2f{0.0f, 0.0f}});
    }

    mesh.indices.reserve(faces.size() * 3);
    for (auto const & f : faces) {
        mesh.indices.push_back(f[0]);
        mesh.indices.push_back(f[1]);
        mesh.indices.push_back(f[2]);
    }

    return mesh;
}

