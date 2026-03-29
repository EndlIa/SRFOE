#pragma once

#include "MeshTriangle.hpp"
#include "OBJ_Loader.hpp"
#include "Vector.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_set>

// 对外接口：从一个 MeshTriangle 提取骨架（线段集合）并以 objl::Mesh 返回
// 实现基于用户选择：1A,2C,3C,4A,5A
inline objl::Mesh getSkeleton(const MeshTriangle &input)
{
    struct Vert { Vector3f pos; bool active = true; };
    struct Edge { int a, b; float length; };
    struct Face { int a, b, c; };

    const float EPS = 1e-6f;

    // 1) 从 input.triangles 构建顶点表、面表（合并相同坐标的顶点）
    std::vector<Vert> verts;
    std::vector<Face> faces;

    auto find_or_add = [&](const Vector3f &p)->int{
        for (size_t i = 0; i < verts.size(); ++i) {
            Vector3f d = verts[i].pos - p;
            if (d.norm() < EPS) return (int)i;
        }
        verts.push_back(Vert{p, true});
        return (int)verts.size() - 1;
    };

    for (auto &tri : input.triangles) {
        int i0 = find_or_add(tri.v0);
        int i1 = find_or_add(tri.v1);
        int i2 = find_or_add(tri.v2);
        faces.push_back(Face{i0, i1, i2});
    }

    // 边表（无向，去重）
    std::vector<Edge> edges;
    auto add_edge = [&](int a, int b){
        if (a == b) return;
        if (a > b) std::swap(a,b);
        for (auto &e : edges) if (e.a == a && e.b == b) return;
        Edge ne; ne.a = a; ne.b = b;
        ne.length = (verts[a].pos - verts[b].pos).norm();
        edges.push_back(ne);
    };

    for (auto &f : faces) {
        add_edge(f.a, f.b);
        add_edge(f.b, f.c);
        add_edge(f.c, f.a);
    }

    auto is_regular_face = [&](const Face &f)->bool{
        if (!verts[f.a].active || !verts[f.b].active || !verts[f.c].active) return false;
        if (f.a == f.b || f.b == f.c || f.c == f.a) return false;
        return true;
    };

    auto compute_valences = [&](){
        std::vector<int> val(verts.size(), 0);
        for (auto &e : edges) {
            if (!verts[e.a].active || !verts[e.b].active) continue;
            if (e.a == e.b) continue;
            val[e.a]++;
            val[e.b]++;
        }
        return val;
    };

    // 3) 循环坍缩，直到没有正常面（按选择 4A）
    auto has_regular = [&]()->bool{
        for (auto &f : faces) if (is_regular_face(f)) return true;
        return false;
    };

    // 主循环
    while (has_regular()) {
        // 找到当前最短边（考虑端点都仍然 active）
        float min_len = std::numeric_limits<float>::infinity();
        for (auto &e : edges) {
            if (!verts[e.a].active || !verts[e.b].active) continue;
            if (e.a == e.b) continue;
            if (e.length < min_len) min_len = e.length;
        }
        if (!std::isfinite(min_len)) break;

        // 收集几乎相等的最短边，按 2C 策略使用端点度数（valence）做 tie-break
        std::vector<int> cand_idxs;
        for (size_t i = 0; i < edges.size(); ++i) {
            auto &e = edges[i];
            if (!verts[e.a].active || !verts[e.b].active) continue;
            if (e.a == e.b) continue;
            if (std::fabs(e.length - min_len) < 1e-6f) cand_idxs.push_back((int)i);
        }

        auto val = compute_valences();
        int best_idx = -1;
        int best_valence = std::numeric_limits<int>::max();
        for (int idx : cand_idxs) {
            int s = val[edges[idx].a] + val[edges[idx].b];
            if (s < best_valence) { best_valence = s; best_idx = idx; }
        }
        if (best_idx == -1) break;

        // 坍缩 edge best_idx：合并 b 到 a（按 1A，保留 a 的坐标）
        int v_keep = edges[best_idx].a;
        int v_rem = edges[best_idx].b;
        if (v_keep == v_rem) { verts[v_rem].active = false; continue; }

        verts[v_rem].active = false;

        // 所有连接到 v_rem 的边改连到 v_keep（按题意）
        for (auto &e : edges) {
            if (e.a == v_rem) e.a = v_keep;
            if (e.b == v_rem) e.b = v_keep;
        }

        // 按 3C 策略：不立刻删除重复边/自环，只有在需要时才处理。
        // 更新受影响边的长度（与 v_keep 有关联的边）
        for (auto &e : edges) {
            if (!verts[e.a].active || !verts[e.b].active) { e.length = 0.f; continue; }
            if (e.a == v_keep || e.b == v_keep) {
                if (e.a == e.b) { e.length = 0.f; }
                else e.length = (verts[e.a].pos - verts[e.b].pos).norm();
            }
        }
    }

    // 4) 从剩下的“退化面”中提取线段（按 5A：提取非重复边并合并重复线段）
    // 使用 set 存储无向边（排序的索引对）
    struct PairHash { size_t operator()(const std::pair<int,int>&p) const noexcept { return std::hash<long long>()(((long long)p.first<<32) ^ (long long)p.second); } };
    std::unordered_set<std::pair<int,int>, PairHash> segs;
    for (auto &f : faces) {
        // 如果是 regular 则跳过
        if (is_regular_face(f)) continue;
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
