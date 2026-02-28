#include "Object.hpp"
#include <eigen3/Eigen/Dense>
#pragma once
class Instance 
{
public:
    Object *obj;
    Eigen::Matrix4f M; 
    Instance() = default;
    Instance(Object *obj) : obj(obj), M(Eigen::Matrix4f::Identity()) {}
    Instance(Object *obj, const Eigen::Matrix4f &M) : obj(obj), M(M) {}
};