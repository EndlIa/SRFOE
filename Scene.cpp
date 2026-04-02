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
    for (uint32_t k = 0; k < lights.size(); ++k)
        emit_area_sum += lights[k]->getArea();

    if (emit_area_sum <= EPSILON) {
        pdf = 0;
        return;
    }


    float p = get_random_float() * emit_area_sum;
    float accum_area = 0;

    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            accum_area += objects[k]->getArea();
            if (p <= accum_area){
                objects[k]->Sample(pos, pdf);
                pdf = 1.0f / emit_area_sum;
                return;
            }
        }
    }
    for (uint32_t k = 0; k < lights.size(); ++k) {
        accum_area += lights[k]->getArea();
        if (p <= accum_area){
            lights[k]->Sample(pos, pdf);
            pdf = 1.0f / emit_area_sum;
            return;
        }
    }
}


