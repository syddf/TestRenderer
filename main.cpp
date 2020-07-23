#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

using Vec3 = glm::vec3;
using Point = glm::vec3;

#define EPSILON 0.0000001f

struct Ray
{
public:
    Point Origin;
    Vec3 Direction;
};

struct Triangle
{
private:
    Point P0;
    Point P1;
    Point P2;
    
    bool Intersect(Point& intersection, const Ray& ray)
    {
        Vec3 e1 = P1 - P0;
        Vec3 e2 = P2 - P0;
        
        Vec3 P = glm::cross(ray.Direction, e2);
        float det = glm::dot(e1, P);
        if (det > -EPSILON && det < EPSILON)
        {
            return false;
        }
        float inv_det = 1.f / det;
        Vec3 T = ray.Origin - P0;
        float u = glm::dot(T, P) * inv_det;
        if (u < 0.f || u > 1.f)
        {
            return false;
        }
        Vec3 Q = glm::cross(T, e1);
        float v = glm::dot(ray.Direction, Q) * inv_det;
        if (v < 0.f || u + v  > 1.f)
        {
            return false;
        }
        float t = glm::dot(e2, Q) * inv_det;
        
        if (t > EPSILON)
        {
            intersection = ray.Origin + t * ray.Direction;
            return true;
        }
        
        return false;
    }
    
};


int main() {
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    std::cout << extensionCount << " extensions supported\n";
    
    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;
    
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    
    return 0;
}
