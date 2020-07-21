#pragma once

#define GLFW_EXPOSE_NATIVE_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <optional>
#include <set>

#include "DebugUtilsMessengerEXT.h"
#include "QueueFamilyIndices.h"

// ___ VALIDATION LAYER ____
#ifdef NDEBUG
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

const std::vector<const char*> validationLayer = {
	"VK_LAYER_KHRONOS_validation"
};
// _________________________

//si author vulkan tutorial menamainya "deviceExtensions"
const std::vector<const char*> deviceExtensionsNeeded = {
	"VK_KHR_swapchain"
};

class HelloTriangleApp
{
public:
	void Run();

private:
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void CleanUp();

	//INSTANCE
	void InitInstance();

	//PHYSICAL DEVICE
	void PickPhysicalDevice();

	//LOGICAL DEVICE
	void CreateLogicalDevice();

	// --- DEBUG MESSENGER ---
	// -----------------------
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSaverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	);
	void PopulateDebugUtilsMessengerCreateInfoEXT( VkDebugUtilsMessengerCreateInfoEXT& createInfo );
	void SetupDebugMessenger();
	// -----------------------

	//SURFACE
	void CreateSurface();

	// --- GETTER ---
	// --------------
	std::vector<const char*> GetRequiredExtension();
	int RateDeviceSuitability( VkPhysicalDevice device );
	std::pair<VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures>
		GetPhysicalDevicePropertiesAndFeatures( VkPhysicalDevice physicalDevice ) const;
	VkPhysicalDeviceProperties GetPhysicalDeviceProperties( VkPhysicalDevice physicalDevice ) const;
	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures( VkPhysicalDevice physicalDevice ) const;
	QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device );
	// -------------

	// --- CHECKER ---
	// ---------------
	bool CheckExtensionProperties( const std::vector<const char*>& extensions, std::vector<VkExtensionProperties>& vkExtensions );
	bool CheckValidationLayerProperties();
	bool IsDeviceSuitable( VkPhysicalDevice physicalDevice );
	bool CheckDeviceExtensionSupport( VkPhysicalDevice physicalDevice );
	// ---------------

public:
	static constexpr int ScreenWidth = 800;
	static constexpr int ScreenHeight = 600;
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
};