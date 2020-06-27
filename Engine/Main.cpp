#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#ifdef NDEBUG
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

const std::vector<const char*> validationLayer = {
	"VK_LAYER_KHRONOS_validation"
};

bool CheckValidationLayerProperties()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
	std::vector<VkLayerProperties> validationLayerAvailable( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, validationLayerAvailable.data() );

	bool isAvailable = true;
	for( const auto& i : validationLayer )
	{
		if( std::find( validationLayerAvailable.begin(), validationLayerAvailable.end(), i ) == validationLayerAvailable.end() )
		{
			isAvailable = false;
			break;
		}
	}
	return isAvailable;
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

		window = glfwCreateWindow( ScreenWidth, ScreenHeight, "Learning Vulkan" ,nullptr, nullptr );
	}
	void InitVulkan()
	{
		InitInstance();
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

		uint32_t glfwExtensionsCount = 0U;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionsCount );
		createInfo.enabledExtensionCount = glfwExtensionsCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		if( enableValidationLayer )
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>( validationLayer.size() );
			createInfo.ppEnabledLayerNames = validationLayer.data();
		}
		else
			createInfo.enabledLayerCount = 0U;

		if( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create instance\n" );

		uint32_t vkExtensionsCount = 0U;
		vkEnumerateInstanceExtensionProperties( nullptr, &vkExtensionsCount, nullptr );
		std::vector<VkExtensionProperties> vkExtensions( vkExtensionsCount );
		vkEnumerateInstanceExtensionProperties( nullptr, &vkExtensionsCount, vkExtensions.data() );

		if( !CheckExtensionProperties( glfwExtensions, glfwExtensionsCount, vkExtensions ) )
			throw std::runtime_error( "Failed to found the extensions\n" );

	}
	void MainLoop()
	{
		while( !glfwWindowShouldClose( window ) )
			glfwPollEvents();
	}
	void CleanUp()
	{
		vkDestroyInstance( instance, nullptr );
		glfwDestroyWindow( window );

		glfwTerminate();
	}

	//Checks member functions
	bool CheckExtensionProperties( const char** extensions, uint32_t extensionsCount ,std::vector<VkExtensionProperties>& vkExtensions )
	{
		bool isAvilable = true;
		const auto end = (&*extensions) + extensionsCount;

		for( auto i = &*extensions; i != end; ++i )
		{
			auto tmp = std::find_if( vkExtensions.begin(), vkExtensions.end(),
				[&i]( const VkExtensionProperties& s ) { return s.extensionName == *i; } );
			if( tmp != vkExtensions.end() )
			{
				isAvilable = false;
				break;
			}
		}
		return isAvilable;
	}

public:
	static constexpr int ScreenWidth = 800;
	static constexpr int ScreenHeight = 600;
private:
	GLFWwindow* window;
	VkInstance instance;
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