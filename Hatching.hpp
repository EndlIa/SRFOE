#include "Skeleton.hpp"
#include "Scene.hpp"


Chain intersectPlaneMesh(const Vector3f &point, const Vector3f &normal, const MeshTriangle &mesh) {
    Chain hatchLine;
    return hatchLine;
}
//plane-mesh intersection to get hatchingLines
std::vector<Chain> buildHatchLines(const std::vector<Curve> &curves) {
    std::vector<Chain> out;
    for (const auto &curve : curves) {
        for (size_t i = 0; i < curve.points.size() - 1; ++i) {

            Vector3f point = curve.points[i];
            Vector3f normal = curve.tangents[i];
            
            Chain hatchLine = intersectPlaneMesh(point, normal, scene::get_objects()[0]); // Assuming the first object is the mesh
            out.push_back(hatchLine);
        }
    }
    return out;
}
//draw hatchingLines