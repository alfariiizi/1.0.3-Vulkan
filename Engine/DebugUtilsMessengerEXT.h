#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

struct DebugUtilsMessengerEXT
{
	static VkResult Create(
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

	static void Destroy(
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
};