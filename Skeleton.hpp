#pragma once

#include "MeshTriangle.hpp"
#include "OBJ_Loader.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <vector>

const float EPS = 1e-6f;

struct Vert {
    Vector3f pos;
    bool active = true;
    std::vector<int> adj; // index for edges
};
struct Edge {
    int a, b;
    float length;
};
struct Face {
    int a, b, c;
};
struct Chain {
    std::vector<Vector3f> points;
};
struct Curve {
    std::vector<Vector3f> points;
    std::vector<Vector3f> tangents;
};
std::vector<Vert> verts;
std::vector<Face> faces;
std::vector<Edge> edges;
bool isRegularFace(const Face &f) {
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
    for (size_t i = 0; i < verts.size();
         ++i) { /// loop to deduplicate(to be optimized)
        Vector3f d = verts[i].pos - p;
        if (d.norm() < EPS)
            return (int)i;
    }
    verts.push_back(Vert{p, true});
    return verts.size() - 1; /// return index
};
void addEdge(int a, int b) {
    if (a == b)
        return;
    if (a > b)
        std::swap(a, b);
    for (auto &e : edges) /// loop to deduplicate(to be optimized)
        if (e.a == a && e.b == b)
            return;
    Edge e;
    e.a = a;
    e.b = b;
    e.length = (verts[a].pos - verts[b].pos).norm();
    edges.push_back(e);
    verts[a].adj.push_back(edges.size() - 1);
    verts[b].adj.push_back(edges.size() - 1);
};
void buildSkeleton(const MeshTriangle &input) {
    verts.clear();
    edges.clear();
    faces.clear();
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
        for (size_t i = 0; i < edges.size(); ++i) { /// to be optimized
            const Edge &e = edges[i];
            if (!verts[e.a].active || !verts[e.b].active)
                continue;
            if (e.a == e.b)
                continue;
            if (e.length < min_len) {
                min_len = e.length;
                min_idx = i;
            }
        }
        if (!std::isfinite(min_len))
            break;
        int v_keep = edges[min_idx].a;
        int v_rem = edges[min_idx].b;
        verts[v_rem].active = false;
        for (auto &edge_idx : verts[v_rem].adj) {
            Edge &e = edges[edge_idx];
            if (e.a == v_rem) {
                e.a = v_keep;
                verts[v_keep].adj.push_back(edge_idx);
                e.length = (verts[e.a].pos - verts[e.b].pos).norm();
            }
            if (e.b == v_rem) {
                e.b = v_keep;
                verts[v_keep].adj.push_back(edge_idx);
                e.length = (verts[e.a].pos - verts[e.b].pos).norm();
            }
        }
    }
}
std::vector<Chain> buildChains() {
    std::vector<Chain> out;
    std::vector<char> edgeVisited(edges.size(), false);
    for (int ei = 0; ei < edges.size(); ++ei) {
        if (edgeVisited[ei])
            continue;
        Edge &startEdge = edges[ei];
        if (!verts[startEdge.a].active || !verts[startEdge.b].active)
            continue;
        Chain chain;
        int prev = -1;
        int curr = startEdge.a;

        while (true) {
            chain.points.push_back(verts[curr].pos);
            int degree = 0;
            for (int eid : verts[curr].adj) {
                Edge &e = edges[eid];
                if (verts[e.a].active && verts[e.b].active)
                    degree++;
            }
            if (degree != 2 && chain.points.size() > 1)
                break;

            int next = -1;
            int usedei = -1;
            for (int eid : verts[curr].adj) {
                Edge &e = edges[eid];
                if (!verts[e.a].active || !verts[e.b].active)
                    continue;
                if (edgeVisited[eid])
                    continue;

                int neighbor = (e.a == curr) ? e.b : e.a;
                if (neighbor != prev) {
                    next = neighbor;
                    usedei = eid;
                    break;
                }
            }

            if (next == -1)
                break;
            edgeVisited[usedei] = true;
            prev = curr;
            curr = next;
        }
        if (chain.points.size() >= 2) {
            out.push_back(chain);
        }
    }
    return out;
}   
Curve CatmullSpline(const Chain &chain, int samples = 3) {
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
            Vector3f pos = (2 * p1 - 2 * p2 + v0 + v1) * t3 +
                           (-3 * p1 + 3 * p2 - 2 * v0 - v1) * t2 + v0 * t + p1;

            Vector3f tan = (6 * p1 - 6 * p2 + 3 * v0 + 3 * v1) * t2 +
                           (-6 * p1 + 6 * p2 - 4 * v0 - 2 * v1) * t + v0;
            curve.points.push_back(pos);
            curve.tangents.push_back(tan.normalized());
        }
    }
    return curve;
}
Chain laplacianSmooth(const Chain &chain, int iter) {
    if (chain.points.size() < 3) return chain;
    Chain smoothed = chain;
    Chain temp = chain;
    for (int k = 0; k < iter; ++k) {
        temp.points[0] = smoothed.points[0];
        temp.points.back() = smoothed.points.back();

        for (size_t i = 1; i < smoothed.points.size() - 1; ++i) {
            temp.points[i] =
                (smoothed.points[i - 1] + smoothed.points[i + 1]) * 0.5f;
        }
        std::swap(smoothed.points, temp.points);
    }
    return smoothed;
}
Curve buildCurve(const Chain &chain) {
    if(!chain.points.size()) return Curve{};
    Chain smoothed = laplacianSmooth(chain, 3); /// harcdoded 3
    Curve spline = CatmullSpline(smoothed);
    return spline;
}
