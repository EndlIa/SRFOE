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
inline objl::Mesh buildSkeleton(const MeshTriangle &input)
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

        // 坍缩 edge best_idx：合并 b 到 a（按 1A，保留 a 的坐标）
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

    // 4) 从剩下的“退化面”中提取线段（按 5A：提取非重复边并合并重复线段）
    // 使用 set 存储无向边（排序的索引对）
    struct PairHash { size_t operator()(const std::pair<int,int>&p) const noexcept { return std::hash<long long>()(((long long)p.first<<32) ^ (long long)p.second); } };
    std::unordered_set<std::pair<int,int>, PairHash> segs;
    for (auto &f : faces) {
        // 如果是 regular 则跳过
        if (isRegularFace(f)) continue;
        // 考虑三条边，若边的两个端点仍然 active 且不相同，则加入
        std::pair<int,int> e0 = {std::min(f.a,f.b), std::max(f.a,f.b)};
        if (verts[f.a].active && verts[f.b].active && e0.first != e0.second) segs.insert(e0);
        std::pair<int,int> e1 = {std::min(f.b,f.c), std::max(f.b,f.c)};
        if (verts[f.b].active && verts[f.c].active && e1.first != e1.second) segs.insert(e1);
        std::pair<int,int> e2 = {std::min(f.c,f.a), std::max(f.c,f.a)};
        if (verts[f.c].active && verts[f.a].active && e2.first != e2.second) segs.insert(e2);
    }

    // 构建输出 objl::Mesh：按 5A 合并重复线段（segs 已去重）
    objl::Mesh out;
    std::vector<int> idx_map(verts.size(), -1);
    for (auto &p : segs) {
        int a = p.first, b = p.second;
        if (idx_map[a] == -1) {
            objl::Vertex v; v.Position = objl::Vector3(verts[a].pos.x, verts[a].pos.y, verts[a].pos.z);
            v.Normal = objl::Vector3(0,0,0);
            v.TextureCoordinate = objl::Vector2(0,0);
            idx_map[a] = (int)out.Vertices.size();
            out.Vertices.push_back(v);
        }
        if (idx_map[b] == -1) {
            objl::Vertex v; v.Position = objl::Vector3(verts[b].pos.x, verts[b].pos.y, verts[b].pos.z);
            v.Normal = objl::Vector3(0,0,0);
            v.TextureCoordinate = objl::Vector2(0,0);
            idx_map[b] = (int)out.Vertices.size();
            out.Vertices.push_back(v);
        }
        out.Indices.push_back(idx_map[a]);
        out.Indices.push_back(idx_map[b]);
    }

    return out;
}
