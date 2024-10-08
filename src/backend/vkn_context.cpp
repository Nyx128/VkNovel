#define VMA_IMPLEMENTATION
#include "vkn_context.hpp"

#include "vulkan/vulkan_to_string.hpp"
#include <vector>
#include <iostream>
#include <sstream>

#ifdef _DEBUG
PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pMessenger){
	return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator){
	return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageFunc(VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT              messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void* pUserData)
{
    std::ostringstream message;
    message << "___________________________________________________________________________\n";
    message << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": "
        << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ":\n";
    message << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
    message << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
    message << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";

    if (0 < pCallbackData->queueLabelCount)
    {
        message << std::string("\t") << "Queue Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++)
        {
            message << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->cmdBufLabelCount)
    {
        message << std::string("\t") << "CommandBuffer Labels:\n";
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
        {
            message << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
    }
    if (0 < pCallbackData->objectCount)
    {
        message << std::string("\t") << "Objects:\n";
        for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
        {
            message << std::string("\t\t") << "Object " << i << "\n";
            message << std::string("\t\t\t") << "objectType   = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
            message << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
            if (pCallbackData->pObjects[i].pObjectName)
            {
                message << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
            }
        }
    }
    message << "___________________________________________________________________________\n";
    std::cout << message.str() << std::endl;
    
    return false;
}
#endif

namespace vkn {
	Context::Context(ContextInfo& info, vkn::Window& _window):ctxInfo(info), window(_window){
        initInstance();
        initDispatchLoader();
        initDevice();
        initMemoryAllocator();
        initSwapchain();
        initSwapchainImageViews();
        initSwapchainDepth();
        initCommandPool();
	}

	Context::~Context(){

        device.destroyCommandPool(singleTimeCommandPool);

        device.destroyImageView(swapchainDepthView);
        vmaDestroyImage(allocator, swapchainDepthImage, depthImageAlloc);

        for (auto& view : swapchainImageViews) {
            device.destroyImageView(view);
        }

        device.destroySwapchainKHR(swapchain);
        vmaDestroyAllocator(allocator);
        device.destroy();

        instance.destroySurfaceKHR(surface);
#ifdef _DEBUG
        instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
		instance.destroy();
	}

    void Context::beginSingleTimeCommandBuffer(vk::CommandBuffer& commandBuffer){
        vk::CommandBufferAllocateInfo allocInfo(singleTimeCommandPool, vk::CommandBufferLevel::ePrimary, 1);

        device.allocateCommandBuffers(&allocInfo, &commandBuffer);

        vk::CommandBufferBeginInfo beginInfo;
       
        commandBuffer.begin(beginInfo);
    }

    void Context::endSingleTimeCommandBuffer(vk::CommandBuffer& commandBuffer){
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        graphicsQueue.submit(submitInfo);

        device.waitIdle();
        device.freeCommandBuffers(singleTimeCommandPool, 1, &commandBuffer);
    }

    void Context::initInstance(){
        vk::ApplicationInfo appInfo(ctxInfo.app_name.c_str(), ctxInfo.appVersion, "VkNovel", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2);
        //Instance creation

        //Debug support query
        std::vector<vk::ExtensionProperties> props = vk::enumerateInstanceExtensionProperties();

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        for (int i = 0; i < glfwExtensionCount; i++) {
            instanceExtensions.push_back(glfwExtensions[i]);
        }

#ifdef _DEBUG
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        for (const auto& ext : instanceExtensions) {
            auto propertyIterator = std::find_if(
                props.begin(), props.end(), [ext](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, ext) == 0; });
            if (propertyIterator == props.end()) {
                std::cout << "Cannot find required extension " << ext << " extension" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
       

        std::vector<vk::LayerProperties> instanceLayerProperties = vk::enumerateInstanceLayerProperties();

        bool layersAvailable = std::all_of(instanceLayers.begin(),
            instanceLayers.end(),
            [&instanceLayerProperties](char const* name)
            {
                return std::any_of(instanceLayerProperties.begin(),
                instanceLayerProperties.end(),
                [&name](vk::LayerProperties const& property) { return strcmp(property.layerName, name) == 0; });
            });

        if (!layersAvailable) {
            throw std::runtime_error("Required layers are not available");
        }


        vk::InstanceCreateInfo instanceInfo{};
        instanceInfo.enabledExtensionCount = instanceExtensions.size();
        instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
        instanceInfo.enabledLayerCount = instanceLayers.size();
        instanceInfo.ppEnabledLayerNames = instanceLayers.data();
        instanceInfo.setPApplicationInfo(&appInfo);

        instance = vk::createInstance(instanceInfo);

#ifdef _DEBUG
        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        if (!pfnVkCreateDebugUtilsMessengerEXT)
        {
            std::cout << "GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function." << std::endl;
            exit(1);
        }

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
        if (!pfnVkDestroyDebugUtilsMessengerEXT)
        {
            std::cout << "GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function." << std::endl;
            exit(1);
        }

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            );
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        debugMessengerCreateInfo.messageSeverity = severityFlags;
        debugMessengerCreateInfo.messageType = messageTypeFlags;
        debugMessengerCreateInfo.pfnUserCallback = debugMessageFunc;

        debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo);
#endif
        VkSurfaceKHR _surface;
        glfwCreateWindowSurface(static_cast<VkInstance>(instance), window.getHandle(), nullptr, &_surface);
        surface = vk::SurfaceKHR(_surface);
    }

    void Context::initDispatchLoader(){
        dLoader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
    }

    void Context::initDevice(){
        std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

        if (physicalDevices.empty()) {
            throw std::runtime_error("Failed to find any GPU with vulkan support");
        }

        vk::DeviceSize maxSize = 0;
        //pick the GPU with maximum vram

        std::cout << "Available GPUs:" << std::endl;
        std::vector<vk::PhysicalDevice> dgpus;

        for (const auto& gpu : physicalDevices) {
            if (gpu.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                dgpus.push_back(gpu);
            }
        }

        if (!dgpus.empty()) {
            //if discrete is available, only pick from those
            for (const auto& gpu : dgpus) {
                vk::PhysicalDeviceMemoryProperties memoryProperties = gpu.getMemoryProperties();

                vk::DeviceSize totalVram = 0;

                for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
                    if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                        totalVram += memoryProperties.memoryHeaps[i].size;
                    }
                }

                std::cout << "Device name: " << gpu.getProperties().deviceName << "| VRAM: " << totalVram / (1024 * 1024) << "MB" << std::endl;

                if (totalVram > maxSize) {
                    maxSize = totalVram;
                    physicalDevice = gpu;
                }
            }

            std::cout << "\n\nchosen device: " << physicalDevice.getProperties().deviceName << std::endl;
        }

        else {
            //in case discrete isnt available, choose from general
            for (const auto& gpu : physicalDevices) {
                vk::PhysicalDeviceMemoryProperties memoryProperties = gpu.getMemoryProperties();

                vk::DeviceSize totalVram = 0;

                for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
                    if (memoryProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
                        totalVram += memoryProperties.memoryHeaps[i].size;
                    }
                }

                std::cout << "Device name: " << gpu.getProperties().deviceName << "| VRAM: " << totalVram / (1024 * 1024) << "MB" << std::endl;

                if (totalVram > maxSize) {
                    maxSize = totalVram;
                    physicalDevice = gpu;
                }
            }

            std::cout << "\n\nchosen device: " << physicalDevice.getProperties().deviceName << std::endl;
        }

        //query for some features

        auto features2 = physicalDevice.getFeatures2();
        vk::PhysicalDeviceDescriptorIndexingFeatures descIndexingFeatures;;


        descIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

        descIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
        descIndexingFeatures.runtimeDescriptorArray = VK_TRUE;


        std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
        float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueInfos;

        int familyIndex = 0;
        for (auto& family : queueFamilies) {
            if (family.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsQueueFamilyIndex = familyIndex;
            }

            if (graphicsQueueFamilyIndex.has_value()) {
                break;
            }

            familyIndex++;
        }

        if (!graphicsQueueFamilyIndex.has_value()) {
            throw std::runtime_error("Failed to find a queue with graphics support");
        }

        //if the graphics queue supports presentation then pick that queue
        presentQueueFamilyIndex = physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex.value(), surface) ? graphicsQueueFamilyIndex.value() : UINT32_MAX;

        //if it doesnt, find another queue
        familyIndex = 0;
        if (presentQueueFamilyIndex == UINT32_MAX) {
            for (auto& family : queueFamilies) {
                if (physicalDevice.getSurfaceSupportKHR(familyIndex, surface) == vk::True) {
                    presentQueueFamilyIndex = familyIndex;
                    queueInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), graphicsQueueFamilyIndex.value(), 1, &queuePriority, nullptr));
                    queueInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), presentQueueFamilyIndex.value(), 1, &queuePriority, nullptr));
                    break;
                }
                familyIndex++;
            }
        }
        else {
            queueInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), graphicsQueueFamilyIndex.value(), 1, &queuePriority, nullptr));
        }

        if (!presentQueueFamilyIndex.has_value()) {
            throw std::runtime_error("Failed to find a queue with presentation support");
        }


        std::vector<vk::ExtensionProperties> deviceProps = physicalDevice.enumerateDeviceExtensionProperties();

        for (const auto& ext : deviceExtensions) {
            auto propertyIterator = std::find_if(
                deviceProps.begin(), deviceProps.end(), [ext](vk::ExtensionProperties const& ep) { return strcmp(ep.extensionName, ext) == 0; });
            if (propertyIterator == deviceProps.end()) {
                std::cout << "Cannot find required device extension " << ext << " extension" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        vk::DeviceCreateInfo deviceInfo{};
        deviceInfo.queueCreateInfoCount = queueInfos.size();
        deviceInfo.pQueueCreateInfos = queueInfos.data();
        deviceInfo.enabledExtensionCount = deviceExtensions.size();
        deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceInfo.pNext = &descIndexingFeatures;

        device = physicalDevice.createDevice(deviceInfo);
        graphicsQueue = device.getQueue(graphicsQueueFamilyIndex.value(), 0);
        presentQueue = device.getQueue(presentQueueFamilyIndex.value(), 0);
    }

    void Context::initMemoryAllocator(){
        VmaAllocatorCreateInfo createInfo{};
        createInfo.device = static_cast<VkDevice>(device);
        createInfo.physicalDevice = static_cast<VkPhysicalDevice>(physicalDevice);
        createInfo.instance = static_cast<VkInstance>(instance);
        createInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        
        auto result = vmaCreateAllocator(&createInfo, &allocator);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to initialize VULKAN MEMORY ALLOCATOR");
        }
    }

    void Context::initSwapchain(){
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);

        std::vector<vk::PresentModeKHR> presentModes;
        presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

        if (surfaceFormats.empty()) {
            throw std::runtime_error("Surface does not have any formats");
        }

        if (presentModes.empty()) {
            throw std::runtime_error("Surface has no present modes available");
        }

        //choosing the right settings
        if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
            swapchainExtent = surfaceCapabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window.getHandle(), &width, &height);
            vk::Extent2D framebufferExtent{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            width = std::clamp(static_cast<uint32_t>(width), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            height = std::clamp(static_cast<uint32_t>(height), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

            swapchainExtent = vk::Extent2D(width, height);
        }

        for (auto& format : surfaceFormats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                swapchainFormat = format;
                break;
            }
        }

        //default present mode which is supported by all vulkan implementations
        swapchainPresentMode = vk::PresentModeKHR::eFifo;

        for (auto& mode : presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                swapchainPresentMode = mode;
            }
        }

        //default is set to 3. However, if thats not supported, it will be clamped.
        swapchainImageCount = std::clamp(swapchainImageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

        vk::SwapchainCreateInfoKHR swapchainInfo(vk::SwapchainCreateFlagsKHR(),
                                                    surface,
                                                    swapchainImageCount,
                                                    swapchainFormat.format,
                                                    swapchainFormat.colorSpace,
                                                    swapchainExtent,
                                                    1,
                                                    vk::ImageUsageFlagBits::eColorAttachment,
                                                    vk::SharingMode::eExclusive,
                                                    {},
                                                    vk::SurfaceTransformFlagBitsKHR::eIdentity,
                                                    vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                    swapchainPresentMode,
                                                    vk::True,
                                                    VK_NULL_HANDLE,
                                                    nullptr);

        std::array<uint32_t, 2> queueIndices = { graphicsQueueFamilyIndex.value(), presentQueueFamilyIndex.value() };
        if (graphicsQueueFamilyIndex.value() != presentQueueFamilyIndex.value()) {
            swapchainInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
            swapchainInfo.queueFamilyIndexCount = 2;
            swapchainInfo.pQueueFamilyIndices = queueIndices.data();
        }

        swapchain = device.createSwapchainKHR(swapchainInfo);

        swapchainImages = device.getSwapchainImagesKHR(swapchain);
    }

    void Context::initSwapchainImageViews(){
        swapchainImageViews.resize(swapchainImages.size());

        for (int i = 0; i < swapchainImageViews.size(); i++) {
            vk::ImageViewCreateInfo viewInfo(
                    vk::ImageViewCreateFlags(),
                    swapchainImages[i],
                    vk::ImageViewType::e2D,
                    swapchainFormat.format,
                    vk::ComponentMapping(),
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1),
                    nullptr
                );

            swapchainImageViews[i] = device.createImageView(viewInfo);
        }
    }

    void Context::initSwapchainDepth(){
        //most common and available pretty much on any system.
        //perfect for this usage case
        vk::Format depthFormat = vk::Format::eD32Sfloat;

        vk::ImageCreateInfo imageInfo{};
        imageInfo.arrayLayers = 1;
        imageInfo.extent = vk::Extent3D(swapchainExtent, 1);
        imageInfo.format = depthFormat;
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.mipLevels = 1;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkImage depthImg;
        VkImageCreateInfo imgCI = imageInfo;
        auto result = vmaCreateImage(allocator, &imgCI, &allocInfo, &depthImg, &depthImageAlloc, nullptr);

        swapchainDepthImage = vk::Image(depthImg);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create depth image");
        }

        vk::ImageViewCreateInfo depthViewInfo;
        depthViewInfo.image = swapchainDepthImage;
        depthViewInfo.format = depthFormat;
        depthViewInfo.viewType = vk::ImageViewType::e2D;
        depthViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        depthViewInfo.subresourceRange.baseArrayLayer = 0;
        depthViewInfo.subresourceRange.baseMipLevel = 0;
        depthViewInfo.subresourceRange.levelCount = 1;
        depthViewInfo.subresourceRange.baseArrayLayer = 0;
        depthViewInfo.subresourceRange.layerCount = 1;

        swapchainDepthView = device.createImageView(depthViewInfo);
    }

    void Context::initCommandPool() {
        vk::CommandPoolCreateInfo createInfo;
        createInfo.queueFamilyIndex = graphicsQueueFamilyIndex.value();
        
        singleTimeCommandPool = device.createCommandPool(createInfo);
    }
}

