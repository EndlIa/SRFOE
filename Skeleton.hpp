#pragma once

#include "MeshTriangle.hpp"
#include "Vector.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

const float EPS = 1e-6f;

struct Vert {
    Vector3f pos;
    bool active = true;
    std::vector<int> adj; // indices of edges
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

bool isRegularFace(const Face& f) {
    if (f.a < 0 || f.b < 0 || f.c < 0) return false;
    if (f.a >= (int)verts.size() || f.b >= (int)verts.size() || f.c >= (int)verts.size())
        return false;
    if (!verts[f.a].active || !verts[f.b].active || !verts[f.c].active)
        return false;
    if (f.a == f.b || f.b == f.c || f.c == f.a)
        return false;
    return true;
}

bool hasRegular() {
    for (const auto& f : faces) {
        if (isRegularFace(f))
            return true;
    }
    return false;
}

int addVert(const Vector3f& p) {
    for (size_t i = 0; i < verts.size(); ++i) { // naive dedup
        Vector3f d = verts[i].pos - p;
        if (d.norm() < EPS)
            return (int)i;
    }
    verts.push_back(Vert{p, true, {}});
    return (int)verts.size() - 1;
}

void addEdge(int a, int b) {
    if (a == b)
        return;
    if (a > b)
        std::swap(a, b);

    for (const auto& e : edges) { // naive dedup
        if (e.a == a && e.b == b)
            return;
    }

    Edge e;
    e.a = a;
    e.b = b;
    e.length = (verts[a].pos - verts[b].pos).norm();

    edges.push_back(e);
    int eid = (int)edges.size() - 1;
    verts[a].adj.push_back(eid);
    verts[b].adj.push_back(eid);
}

void rebuildEdgesFromFaces() {
    edges.clear();
    for (auto& v : verts)
        v.adj.clear();

    for (const auto& f : faces) {
        if (!isRegularFace(f))
            continue;
        addEdge(f.a, f.b);
        addEdge(f.b, f.c);
        addEdge(f.c, f.a);
    }
}

void collapseVertInFaces(int v_keep, int v_rem) {
    for (auto& f : faces) {
        if (f.a == v_rem) f.a = v_keep;
        if (f.b == v_rem) f.b = v_keep;
        if (f.c == v_rem) f.c = v_keep;
    }
}

void removeDegenerateFaces() {
    std::vector<Face> newFaces;
    newFaces.reserve(faces.size());

    for (const auto& f : faces) {
        if (!isRegularFace(f))
            continue;
        newFaces.push_back(f);
    }

    faces.swap(newFaces);
}

int activeDegree(int v) {
    int degree = 0;
    for (int eid : verts[v].adj) {
        const Edge& e = edges[eid];
        if (!verts[e.a].active || !verts[e.b].active)
            continue;
        if (e.a == e.b)
            continue;
        ++degree;
    }
    return degree;
}

int otherEndpoint(const Edge& e, int v) {
    if (e.a == v) return e.b;
    if (e.b == v) return e.a;
    return -1;
}

Chain traceChainFromEdge(int start_eid, int start_v, std::vector<char>& edgeVisited) {
    Chain chain;

    if (start_eid < 0 || start_eid >= (int)edges.size())
        return chain;
    if (start_v < 0 || start_v >= (int)verts.size())
        return chain;
    if (!verts[start_v].active)
        return chain;

    int prev = -1;
    int curr = start_v;
    bool firstStep = true;
    int loopStart = start_v;

    while (true) {
        // 避免闭环时把起点重复 push 到结尾
        if (!chain.points.empty() && curr == loopStart && !firstStep)
            break;

        chain.points.push_back(verts[curr].pos);

        int next = -1;
        int next_eid = -1;

        if (firstStep) {
            const Edge& se = edges[start_eid];
            if (verts[se.a].active && verts[se.b].active && se.a != se.b &&
                !edgeVisited[start_eid]) {
                int forced = otherEndpoint(se, curr);
                if (forced >= 0 && forced != prev) {
                    next = forced;
                    next_eid = start_eid;
                }
            }
        }

        if (next == -1) {
            for (int eid : verts[curr].adj) {
                const Edge& e = edges[eid];
                if (!verts[e.a].active || !verts[e.b].active)
                    continue;
                if (e.a == e.b)
                    continue;
                if (edgeVisited[eid])
                    continue;

                int nb = otherEndpoint(e, curr);
                if (nb < 0)
                    continue;
                if (nb == prev)
                    continue;

                next = nb;
                next_eid = eid;
                break;
            }
        }

        if (next == -1)
            break;

        edgeVisited[next_eid] = true;
        prev = curr;
        curr = next;
        firstStep = false;

        // 开链走到端点/分叉点就结束，但不要在这里提前 push，
        // 否则和 while 开头的 push 叠加，容易把点重复塞两次。
        if (curr != loopStart && activeDegree(curr) != 2)
            continue;
    }

    return chain;
}

inline std::vector<Chain> buildSkeleton(const MeshTriangle& input) {
    verts.clear();
    faces.clear();
    edges.clear();

    for (const auto& tri : input.triangles) {
        int i0 = addVert(tri.v0);
        int i1 = addVert(tri.v1);
        int i2 = addVert(tri.v2);
        faces.push_back(Face{i0, i1, i2});
    }

    removeDegenerateFaces();
    rebuildEdgesFromFaces();

    while (hasRegular()) {
        float min_len = std::numeric_limits<float>::infinity();
        int min_idx = -1;

        for (size_t i = 0; i < edges.size(); ++i) {
            const Edge& e = edges[i];
            if (!verts[e.a].active || !verts[e.b].active)
                continue;
            if (e.a == e.b)
                continue;
            if (e.length < min_len) {
                min_len = e.length;
                min_idx = (int)i;
            }
        }

        if (min_idx < 0 || !std::isfinite(min_len))
            break;

        int v_keep = edges[min_idx].a;
        int v_rem  = edges[min_idx].b;

        if (v_keep == v_rem)
            break;
        if (!verts[v_keep].active || !verts[v_rem].active)
            break;

        // collapse 到中点，而不是把 v_rem 硬吸到 v_keep 原位置
        verts[v_keep].pos = (verts[v_keep].pos + verts[v_rem].pos) * 0.5f;
        verts[v_rem].active = false;

        collapseVertInFaces(v_keep, v_rem);
        removeDegenerateFaces();
        rebuildEdgesFromFaces();

        if (edges.empty())
            break;
    }

    std::vector<Chain> out;
    std::vector<char> edgeVisited(edges.size(), false);

    // 1) first trace open chains from endpoints / junctions
    for (int v = 0; v < (int)verts.size(); ++v) {
        if (!verts[v].active)
            continue;

        int deg = activeDegree(v);
        if (deg == 0 || deg == 2)
            continue;

        for (int eid : verts[v].adj) {
            const Edge& e = edges[eid];
            if (!verts[e.a].active || !verts[e.b].active)
                continue;
            if (e.a == e.b)
                continue;
            if (edgeVisited[eid])
                continue;

            Chain chain = traceChainFromEdge(eid, v, edgeVisited);
            if (chain.points.size() >= 2)
                out.push_back(chain);
        }
    }

    // 2) then handle remaining pure loops
    for (int eid = 0; eid < (int)edges.size(); ++eid) {
        if (edgeVisited[eid])
            continue;

        const Edge& e = edges[eid];
        if (!verts[e.a].active || !verts[e.b].active)
            continue;
        if (e.a == e.b)
            continue;

        Chain chain = traceChainFromEdge(eid, e.a, edgeVisited);
        if (chain.points.size() >= 2)
            out.push_back(chain);
    }

    return out;
}

Curve CatmullSpline(const Chain &chain, int samples) {
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
    Chain smoothed = chain;
    if (smoothed.points.size() <= 2)
        return smoothed;
    for (int k = 0; k < iter; ++k) {
        Chain prevChain = smoothed;
        for (size_t i = 1; i < prevChain.points.size() - 1; ++i) {
            smoothed.points[i] =
                (prevChain.points[i - 1] + prevChain.points[i + 1]) * 0.5f;
        }
    }

    return smoothed;
}

Curve buildCurve(const Chain &chain) {
    if (chain.points.size() == 0) return Curve();
    Chain smoothed = laplacianSmooth(chain, 3); /// harcdoded 3
    Curve spline = CatmullSpline(smoothed, 3);
    return spline;
}