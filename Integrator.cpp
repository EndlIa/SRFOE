#include "Integrator.hpp"

Vector3f PathTracer::Li(const Ray &ray, const Scene &scene, int depth) const
{
    Intersection inter = scene.intersect(ray);
    if (!inter.happened) {
        return scene.backgroundColor;
    }
    if (inter.m->hasEmission()) {
        return inter.m->getEmission(); ///return directly for emissive material?
    }

    Vector3f dir_light = {0., 0., 0.};
    Intersection lightInter;
    float pdf_light = 0.0f;
    scene.sampleLight(lightInter, pdf_light);
    Vector3f lightDir = (lightInter.coords - inter.coords).normalized(); ///hitpoint->light
    Ray lightRay(inter.coords + inter.normal * EPSILON, lightDir);
    float lightDis = (lightInter.coords - inter.coords).norm();
    Intersection shadowInter = scene.intersect(lightRay);
    if ((shadowInter.happened && shadowInter.distance < lightDis-0.001) || pdf_light < EPSILON) { ///?
        dir_light = {0., 0., 0.};
    } else {
        float invDis2 = 1.0f / dotProduct(lightInter.coords - inter.coords, lightInter.coords - inter.coords);
        dir_light = lightInter.emit.cwiseProduct(inter.m->eval(-lightDir, -ray.direction, inter.normal, inter.tcoords)) * std::max(0.f, dotProduct(inter.normal, lightDir)) * std::max(0.f, dotProduct(lightInter.normal, -lightDir))
         * invDis2 / pdf_light;
        ///a question: cwise product for color and BRDF?
    }

    if(get_random_float() > RussianRoulette || depth >= maxDepth) {
        return dir_light;
    }
    Vector3f dir_indirect = {0., 0., 0.};
    Vector3f wi = inter.m->sample(ray.direction, inter.normal); ///hitpoint->wi
    Ray indirectRay(inter.coords + inter.normal * EPSILON, wi);
    Intersection interIndirect = scene.intersect(indirectRay);
    if(interIndirect.happened && !interIndirect.obj->hasEmit()){
        float pdf_indirect = inter.m->pdf(-wi, -ray.direction, inter.normal);
        if (pdf_indirect > EPSILON){
            dir_indirect = Li(indirectRay, scene, depth + 1).cwiseProduct(inter.m->eval(-wi, -ray.direction, inter.normal, inter.tcoords)) * std::max(0.f, dotProduct(inter.normal, wi))
             / pdf_indirect / RussianRoulette;
        }
    }
    return dir_light + dir_indirect;
}

Vector3f AmbientLight::Li(const Ray &ray, const Scene &scene, int depth) const
{
    Intersection inter = scene.intersect(ray);
    if (!inter.happened) {
        return scene.backgroundColor;
    }
    if (inter.m->hasEmission()) {
        return inter.m->getEmission(); ///return directly for emissive material?
    }

    Vector3f dir_light = {0., 0., 0.};
    Intersection lightInter;
    float pdf_light = 0.0f;
    scene.sampleLight(lightInter, pdf_light);
    Vector3f lightDir = (lightInter.coords - inter.coords).normalized(); ///hitpoint->light
    Ray lightRay(inter.coords + inter.normal * EPSILON, lightDir);
    float lightDis = (lightInter.coords - inter.coords).norm();
    Intersection shadowInter = scene.intersect(lightRay);
    if ((shadowInter.happened && shadowInter.distance < lightDis-0.001) || pdf_light < EPSILON) { ///?
        dir_light = {0., 0., 0.};
    } else {
        float invDis2 = 1.0f / dotProduct(lightInter.coords - inter.coords, lightInter.coords - inter.coords);
        dir_light = lightInter.emit.cwiseProduct(inter.m->eval(-lightDir, -ray.direction, inter.normal, inter.tcoords)) * std::max(0.f, dotProduct(inter.normal, lightDir)) * std::max(0.f, dotProduct(lightInter.normal, -lightDir))
         * invDis2 / pdf_light;
        ///a question: cwise product for color and BRDF?
    }

    if(get_random_float() > RussianRoulette || depth >= maxDepth) {
        return dir_light;
    }
    Vector3f dir_indirect = {0., 0., 0.};
    Vector3f wi = inter.m->sample(ray.direction, inter.normal); ///hitpoint->wi
    Ray indirectRay(inter.coords + inter.normal * EPSILON, wi);
    Intersection interIndirect = scene.intersect(indirectRay);
    if(interIndirect.happened && !interIndirect.obj->hasEmit()){
        float pdf_indirect = inter.m->pdf(-wi, -ray.direction, inter.normal);
        if (pdf_indirect > EPSILON){
            dir_indirect = Li(indirectRay, scene, depth + 1).cwiseProduct(inter.m->eval(-wi, -ray.direction, inter.normal, inter.tcoords)) * std::max(0.f, dotProduct(inter.normal, wi))
             / pdf_indirect / RussianRoulette;
        }
    }
    return dir_light + dir_indirect + ambientColor.cwiseProduct(inter.m->eval(inter.normal, -ray.direction, inter.normal, inter.tcoords)); 
}