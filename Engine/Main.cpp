#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <optional>

#ifdef NDEBUG
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

const std::vector<const char*> validationLayer = {
	"VK_LAYER_KHRONOS_validation"
};

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
	if( func != nullptr )
		return func( instance, pCreateInfo, pAllocator, pDebugMessenger );
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
	if( func != nullptr )
		func( instance, debugMessenger, pAllocator );
	else
		throw std::runtime_error( "Failed destroy the debug messenger!" );
}

struct QueueFamilyIndices
{
public:
	bool IsComplete() const
	{
		return graphicsFamily.has_value();
	}
public:
	std::optional<uint32_t> graphicsFamily;
};

QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device )
{
	QueueFamilyIndices indices;

	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, nullptr );
	std::vector<VkQueueFamilyProperties> queueFamilies( queueFamiliesCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, queueFamilies.data() );

	int i = 0;
	for( const auto& queueFamily : queueFamilies )
	{
		if( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
		{
			indices.graphicsFamily = i;
			break;
		}
		++i;
	}

	return indices;
}

class HelloTriangleApp
{
public:
	void Run()
	{
		InitWindow();
		InitVulkan();
		MainLoop();
		CleanUp();
	}
private:
	void InitWindow()
	{
		glfwInit();
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

		window = glfwCreateWindow( ScreenWidth, ScreenHeight, "Learning Vulkan", nullptr, nullptr );
	}
	void InitVulkan()
	{
		InitInstance();
		SetupDebugMessenger();
		PickPhysicalDevice();
	}
	void InitInstance()
	{
		if( enableValidationLayer && !CheckValidationLayerProperties() )
			throw std::runtime_error( "Validation Layer requested, but not available!" );

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_0;
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		const auto extensions = GetRequiredExtension(); // sekarang extensions ini nya jadi ada glfw dan vk extensions (di debug mode)
		createInfo.enabledExtensionCount = static_cast<uint32_t>( extensions.size() );
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if( enableValidationLayer )
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>( validationLayer.size() );
			createInfo.ppEnabledLayerNames = validationLayer.data();

			PopulateDebugUtilsMessengerCreateInfoEXT( debugCreateInfo );
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0U;
			createInfo.pNext = nullptr;
		}

		if( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create instance\n" );

		uint32_t vkExtensionsCount = 0U;
		vkEnumerateInstanceExtensionProperties( nullptr, &vkExtensionsCount, nullptr );
		std::vector<VkExtensionProperties> vkExtensions( vkExtensionsCount );
		vkEnumerateInstanceExtensionProperties( nullptr, &vkExtensionsCount, vkExtensions.data() );

		if( !CheckExtensionProperties( extensions, vkExtensions ) )
			throw std::runtime_error( "Failed to found the extensions\n" );

	}
	void PickPhysicalDevice()
	{
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices( instance, &physicalDeviceCount, nullptr );

		if( physicalDeviceCount == 0 )
			throw std::runtime_error( "Failed to find GPUs with Vulkan Support!" );

		std::vector<VkPhysicalDevice> devices( physicalDeviceCount );
		vkEnumeratePhysicalDevices( instance, &physicalDeviceCount, devices.data() );

		for( const auto& device : devices )
		{
			if( IsDeviceSuitable( device ) )
			{
				physicalDevice = device;
				break;
			}
		}
		if( physicalDevice == VK_NULL_HANDLE )
			throw std::runtime_error( "Failed to find suitable GPUs!" );

		//pilih yang atas (dikomen) itu, atau yang bawah ini. [pilih salah satu!]

		/*std::multimap<int, VkPhysicalDevice> candidates;
		for( const auto device : devices )
		{
			const int score = RateDeviceSuitability( device );
			candidates.insert( std::make_pair( score, device ) );
		}

		if( candidates.rbegin()->first > 0 )
			physicalDevice = candidates.rbegin()->second;
		else
			throw std::runtime_error( "Failed to find suitable GPUs!" );*/
	}
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSaverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
	void PopulateDebugUtilsMessengerCreateInfoEXT( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}
	void SetupDebugMessenger()
	{
		if( !enableValidationLayer ) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugUtilsMessengerCreateInfoEXT( createInfo );

		if( CreateDebugUtilsMessengerEXT( instance, &createInfo, nullptr, &debugMessenger ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to setup debug messenger!" );

	}
	void MainLoop()
	{
		while( !glfwWindowShouldClose( window ) )
			glfwPollEvents();
	}
	void CleanUp()
	{
		if( enableValidationLayer )
			DestroyDebugUtilsMessengerEXT( instance, debugMessenger, nullptr );

		vkDestroyInstance( instance, nullptr );
		glfwDestroyWindow( window );

		glfwTerminate();
	}

	//getter
	std::vector<const char*> GetRequiredExtension()
	{
		uint32_t glfwExtensionsCount = 0U;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionsCount );

		std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionsCount );
		if( enableValidationLayer )
			extensions.push_back( "VK_EXT_debug_utils" );
		
		return extensions;
	}
	int RateDeviceSuitability( VkPhysicalDevice device )
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties( device, &deviceProperties );
		vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

		int score = 0;

		if( !deviceFeatures.geometryShader )
			return 0;

		if( deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
			score += 1000;

		score += deviceProperties.limits.maxImageDimension2D;

		return 0;
	}

	//Checks member functions
	bool CheckExtensionProperties( const std::vector<const char*>& extensions, std::vector<VkExtensionProperties>& vkExtensions )
	{
		bool isAvilable = true;

		for( const auto& i : extensions )
		{
			auto tmp = std::find_if( vkExtensions.begin(), vkExtensions.end(),
				[&i]( const VkExtensionProperties& s ) { return std::strcmp( s.extensionName, i ) == 0; } );
			if( tmp == vkExtensions.end() )
			{
				isAvilable = false;
				break;
			}
		}
		return isAvilable;
	}
	bool CheckValidationLayerProperties()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
		std::vector<VkLayerProperties> validationLayerAvailable( layerCount );
		vkEnumerateInstanceLayerProperties( &layerCount, validationLayerAvailable.data() );

		bool isAvailable = true;
		for( const auto& i : validationLayer )
		{
			const auto tmp = std::find_if(
				validationLayerAvailable.begin(), validationLayerAvailable.end(),
				[&i]( const VkLayerProperties& s ) { return std::strcmp( s.layerName, i ) == 0; } );
			if( tmp == validationLayerAvailable.end() )
			{
				isAvailable = false;
				break;
			}
		}
		return isAvailable;
	}
	bool IsDeviceSuitable( VkPhysicalDevice device )
	{
		/*VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties( device, &deviceProperties );
		vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;*/

		//kita bakalan gunain apapun graphics card nya
		QueueFamilyIndices indices = FindQueueFamilies( device );
		return indices.IsComplete();
	}

public:
	static constexpr int ScreenWidth = 800;
	static constexpr int ScreenHeight = 600;
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

int main()
{
	HelloTriangleApp app;

	try
	{
		app.Run();
	} catch( const std::exception& e ) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}