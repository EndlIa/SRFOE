#include "Renderer.hpp"
#include "Scene.hpp"
#include "MeshTriangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "Camera.hpp"
#include "global.hpp"
#include "Integrator.hpp"
#include <chrono>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    Material* testTex = new Material(DIFFUSE, Vector3f(0.0f));
    testTex->tex = new Texture("../assets/ttest/test.png");
    
    MeshTriangle a123("../assets/ttest/test.obj", testTex);


    scene.Add(&a123);

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