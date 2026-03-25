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
#include "stb_image_write.hpp"

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
    
    MeshTriangle c1("../model/c1.obj", light);
    MeshTriangle c2("../model/c2.obj", light);
    MeshTriangle c3("../model/c3.obj", light);
    MeshTriangle c4("../model/c4.obj", light);
    MeshTriangle cake("../model/cake.obj", white);
    MeshTriangle candle("../model/candle.obj", white);
    MeshTriangle fire("../model/fire.obj", light);
    MeshTriangle floor("../model/floor.obj", white);
    MeshTriangle m1("../model/mirror.001.obj", mirror);
    MeshTriangle m2("../model/mirror.002.obj", mirror);
    MeshTriangle m3("../model/mirror.obj", mirror);

    scene.Add(&c1);
    scene.Add(&c2);
    scene.Add(&c3);
    scene.Add(&c4);
    scene.Add(&cake);
    scene.Add(&candle);
    scene.Add(&fire);
    scene.Add(&floor);
    scene.Add(&m1);
    scene.Add(&m2);
    scene.Add(&m3);
    scene.buildBVH();


    auto integ = std::make_shared<PathTracer>();
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