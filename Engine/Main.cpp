#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <iostream>

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
		// TODO
	}
	void MainLoop()
	{
		while( !glfwWindowShouldClose( window ) )
			glfwPollEvents();
	}
	void CleanUp()
	{
		glfwDestroyWindow( window );

		glfwTerminate();
	}
public:
	static constexpr int ScreenWidth = 800;
	static constexpr int ScreenHeight = 600;
private:
	GLFWwindow* window;
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