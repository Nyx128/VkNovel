#include "vulkan/vulkan.hpp"
#include "vkn_window.hpp"

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
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		vk::Device device;

		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		vk::Extent2D swapchainExtent;
		vk::SurfaceFormatKHR swapchainFormat;
		vk::PresentModeKHR swapchainPresentMode;
		uint32_t swapchainImageCount = 3u;
		vk::SwapchainKHR swapchain;

		//functions
		void initInstance();
		void initDevice();
		void initSwapchain();
	};
}