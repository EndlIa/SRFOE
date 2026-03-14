//
// Created by Göksu Güvendiren on 2019-05-14.
//

#pragma once

#include <vector>
#include "Vector.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "AreaLight.hpp"
#include "BVH.hpp"
#include "Ray.hpp"
#include "Camera.hpp"


class Scene
{
public:
    // setting up options
    int width = 1280;
    int height = 960;
    Camera camera;
    Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);

    Scene(int w, int h, const Camera& cam = Camera()) : width(w), height(h), camera(cam)
    {}

    void Add(Object *object) { objects.push_back(object); }
    void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }

    const std::vector<Object*>& get_objects() const { return objects; }
    const std::vector<std::unique_ptr<Light> >&  get_lights() const { return lights; }
    Intersection intersect(const Ray& ray) const;
    BVHAccel *bvh;
    void buildBVH();
    void sampleLight(Intersection &pos, float &pdf) const;

    // creating the scene (adding objects and lights)
    std::vector<Object* > objects;
    std::vector<std::unique_ptr<Light> > lights;
};