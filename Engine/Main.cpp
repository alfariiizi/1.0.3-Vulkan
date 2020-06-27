#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <iostream>

int main()
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

	GLFWwindow* window = glfwCreateWindow( 800U, 600U, "Testing", nullptr, nullptr );

	uint32_t extensionsCount = 0U;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionsCount, nullptr );
	std::vector<VkExtensionProperties> extensions( extensionsCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionsCount, extensions.data() );
	for( const auto& e : extensions )
		std::cout << e.extensionName << '\n';

	while( !glfwWindowShouldClose( window ) )
		glfwPollEvents();

	glfwDestroyWindow( window );
	glfwTerminate();

	return EXIT_SUCCESS;
}