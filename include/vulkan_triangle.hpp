#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <optional>
#include <vector>

namespace VulkanApp
{
const int MAX_FRAMES_IN_FLIGHT = 2;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
class VulkanTriangleApplication
{

public:
  uint32_t currentFrame = 0;
  bool framebufferResized = false;
  void run ();

private:
  VkDebugUtilsMessengerEXT debugMessenger;
  GLFWwindow *window;
  VkInstance instance;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;
  VkSwapchainKHR swapChain;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  VkRenderPass renderPass;
  VkPipeline graphicsPipeline;
  VkPipelineLayout pipelineLayout;
  VkCommandPool commandPool;

  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  const std::vector<const char *> validationLayers
      = { "VK_LAYER_KHRONOS_validation" };

  const std::vector<const char *> deviceExtensions
      = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool
    isComplete ()
    {
      return graphicsFamily.has_value () && presentFamily.has_value ();
    }
  } indices;

  struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } swapChainDetails;

  void initWindow ();

  void initVulkan ();

  void populateDebugMessengerCreateInfo (
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void setupDebugMessenger ();

  void createInstance ();

  void createSurface ();

  void createLogicalDevice ();

  void createSwapChain ();
  void recreateSwapChain ();
  void cleanupSwapChain ();

  void createImageViews ();

  void createGraphicsPipeline ();

  void createRenderPass ();

  void createFramebuffers ();

  void createCommandPool ();

  void createCommandBuffers ();
  void recordCommandBuffer (VkCommandBuffer buffer, uint32_t imageIndex);

  void createSyncObjects ();

  void drawFrame ();

  bool verifyExtensions (const char **glfwExtensions,
                         uint32_t glfwExtensionCount);
  bool checkValidationLayerSupport ();

  bool isDeviceSuitable (VkPhysicalDevice device);

  bool checkDeviceExtensionSupport (VkPhysicalDevice device);

  VkSurfaceFormatKHR chooseSwapSurfaceFormat (
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkPresentModeKHR chooseSwapPresentMode (
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  VkExtent2D chooseSwapExtent (const VkSurfaceCapabilitiesKHR &capabilities);

  VkShaderModule createShaderModule (const std::vector<char> &code);

  void findQueueFamilies (VkPhysicalDevice device);

  void querySwapChainSupport (VkPhysicalDevice device);

  void pickPhysicalDevice ();

  void mainLoop ();

  void cleanup ();
};
} // namespace VulkanApp
