#pragma once
#include "vulkan/vulkan.hpp"
#include "vkn_window.hpp"

#include "vma/vk_mem_alloc.h"

#include <optional>

namespace vkn {
	struct ContextInfo {
		std::string app_name;
		uint32_t appVersion;
	};
	class Context {
	public:
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(ContextInfo& info, vkn::Window& _window);
		~Context();

		//getters
		vk::Device& getDevice() { return device; }
		vk::SurfaceFormatKHR getSwapchainFormat() { return swapchainFormat; }
		std::vector<vk::ImageView>& getSwapchainImageViews() { return swapchainImageViews; }
		vk::Extent2D getSwapchainExtent() { return swapchainExtent; }
		uint32_t getGraphicsQueueFamilyIndex() { return graphicsQueueFamilyIndex.value(); }
		uint32_t getPresentQueueFamilyIndex() { return presentQueueFamilyIndex.value(); }
		vk::SwapchainKHR& getSwapchain() { return swapchain; }
		vk::Queue& getGraphicsQueue() { return graphicsQueue; }
		vk::Queue& getPresentQueue() { return presentQueue; }
		VmaAllocator& getAllocator() { return allocator; }
		vk::DispatchLoaderDynamic& getDispatchLoader() { return dLoader; }

	private:
		ContextInfo ctxInfo;

		//handles
		vk::Instance instance;
		std::vector<const char*> instanceLayers;
		std::vector<const char*> instanceExtensions;
#ifdef _DEBUG
		vk::DebugUtilsMessengerEXT debugUtilsMessenger;
#endif
		vkn::Window& window;
		vk::SurfaceKHR surface;

		vk::PhysicalDevice physicalDevice;
		std::optional<uint32_t> graphicsQueueFamilyIndex;
		std::optional<uint32_t> presentQueueFamilyIndex;

		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
		};
		vk::Device device;

		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		VmaAllocator allocator;

		vk::Extent2D swapchainExtent;
		vk::SurfaceFormatKHR swapchainFormat;
		vk::PresentModeKHR swapchainPresentMode;
		uint32_t swapchainImageCount = 3u;
		vk::SwapchainKHR swapchain;

		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapchainImageViews;

		vk::DispatchLoaderDynamic dLoader;

		//functions
		void initInstance();
		void initDispatchLoader();
		void initDevice();
		void initMemoryAllocator();
		void initSwapchain();
		void initSwapchainImageViews();
	};
}