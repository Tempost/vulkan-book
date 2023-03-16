#include "../include/vulkan_triangle.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
using namespace VulkanApp;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static std::vector<char>
readFile (const std::string &filename)
{
  // ate - start reading at end of file
  // binary - read file as binary no text transforms
  std::ifstream file (filename, std::ios::ate | std::ios::binary);

  if (!file.is_open ())
    {
      throw std::runtime_error ("Failed to open file!");
    }

  size_t fileSize = (size_t)file.tellg ();
  std::vector<char> buffer (fileSize);

  file.seekg (0);
  file.read (buffer.data (), fileSize);

  file.close ();
  return buffer;
}

// PUBLIC
void
VulkanTriangleApplication::run ()
{
  initWindow ();
  initVulkan ();
  mainLoop ();
  cleanup ();
}

// PRIVATE
void
VulkanTriangleApplication::initVulkan ()
{
  createInstance ();
  createSurface ();
  pickPhysicalDevice ();
  createLogicalDevice ();
  createSwapChain ();
  createImageViews ();
  createRenderPass ();
  createGraphicsPipeline ();
}

void
VulkanTriangleApplication::createInstance ()
{
  if (enableValidationLayers && !checkValidationLayerSupport ())
    {
      throw std::runtime_error (
          "Validation layers requested, but not available!");
    }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions
      = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;

  if (enableValidationLayers)
    {
      createInfo.enabledLayerCount
          = static_cast<uint32_t> (validationLayers.size ());
      createInfo.ppEnabledLayerNames = validationLayers.data ();
    }
  else
    {
      createInfo.enabledLayerCount = 0;
    }

  if (verifyExtensions (glfwExtensions, glfwExtensionCount))
    {
      throw std::runtime_error ("Unsupported required glfw extension.");
    }

  if (vkCreateInstance (&createInfo, nullptr, &instance))
    {
      throw std::runtime_error ("Failed to create instance!");
    }
}

void
VulkanTriangleApplication::createSurface ()
{
  if (glfwCreateWindowSurface (instance, window, nullptr, &surface)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create window surface!");
    }
}

void
VulkanTriangleApplication::createLogicalDevice ()
{
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies
      = { indices.graphicsFamily.value (), indices.presentFamily.value () };
  float queuePriority = 1.0f;

  for (uint32_t queueFamily : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back (queueCreateInfo);
    }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount
      = static_cast<uint32_t> (queueCreateInfos.size ());
  createInfo.pQueueCreateInfos = queueCreateInfos.data ();
  createInfo.queueCreateInfoCount = 1;
  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount
      = static_cast<uint32_t> (deviceExtensions.size ());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data ();

  if (enableValidationLayers)
    {
      createInfo.enabledLayerCount
          = static_cast<uint32_t> (validationLayers.size ());
      createInfo.ppEnabledLayerNames = validationLayers.data ();
    }
  else
    {
      createInfo.enabledLayerCount = 0;
    }

  if (vkCreateDevice (physicalDevice, &createInfo, nullptr, &device)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create logical device!");
    }

  vkGetDeviceQueue (device, indices.graphicsFamily.value (), 0,
                    &graphicsQueue);
  vkGetDeviceQueue (device, indices.presentFamily.value (), 0, &presentQueue);
}

void
VulkanTriangleApplication::createSwapChain ()
{
  VkSurfaceFormatKHR surfaceFormat
      = chooseSwapSurfaceFormat (swapChainDetails.formats);
  VkPresentModeKHR presentMode
      = chooseSwapPresentMode (swapChainDetails.presentModes);
  VkExtent2D extent = chooseSwapExtent (swapChainDetails.capabilities);

  // sticking to the min means we might have the wait for the driver to
  // complete ops before fetching another image to render to. min + 1 is a work
  // around
  uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;

  // Zero is special value, meaning no maxImageCount
  // checking if we excede the maxImageCount
  if (swapChainDetails.capabilities.maxImageCount > 0
      && imageCount > swapChainDetails.capabilities.maxImageCount)
    {
      imageCount = swapChainDetails.capabilities.maxImageCount;
    }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  // Might use VK_IMAGE_USAGE_TRANSFER_DST_BIT for doing post processing
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[]
      = { indices.graphicsFamily.value (), indices.presentFamily.value () };

  if (indices.graphicsFamily != indices.presentFamily)
    {
      // No ownership handling of images
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
  else
    {
      // Ownership handling
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

  // currentTransform says "Do nothing to this image"
  createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
  // Ignore alpha channel so we don't blend with windowing system
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  // Ignore hidden pixels
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR (device, &createInfo, nullptr, &swapChain)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create swap chain!");
    }

  vkGetSwapchainImagesKHR (device, swapChain, &imageCount, nullptr);
  swapChainImages.resize (imageCount);
  vkGetSwapchainImagesKHR (device, swapChain, &imageCount,
                           swapChainImages.data ());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void
VulkanTriangleApplication::createImageViews ()
{
  swapChainImageViews.resize (swapChainImages.size ());

  for (size_t i = 0; i < swapChainImages.size (); i++)
    {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

      createInfo.image = swapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;

      // Swizzle color channels around, aka mapping different channels
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      // Define image purpose and image access
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView (device, &createInfo, nullptr,
                             &swapChainImageViews[i])
          != VK_SUCCESS)
        {
          throw std::runtime_error ("Failed to create image views!");
        }
    }
}

void
VulkanTriangleApplication::createGraphicsPipeline ()
{
  auto vertShaderCode = readFile ("shaders/vert.spv");
  auto fragShaderCode = readFile ("shaders/frag.spv");

  VkShaderModule vertShaderModule = createShaderModule (vertShaderCode);
  VkShaderModule fragShaderModule = createShaderModule (fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[]
      = { vertShaderStageInfo, fragShaderStageInfo };

  std::vector<VkDynamicState> dynamicStates
      = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount
      = static_cast<uint32_t> (dynamicStates.size ());
  dynamicState.pDynamicStates = dynamicStates.data ();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  // We hardcoded vertext data into the shader so we don't need to fill in
  // anything else in this struct
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType
      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  // Since we are 'hand' drawing triangles these settings are fine
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)swapChainExtent.width;
  viewport.height = (float)swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = { 0, 0 };
  scissor.extent = swapChainExtent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType
      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  // VK_TRUE here requires GPU feature
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  // Anything aside from VK_POLYGON_MODE_FILL required GPU feature
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  // lineWidth higher than 1.0f required wideLines GPU feature
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType
      = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  // Here to alphaToOneEnable are optional
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // NOTE: if blendEnable is set to VK_FALSE then the color from the frag
  // shader is passed through unmodified
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask
      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  // NOTE: If wanting the implment alpha color blending
  /* colorBlendAttachment.blendEnable = VK_TRUE; */
  /* colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; */
  /* colorBlendAttachment.dstColorBlendFactor =
   * VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; */
  /* colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; */
  /* colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; */
  /* colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; */
  /* colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; */

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType
      = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  if (vkCreatePipelineLayout (device, &pipelineLayoutInfo, nullptr,
                              &pipelineLayout)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create pipeline layout");
    }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = nullptr;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  if (vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                 nullptr, &graphicsPipeline)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create graphics pipeline!");
    }

  vkDestroyShaderModule (device, fragShaderModule, nullptr);
  vkDestroyShaderModule (device, vertShaderModule, nullptr);
}

void
VulkanTriangleApplication::createRenderPass ()
{
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  if (vkCreateRenderPass (device, &renderPassInfo, nullptr, &renderPass)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create render pass!");
    }
}

bool
VulkanTriangleApplication::verifyExtensions (const char **glfwExtensions,
                                             uint32_t glfwExtensionCount)
{
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions (extensionCount);

  vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount,
                                          extensions.data ());

  for (uint32_t i{}; i < glfwExtensionCount; i++)
    {
      for (const auto &extension : extensions)
        {
          if (strcmp (extension.extensionName, glfwExtensions[i]) == 0)
            {
              return false;
            }
        }
    }

  return true;
}

bool
VulkanTriangleApplication::checkValidationLayerSupport ()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties (&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers (layerCount);
  vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());
  for (const char *layerName : validationLayers)
    {
      bool layerFound = false;

      for (const auto &layerProperties : availableLayers)
        {
          if (strcmp (layerName, layerProperties.layerName) == 0)
            {
              layerFound = true;
              break;
            }
        }
      if (!layerFound)
        {
          return false;
        }
    }

  return true;
}

// This can be extended to "rate" the different devices
// giving a descrete GPU a higher score or one with more features a higher
// score
bool
VulkanTriangleApplication::isDeviceSuitable (VkPhysicalDevice device)
{
  findQueueFamilies (device);
  bool extensionsSupported = checkDeviceExtensionSupport (device);

  bool swapChainAdequate = false;
  if (extensionsSupported)
    {
      querySwapChainSupport (device);
      swapChainAdequate = !swapChainDetails.formats.empty ()
                          && !swapChainDetails.presentModes.empty ();
    }

  return indices.isComplete () && extensionsSupported && swapChainAdequate;
}

bool
VulkanTriangleApplication::checkDeviceExtensionSupport (
    VkPhysicalDevice device)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties (device, nullptr, &extensionCount,
                                        nullptr);

  std::vector<VkExtensionProperties> availableExtensions (extensionCount);
  vkEnumerateDeviceExtensionProperties (device, nullptr, &extensionCount,
                                        availableExtensions.data ());

  std::set<std::string> requiredExtensions (deviceExtensions.begin (),
                                            deviceExtensions.end ());

  for (const auto &extension : availableExtensions)
    {
      requiredExtensions.erase (extension.extensionName);
    }

  return requiredExtensions.empty ();
}

VkSurfaceFormatKHR
VulkanTriangleApplication::chooseSwapSurfaceFormat (
    const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
  for (const auto &availableFormat : availableFormats)
    {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
          && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
          return availableFormat;
        }
    }

  return availableFormats[0];
}

VkPresentModeKHR
VulkanTriangleApplication::chooseSwapPresentMode (
    const std::vector<VkPresentModeKHR> &availablePresentModes)
{
  for (const auto &availablePresentMode : availablePresentModes)
    {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
          return availablePresentMode;
        }
    }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanTriangleApplication::chooseSwapExtent (
    const VkSurfaceCapabilitiesKHR &capabilities)
{
  if (capabilities.currentExtent.width
      != std::numeric_limits<uint32_t>::max ())
    {
      return capabilities.currentExtent;
    }
  else
    {
      int width, height;
      glfwGetFramebufferSize (window, &width, &height);

      VkExtent2D actualExtent
          = { static_cast<uint32_t> (width), static_cast<uint32_t> (height) };

      actualExtent.width
          = std::clamp (actualExtent.width, capabilities.minImageExtent.width,
                        capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp (actualExtent.height,
                                        capabilities.minImageExtent.height,
                                        capabilities.maxImageExtent.height);

      return actualExtent;
    }
}

VkShaderModule
VulkanTriangleApplication::createShaderModule (const std::vector<char> &code)
{
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size ();
  createInfo.pCode = reinterpret_cast<const uint32_t *> (code.data ());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule (device, &createInfo, nullptr, &shaderModule)
      != VK_SUCCESS)
    {
      throw std::runtime_error ("Failed to create shader module!");
    }

  return shaderModule;
}

void
VulkanTriangleApplication::findQueueFamilies (VkPhysicalDevice device)
{
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties (device, &queueFamilyCount,
                                            nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties (device, &queueFamilyCount,
                                            queueFamilies.data ());

  int i = 0;
  for (const auto &queueFamily : queueFamilies)
    {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR (device, i, surface,
                                            &presentSupport);
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
          indices.graphicsFamily = i;
        }

      if (presentSupport)
        {
          indices.presentFamily = i;
        }

      if (indices.isComplete ())
        {
          break;
        }

      i++;
    }
}

void
VulkanTriangleApplication::querySwapChainSupport (VkPhysicalDevice device)
{
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR (device, surface,
                                             &swapChainDetails.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount,
                                        nullptr);

  if (formatCount != 0)
    {
      swapChainDetails.formats.resize (formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount,
                                            swapChainDetails.formats.data ());
    }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface,
                                             &presentModeCount, nullptr);

  if (presentModeCount != 0)
    {
      swapChainDetails.presentModes.resize (presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR (
          device, surface, &presentModeCount,
          swapChainDetails.presentModes.data ());
    }
}

void
VulkanTriangleApplication::pickPhysicalDevice ()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);

  if (deviceCount == 0)
    {
      throw std::runtime_error ("Failed to find GPUs with vulkan support!");
    }

  std::vector<VkPhysicalDevice> devices (deviceCount);
  vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());
  for (const auto &device : devices)
    {
      if (isDeviceSuitable (device))
        {
          physicalDevice = device;
          break;
        }

      if (physicalDevice == VK_NULL_HANDLE)
        {
          throw std::runtime_error ("Failed to find a suitable GPU!");
        }
    }
}

void
VulkanTriangleApplication::initWindow ()
{
  glfwInit ();
  glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow (WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void
VulkanTriangleApplication::mainLoop ()
{
  while (!glfwWindowShouldClose (window))
    {
      glfwPollEvents ();
    }
}

void
VulkanTriangleApplication::cleanup ()
{
  vkDestroyPipeline (device, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout (device, pipelineLayout, nullptr);
  vkDestroyRenderPass (device, renderPass, nullptr);

  for (auto imageView : swapChainImageViews)
    {
      vkDestroyImageView (device, imageView, nullptr);
    }

  vkDestroySwapchainKHR (device, swapChain, nullptr);

  vkDestroyDevice (device, nullptr);

  vkDestroySurfaceKHR (instance, surface, nullptr);

  vkDestroyInstance (instance, nullptr);

  glfwDestroyWindow (window);

  glfwTerminate ();
}
