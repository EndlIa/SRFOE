
#include <fstream>
#include <thread>   
#include <mutex>
#include <string>
#include "Scene.hpp"
#include "Renderer.hpp"
#include "Integrator.hpp"
#include "stb_image_write.hpp"

const float EPSILON = 0.000015;

void Renderer::Render(const Scene& scene)
{
    width = scene.width;
    height = scene.height;
    framebuffer.assign(width * height, Vector3f(0.f));

    int spp = 50;
    int thread_bum = 8;
    int rows_per_thread = height / thread_bum;
    std::vector<std::thread> threads(thread_bum);
    std::mutex progress_mutex;
    float progress = 0.f;
    std::cout << "SPP: " << spp << "\n";
    std::cout << "Thread number: " << thread_bum << "\n";

    auto castRayThread = [&](int id)
    {
        for (uint32_t j = id * rows_per_thread; j < (id + 1) * rows_per_thread; ++j){
            for (uint32_t i = 0; i < scene.width; ++i){
                Ray primaryRay = scene.camera.GenerateRay(i, j, scene.width, scene.height);
                for (int k = 0; k < spp; k++){
                    framebuffer[j * scene.width + i] += integrator->Li(primaryRay, scene, 0) / spp;
                }
            }
            progress_mutex.lock();
            progress += 1.f / scene.height;
            UpdateProgress(progress);
            progress_mutex.unlock();
        }
    };
    for(int i = 0; i < thread_bum; i++){
        threads[i] = std::thread(castRayThread, i);
    }
    for(int i = 0; i < thread_bum; i++){
        threads[i].join();
    }
    UpdateProgress(1.f);
}

void Renderer::SaveImage(const std::string& filename, float gamma) const
{
    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < width * height; ++i) {
        img[i * 3 + 0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), gamma));
        img[i * 3 + 1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), gamma));
        img[i * 3 + 2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), gamma));
    }
    stbi_write_png(filename.c_str(), width, height, 3, img.data(), width * 3);
}
