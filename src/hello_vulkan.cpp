#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "../include/vulkan_triangle.hpp"
using namespace VulkanApp;

int main() {
  VulkanTriangleApplication app; 

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
