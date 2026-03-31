#include "Renderer.hpp"
#include "Scene.hpp"
#include "MeshTriangle.hpp"
#include "Vector.hpp"
#include "Camera.hpp"
#include "global.hpp"
#include "Integrator.hpp"
#include <chrono>
#include <initializer_list>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Skeleton.hpp"

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{

    // Change the definition here to change resolution
    Camera camera;
    Scene scene(1200, 800, camera);

    Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material* light = new Material(DIFFUSE, Vector3f(20.0f, 8.0f, 1.0f));
    light->Kd = Vector3f(0.8f); 
    Material* mirror = new Material(MIRROR, Vector3f(0.0f));
    mirror->Ks = Vector3f(0.9f);
    mirror->specularExponent = 1000.0f;
    Material* cakeTex = new Material(DIFFUSE, Vector3f(0.0f));
    cakeTex->tex = new Texture("../assets/cake/caketex2048.png");
    Material* candleTex = new Material(DIFFUSE, Vector3f(0.0f));
    candleTex->tex = new Texture("../assets/cake/candletex256.png");

    auto addObj = [&](const std::string &path, std::initializer_list<Material*> mats) {
        objl::Loader loader;
        if (!loader.LoadFile(path)) {
            std::cerr << "Failed to load obj: " << path << std::endl;
            return;
        }
        std::vector<Material*> vm(mats);
        size_t mi = 0;
        for (auto &mesh : loader.LoadedMeshes) {
            Material* mat = nullptr;
            if (!vm.empty()) {
                if (mi < vm.size()) mat = vm[mi];
                else mat = vm.back();
            }
            scene.Add(new MeshTriangle(mesh, mat));
            ++mi;
        }
    };

    addObj("../assets/cake/cake.obj", {cakeTex});
    addObj("../assets/cake/candle.obj", {candleTex});
    addObj("../assets/cake/floor.obj", {white});
    addObj("../assets/cake/m1.obj", {mirror});
    addObj("../assets/cake/sq1.obj", {mirror});
    addObj("../assets/cake/m2.obj", {mirror});

    scene.buildBVH();

    auto integ = std::make_shared<AmbientLight>(Vector3f(0.8f));
    Renderer r(integ);

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    r.SaveImage("out.png");
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}