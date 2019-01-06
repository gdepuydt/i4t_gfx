// i4trender.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>
#if 1
//C header
#include <vulkan/vulkan.h>
#else
//Cpp header
#include <vulkan/vulkan.hpp>
#endif

#define VK_CHECK(call) \
	do { \
		VkResult result_ = call; \
		assert(result_ == VK_SUCCESS); \
	} while(0)



VkPhysicalDevice pickPhysicalDevice(VkPhysicalDevice *physicalDevices, uint32_t physicalDeviceCount) {
	
	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &props);
		if (props.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			printf("Picking discrete GPU: %s\n", props.deviceName);
			return physicalDevices[i];
		}
	}
	if (physicalDevices > 0) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevices[0], &props);

		printf("Picking fallback GPU: %s\n", props.deviceName);
		return physicalDevices[0];
	}

	printf("No physical devices available!");
	return 0;
}

int main()
{
	assert(glfwInit());

	//SHORTCUT: In production Vulkan applications you should check if 1.1 is available via vkEnumerateInstanceVersion 
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG

	const char *debugLayers[] = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	createInfo.ppEnabledLayerNames = debugLayers;
	createInfo.enabledLayerCount = sizeof(debugLayers) / sizeof(debugLayers[0]);
 
#endif

	const char *extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME
	};

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	//SHORTCUT: In production Vulkan applications you should handle the real case that the instance creation fails because of Vulkan incompatible hardware
	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

	VkPhysicalDevice physicalDevices[16];
	uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

	//SHORTCUT: In production Vulkan applications you should handle the real case that the instance creation fails because of Vulkan incompatible hardware
	VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
	assert(physicalDevice);

	GLFWwindow *window = glfwCreateWindow(1024, 768, "i4t_Vulkan", 0, 0);
	assert(window);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	
	
	glfwDestroyWindow(window);
	vkDestroyInstance(instance, 0);



}