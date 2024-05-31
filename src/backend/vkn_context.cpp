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
        initDevice();
        initSwapchain();
	}

	Context::~Context(){
        device.destroySwapchainKHR(swapchain);
        device.destroy();

        instance.destroySurfaceKHR(surface);
#ifdef _DEBUG
        instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
		instance.destroy();
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

        device = physicalDevice.createDevice(deviceInfo);
        device.getQueue(graphicsQueueFamilyIndex.value(), 0);
        device.getQueue(presentQueueFamilyIndex.value(), 0);
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
    }
}

