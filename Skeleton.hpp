#pragma once

#include "MeshTriangle.hpp"
#include "OBJ_Loader.hpp"
#include "Vector.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

const float EPS = 1e-6f;

struct Vert { Vector3f pos; bool active = true; };
struct Edge { int a, b; float length; };
struct Face { int a, b, c; };
struct Chain{ std::vector<Vector3f> points;};
struct Curve {
    std::vector<Vector3f> points;    // 平滑后的点
    std::vector<Vector3f> tangents;  // 每点切线（导数）→ 直接用于定义平面
};
std::vector<Vert> verts;
std::vector<Face> faces;
std::vector<Edge> edges;
bool isRegularFace(const Face &f){
    if (!verts[f.a].active || !verts[f.b].active || !verts[f.c].active)
        return false;
    if (f.a == f.b || f.b == f.c || f.c == f.a)
        return false;
    return true;
};
bool hasRegular() {
    for (auto &f : faces)
        if (isRegularFace(f))
            return true;
    return false;
};
int addVert(const Vector3f &p) {
    for (size_t i = 0; i < verts.size(); ++i) { /// loop to deduplicate(to be optimized)
        Vector3f d = verts[i].pos - p;
        if (d.norm() < EPS)
            return (int)i;
    }
    verts.push_back(Vert{p, true});
    return verts.size() - 1;   ///return index
};
void addEdge(int a, int b){
    if (a == b)
        return;
    if (a > b)
        std::swap(a, b);
    for (auto &e : edges)  /// loop to deduplicate(to be optimized)
        if (e.a == a && e.b == b)
            return;
    Edge e;
    e.a = a;
    e.b = b;
    e.length = (verts[a].pos - verts[b].pos).norm();
    edges.push_back(e);
};
inline std::vector<Chain> buildSkeleton(const MeshTriangle &input)
{
    for (auto &tri : input.triangles) {
        int i0 = addVert(tri.v0);
        int i1 = addVert(tri.v1);
        int i2 = addVert(tri.v2);
        faces.push_back(Face{i0, i1, i2});
    }
    for (auto &f : faces) {
        addEdge(f.a, f.b);
        addEdge(f.b, f.c);
        addEdge(f.c, f.a);
    }
    while (hasRegular()) {
        float min_len = std::numeric_limits<float>::infinity();
        float min_idx = -1;
        for (size_t i = 0; i < edges.size(); ++i) {  ///to be optimized
            const Edge &e = edges[i];
            if (!verts[e.a].active || !verts[e.b].active) continue;
            if (e.a == e.b) continue;
            if (e.length < min_len) {
                min_len = e.length;
                min_idx = i;
            }
        }
        if (!std::isfinite(min_len)) break;

        int v_keep = edges[min_idx].a;
        int v_rem = edges[min_idx].b;

        verts[v_rem].active = false;
        for (auto &e : edges) { ///to be optimized
            if (e.a == v_rem) {
                e.a = v_keep;
                e.length = (verts[e.a].pos - verts[e.b].pos).norm();
            }
            if (e.b == v_rem) {
                e.b = v_keep;
                e.length = (verts[e.a].pos - verts[e.b].pos).norm();
            }
        }
    }

        std::vector<Chain> out;

        // Build adjacency for active vertices
        std::vector<std::vector<int>> adj(verts.size());
        for (const auto &e : edges) {
            if (e.a < 0 || e.b < 0 || e.a >= (int)verts.size() || e.b >= (int)verts.size()) continue;
            if (!verts[e.a].active || !verts[e.b].active) continue;
            if (e.a == e.b) continue;
            adj[e.a].push_back(e.b);
            adj[e.b].push_back(e.a);
        }

        std::vector<char> visited(verts.size(), 0);

        auto build_chain_from_endpoint = [&](int start)->Chain {
            Chain chain;
            int prev = -1;
            int cur = start;
            // follow until degree != 2 (stop at junction or end)
            while (true) {
                chain.points.push_back(verts[cur].pos);
                visited[cur] = 1;
                const auto &nbrs = adj[cur];
                if (nbrs.size() == 0) break;
                int next = -1;
                for (int nb : nbrs) if (nb != prev) { next = nb; break; }
                if (next == -1) break; // dead end
                // if current is junction (degree != 2) and not start, stop after adding it
                if (prev != -1 && adj[cur].size() != 2) break;
                prev = cur;
                cur = next;
                if (visited[cur]) { // already visited neighbor, stop
                    break;
                }
            }
            return chain;
        };

        // 1) extract chains starting from endpoints (degree==1)
        for (size_t i = 0; i < verts.size(); ++i) {
            if (!verts[i].active) continue;
            if (visited[i]) continue;
            if (adj[i].size() == 1) {
                Chain c = build_chain_from_endpoint((int)i);
                if (!c.points.empty()) out.push_back(std::move(c));
            }
        }

        // 2) extract remaining cycles / components (no endpoints)
        for (size_t i = 0; i < verts.size(); ++i) {
            if (!verts[i].active) continue;
            if (visited[i]) continue;
            if (adj[i].empty()) { // isolated vertex
                Chain c; c.points.push_back(verts[i].pos); visited[i] = 1; out.push_back(c); continue;
            }
            // walk a cycle/chain within the component
            int prev = -1;
            int cur = (int)i;
            Chain c;
            while (true) {
                c.points.push_back(verts[cur].pos);
                visited[cur] = 1;
                int next = -1;
                for (int nb : adj[cur]) if (nb != prev) { next = nb; break; }
                if (next == -1) break;
                prev = cur;
                cur = next;
                if (visited[cur]) break;
            }
            if (!c.points.empty()) out.push_back(std::move(c));
        }

        // compute lengths and sort from long to short
        std::vector<std::pair<float, Chain>> tmp;
        tmp.reserve(out.size());
        for (auto &c : out) {
            float len = 0.0f;
            for (size_t k = 1; k < c.points.size(); ++k)
                len += (c.points[k] - c.points[k-1]).norm();
            tmp.emplace_back(len, std::move(c));
        }
        std::sort(tmp.begin(), tmp.end(), [](const auto &a, const auto &b){ return a.first > b.first; });
        out.clear(); out.reserve(tmp.size());
        for (auto &p : tmp) out.push_back(std::move(p.second));

    


    return out;
}

Curve leastSquaresSpline(const Chain &chain, int samples = 3)
{
    Curve curve;
    size_t pointCount = chain.points.size();

    if (pointCount == 1) {
        curve.points = chain.points;
        curve.tangents.emplace_back(1, 0, 0); 
        return curve;
    }
    if (pointCount < 4) {
        for (size_t i = 0; i < pointCount - 1; ++i) {
            Vector3f start = chain.points[i];
            Vector3f end = chain.points[i + 1];
            Vector3f dir = end - start;
            Vector3f tanDir = dir.normalized();
            int maxK = (i == pointCount - 2) ? (samples) : (samples - 1);
            for (int k = 0; k <= maxK; ++k) {
                float t = (float)k / (float)samples;
                Vector3f pos = (1 - t) * start + t * end;
                curve.points.push_back(pos);
                curve.tangents.push_back(tanDir);
            }
        }
        return curve;
    }
    Vector3f p0 = chain.points[0];
    Vector3f p1 = chain.points[0];
    Vector3f p2 = chain.points[1];
    Vector3f p3 = chain.points[2];
    Vector3f v0 = (p2 - p0) * 0.5f;
    Vector3f firstTan = v0.normalized();
    curve.points.push_back(p1);
    curve.tangents.push_back(firstTan);

    for (size_t i = 0; i < pointCount - 1; ++i) {
        p0 = (i == 0) ? chain.points[0] : chain.points[i - 1];
        p1 = chain.points[i];
        p2 = chain.points[i + 1];
        p3 = (i + 2 >= pointCount) ? chain.points.back() : chain.points[i + 2];
        v0 = (p2 - p0) * 0.5f;
        Vector3f v1 = (p3 - p1) * 0.5f;

        for (int k = 1; k <= samples; ++k) {
            float t = (float)k / (float)samples;
            float t2 = t * t;
            float t3 = t2 * t;

            Vector3f pos = (2 * p1 - 2 * p2 + v0 + v1) * t3
                         + (-3 * p1 + 3 * p2 - 2 * v0 - v1) * t2
                         + v0 * t + p1;

            Vector3f tan = (6 * p1 - 6 * p2 + 3 * v0 + 3 * v1) * t2
                         + (-6 * p1 + 6 * p2 - 4 * v0 - 2 * v1) * t
                         + v0;

            curve.points.push_back(pos);
            curve.tangents.push_back(tan.normalized());
        }
    }
    return curve;
}
Chain laplacianSmooth(const Chain &chain, int iter) {
    Chain smoothed = chain;
    for (int k = 0; k < iter; ++k) {
        for (size_t i = 1; i < smoothed.points.size()-1; ++i) {
            smoothed.points[i] = (smoothed.points[i-1] + smoothed.points[i+1]) * 0.5f;
        }
    }
    return smoothed;
}
Curve buildCurve(const Chain &chain) {
    Chain smoothed = laplacianSmooth(chain, 3); ///harcdoded 3
    Curve spline = leastSquaresSpline(smoothed);
    return spline;
}
