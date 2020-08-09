#include "HelloTriangleApp.h"
#include <algorithm>

void HelloTriangleApp::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	CleanUp();
}

void HelloTriangleApp::InitWindow()
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	window = glfwCreateWindow( ScreenWidth, ScreenHeight, "Learning Vulkan", nullptr, nullptr );
}

void HelloTriangleApp::InitVulkan()
{
	InitInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
}

void HelloTriangleApp::MainLoop()
{
	while( !glfwWindowShouldClose( window ) )
		glfwPollEvents();
}

void HelloTriangleApp::CleanUp()
{
	for( auto& imageView : swapchainImageViews )
		vkDestroyImageView( device, imageView, nullptr );

	vkDestroySwapchainKHR( device, swapchain, nullptr );

	vkDestroyDevice( device, nullptr );

	if( enableValidationLayer )
		DebugUtilsMessengerEXT::Destroy( instance, debugMessenger, nullptr );

	vkDestroySurfaceKHR( instance, surface, nullptr );
	vkDestroyInstance( instance, nullptr );
	glfwDestroyWindow( window );

	glfwTerminate();
}

void HelloTriangleApp::InitInstance()
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

void HelloTriangleApp::PickPhysicalDevice()
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

void HelloTriangleApp::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies( physicalDevice );

	std::vector<VkDeviceQueueCreateInfo> queueInfosss;
	std::set<uint32_t> uniqueQueueFamilies{ indices.GetGraphicsFamilyValue(), indices.GetPresentFamilyValue() };

	float queuePriority = 1.0f;

	for( uint32_t queueFamily : uniqueQueueFamilies )
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.flags = 0;
		queueInfo.pNext = nullptr;
		queueInfo.queueFamilyIndex = queueFamily;
		queueInfo.queueCount = 1;
		// representing the relative priority of work submitted to each queues
		// the number are normalize number, in range 0.0f to 1.0f
		// Queues dengan priority yang tinggi akan dialokasikan dengan proses yang lebih banyak resource nya atau di jadwal lebih agresif [whatt]
		queueInfo.pQueuePriorities = &queuePriority;

		queueInfosss.push_back( queueInfo );
	}

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.flags = 0;
	deviceInfo.pQueueCreateInfos = queueInfosss.data();
	deviceInfo.queueCreateInfoCount = static_cast<uint32_t>( queueInfosss.size() );

	VkPhysicalDeviceFeatures physicalDeviceFeatures = GetPhysicalDeviceFeatures( physicalDevice );
	deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>( deviceExtensionsNeeded.size() );
	deviceInfo.ppEnabledExtensionNames = deviceExtensionsNeeded.data();
	if( enableValidationLayer )
	{
		deviceInfo.enabledLayerCount = static_cast<uint32_t>( validationLayer.size() );
		deviceInfo.ppEnabledLayerNames = validationLayer.data();
	}
	else
	{
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
	}

	if( vkCreateDevice( physicalDevice, &deviceInfo, nullptr, &device ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create Logical Device" );

	vkGetDeviceQueue( device, indices.GetGraphicsFamilyValue(), 0, &graphicsQueue );
	vkGetDeviceQueue( device, indices.GetPresentFamilyValue(), 0, &presentQueue );
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::debugCallback( 
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSaverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType, 
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
	void* pUserData )
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void HelloTriangleApp::PopulateDebugUtilsMessengerCreateInfoEXT( VkDebugUtilsMessengerCreateInfoEXT& createInfo )
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

void HelloTriangleApp::SetupDebugMessenger()
{
	if( !enableValidationLayer ) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugUtilsMessengerCreateInfoEXT( createInfo );

	if( DebugUtilsMessengerEXT::Create( instance, &createInfo, nullptr, &debugMessenger ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to setup debug messenger!" );
}

void HelloTriangleApp::CreateSurface()
{
	/*VkWin32SurfaceCreateInfoKHR surfaceInfo{};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = nullptr;
	surfaceInfo.hwnd = glfwGetWin32Window( window );
	surfaceInfo.hinstance = GetModuleHandle( nullptr );

	if( vkCreateWin32SurfaceKHR( instance, &surfaceInfo, nullptr, &surface ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create Surface" );*/

	if( glfwCreateWindowSurface( instance, window, nullptr, &surface ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create Surface" );
}

void HelloTriangleApp::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport( physicalDevice );
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( swapChainSupport.format );
	VkPresentModeKHR presentMode = ChooseSwapPresentMode( swapChainSupport.presentationModes );
	VkExtent2D extent = ChooseSwapExtent( swapChainSupport.capabilities );

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if( swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount )
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// Creation info
	// -------------
	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = surfaceFormat.format;
	swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainInfo.imageExtent = extent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies( physicalDevice );
	uint32_t queueFamilyIndices [] = { indices.GetGraphicsFamilyValue(), indices.GetPresentFamilyValue() };
	if( indices.graphicsFamily != indices.presentFamily )
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapchainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	// -------------

	// Creating Swapchain
	// ------------------
	if( vkCreateSwapchainKHR( device, &swapchainInfo, nullptr, &swapchain ) != VK_SUCCESS )
		throw std::runtime_error( "Failed to create swapchain !" );
	// ------------------

	// Retrieving Swapchain Images
	// ---------------------------
	vkGetSwapchainImagesKHR( device, swapchain, &imageCount, nullptr );
	swapchainImages.resize( imageCount );
	vkGetSwapchainImagesKHR( device, swapchain, &imageCount, swapchainImages.data() );
	// ---------------------------

	// Inisialisasi beberapa member variable yang mana akan berguna pada chapter berikutnya
	// ------------------------------------------------------------------------------------
	swapchainFormat = surfaceFormat.format;
	swapchainExtent = extent;
	// ------------------------------------------------------------------------------------
}

void HelloTriangleApp::CreateImageViews()
{
	swapchainImageViews.resize( swapchainImages.size() );

	for( size_t i = 0; i < swapchainImages.size(); ++i )
	{
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = swapchainImages[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = swapchainFormat;

		// component
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// subresource range
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		if( vkCreateImageView( device, &imageViewInfo, nullptr, &swapchainImageViews[i] ) != VK_SUCCESS )
			throw std::runtime_error( "Failed to create imageview" );
	}
}

std::vector<const char*> HelloTriangleApp::GetRequiredExtension()
{
	uint32_t glfwExtensionsCount = 0U;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionsCount );

	std::vector<const char*> extensions( glfwExtensions, glfwExtensions + glfwExtensionsCount );
	if( enableValidationLayer )
		extensions.push_back( "VK_EXT_debug_utils" );

	return extensions;
}

int HelloTriangleApp::RateDeviceSuitability( VkPhysicalDevice device )
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

std::pair<VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures> HelloTriangleApp::GetPhysicalDevicePropertiesAndFeatures( VkPhysicalDevice physicalDevice ) const
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );
	vkGetPhysicalDeviceFeatures( physicalDevice, &deviceFeatures );

	return { deviceProperties, deviceFeatures };
}

VkPhysicalDeviceProperties HelloTriangleApp::GetPhysicalDeviceProperties( VkPhysicalDevice physicalDevice ) const
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties( physicalDevice, &properties );
	return properties;
}

VkPhysicalDeviceFeatures HelloTriangleApp::GetPhysicalDeviceFeatures( VkPhysicalDevice physicalDevice ) const
{
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures( physicalDevice, &features );
	return features;
}

QueueFamilyIndices HelloTriangleApp::FindQueueFamilies( VkPhysicalDevice device )
{
	QueueFamilyIndices indices;

	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, nullptr );
	std::vector<VkQueueFamilyProperties> queueFamilies( queueFamiliesCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamiliesCount, queueFamilies.data() );

	VkBool32 isPresentSupport = false;
	int i = 0;
	for( const auto& queueFamily : queueFamilies )
	{
		if( ( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT ) && !indices.graphicsFamily.has_value() )
			indices.graphicsFamily = i;

		vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &isPresentSupport );

		if( isPresentSupport && !indices.presentFamily.has_value() )
			indices.presentFamily = i;

		if( indices.IsComplete() )
			break;

		++i;
	}

	return indices;
}

SwapChainSupportDetails HelloTriangleApp::QuerySwapChainSupport( VkPhysicalDevice physicalDevice )
{
	SwapChainSupportDetails details;
	
	// Get Basic Surface Capabilites details
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &details.capabilities );

	// Get Surface Format details
	// ----------------------
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &surfaceFormatCount, nullptr );
	if( surfaceFormatCount != 0 )
	{
		details.format.resize( static_cast<size_t>(surfaceFormatCount) );
		vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &surfaceFormatCount, details.format.data() );
	}
	// ----------------------

	// Get Presentation Modes details
	// --------------------------
	uint32_t presentationModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentationModesCount, nullptr );
	if( presentationModesCount != 0 )
	{
		details.presentationModes.resize( static_cast<size_t>(presentationModesCount) );
		vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentationModesCount, details.presentationModes.data() );
	}
	// --------------------------

	return details;
}

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats )
{
	for( const auto& a : availableSurfaceFormats )
	{
		if( a.format == VK_FORMAT_B8G8R8_SRGB && a.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
			return a;
	}

	// jika VK_FORMAT_B8G8R8_SRGB dan VK_COLOR_SPACE_SRGB_NONLINEAR_KHR tidak ditemukan,
	// maka kita me-return element array pertama dari Surface Format yang tersedia saja [pick seadanya :v]
	return availableSurfaceFormats[0];
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapPresentMode( const std::vector<VkPresentModeKHR>& availablePresentModes )
{
	for( const auto& a : availablePresentModes )
	{
		if( a == VK_PRESENT_MODE_MAILBOX_KHR ) // jika terdapat presentation mode yang capable dalam triple buffering
			return a;
	}

	// kalau nggak ada yang capable dalam triple buffering, yaa kita return yang double buffering aja.
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::ChooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities )
{
	if( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() )
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = { ScreenWidth, ScreenHeight };
		actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
		actualExtent.height = std::clamp( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );
		return actualExtent;
	}
}

bool HelloTriangleApp::CheckExtensionProperties( 
	const std::vector<const char*>& extensions, std::vector<VkExtensionProperties>& vkExtensions )
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

bool HelloTriangleApp::CheckValidationLayerProperties()
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

bool HelloTriangleApp::IsDeviceSuitable( VkPhysicalDevice physicalDevice )
{
	/*VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties( device, &deviceProperties );
		vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;*/

			//kita bakalan gunain apapun graphics card nya
	QueueFamilyIndices indices = FindQueueFamilies( physicalDevice );
	bool extensionSupported = CheckDeviceExtensionSupport( physicalDevice );

	// verifying swap chain support
	// ----------------------------
	bool swapChainAdequate = false;
	if( extensionSupported )
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport( physicalDevice );
		swapChainAdequate = !swapChainSupport.format.empty() && !swapChainSupport.presentationModes.empty();
	}
	// ----------------------------

	return indices.IsComplete() && extensionSupported && swapChainAdequate;
}

bool HelloTriangleApp::CheckDeviceExtensionSupport( VkPhysicalDevice physicalDevice )
{
	uint32_t deviceExtensionsCount = 0;
	vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &deviceExtensionsCount, nullptr );
	std::vector<VkExtensionProperties> deviceExtensions( deviceExtensionsCount );
	vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &deviceExtensionsCount, deviceExtensions.data() );

	std::set<std::string> requiredExtension( deviceExtensionsNeeded.begin(), deviceExtensionsNeeded.end() );
	for( const auto& e : deviceExtensions )
		requiredExtension.erase( e.extensionName );

	return requiredExtension.empty(); //jika kosong, maka akan me-return true

	// ----> kode tepat diatas yang aku komen itu merupakan solusi dari si author vulkan tutorial.

	/*bool isFound = true;
	for( const auto& e : deviceExtensionsNeeded )
	{
		if(
			std::find_if( deviceExtensions.begin(), deviceExtensions.end(),
				[e]( const VkExtensionProperties& v )
				{
					return std::strcmp( v.extensionName, e ) == 0;
				}
			) == deviceExtensions.end() )
		{
			isFound = false;
			break;
		}
	}
	return isFound;*/
}
