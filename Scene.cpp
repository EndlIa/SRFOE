//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter = intersect(ray);
    if (!inter.happened) {
        return backgroundColor;
    }
    if (inter.m->hasEmission()) {
        return inter.m->getEmission(); ///return directly for emissive material?
    }

    Vector3f dir_light = {0., 0., 0.};
    Intersection lightInter;
    float pdf_light = 0.0f;
    sampleLight(lightInter, pdf_light);
    Vector3f lightDir = (lightInter.coords - inter.coords).normalized(); ///hitpoint->light
    Ray lightRay(inter.coords + inter.normal * EPSILON, lightDir);
    float lightDis = (lightInter.coords - inter.coords).norm();
    Intersection shadowInter = intersect(lightRay);
    if ((shadowInter.happened && shadowInter.distance < lightDis-0.001) || pdf_light < EPSILON) { ///?
        dir_light = {0., 0., 0.};
    } else {
        float invDis2 = 1.0f / dotProduct(lightInter.coords - inter.coords, lightInter.coords - inter.coords);
        dir_light = lightInter.emit * inter.m->eval(-lightDir, -ray.direction, inter.normal) * std::max(0.f, dotProduct(inter.normal, lightDir)) * std::max(0.f, dotProduct(lightInter.normal, -lightDir))
         * invDis2 / pdf_light;
        ///a question: cwise product for color and BRDF?
    }

    if(get_random_float() > RussianRoulette || depth >= maxDepth) {
        return dir_light;
    }
    Vector3f dir_indirect = {0., 0., 0.};
    Vector3f wi = inter.m->sample(ray.direction, inter.normal); ///hitpoint->wi
    Ray indirectRay(inter.coords + inter.normal * EPSILON, wi);
    Intersection interIndirect = intersect(indirectRay);
    if(interIndirect.happened && !interIndirect.obj->hasEmit()){
        float pdf_indirect = inter.m->pdf(-wi, -ray.direction, inter.normal);
        if (pdf_indirect > EPSILON){
            dir_indirect = castRay(indirectRay, depth + 1) * inter.m->eval(-wi, -ray.direction, inter.normal) * std::max(0.f, dotProduct(inter.normal, wi))
             / pdf_indirect / RussianRoulette;
        }
    }
    return dir_light + dir_indirect;
}