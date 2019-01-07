// i4trender.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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

VkInstance createInstance() {
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
		VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
	};

	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	//SHORTCUT: In production Vulkan applications you should handle the real case that the instance creation fails because of Vulkan incompatible hardware
	VkInstance instance = 0;
	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));

	return instance;
}

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


VkDevice createDevice(VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t *familyIndex) {
	
	*familyIndex = 0; //SHORTCUT: This needs to be computed from queue properties; //TODO: this creates a validation error
	
	float queuePriorities[] = { 1.0f };

	VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	queueInfo.queueFamilyIndex = *familyIndex; 
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	const char *extensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueInfo;
	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);

	VkDevice device = 0;
	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, 0, &device));

	return device;
}
	

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window) {
#if defined(VK_USE_PLATFORM_WIN32_KHR) //This is platform specific code!
	VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
	createInfo.hinstance = GetModuleHandle(0);
	createInfo.hwnd = glfwGetWin32Window(window);

	VkSurfaceKHR surface = 0;
	VK_CHECK(vkCreateWin32SurfaceKHR(instance, &createInfo, 0, &surface));
	return surface;

#else 
#error Unsupported platform
#endif
}

int main()
{
	assert(glfwInit());

	VkInstance instance = createInstance();
	assert(instance);

	VkPhysicalDevice physicalDevices[16];
	uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

	//SHORTCUT: In production Vulkan applications you should handle the real case that the instance creation fails because of Vulkan incompatible hardware
	VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
	assert(physicalDevice);

	uint32_t familyIndex = 0;
	VkDevice device = createDevice(instance, physicalDevice, &familyIndex);
	assert(device);

	GLFWwindow *window = glfwCreateWindow(1024, 768, "i4t_Vulkan", 0, 0);
	assert(window);

	VkSurfaceKHR surface = createSurface(instance, window);
	assert(surface);

	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	//TODO --->Here's where we are!
	/*VkSwapchainCreateFlagsKHR        flags;
	VkSurfaceKHR                     surface;
	uint32_t                         minImageCount;
	VkFormat                         imageFormat;
	VkColorSpaceKHR                  imageColorSpace;
	VkExtent2D                       imageExtent;
	uint32_t                         imageArrayLayers;
	VkImageUsageFlags                imageUsage;
	VkSharingMode                    imageSharingMode;
	uint32_t                         queueFamilyIndexCount;
	const uint32_t*                  pQueueFamilyIndices;
	VkSurfaceTransformFlagBitsKHR    preTransform;
	VkCompositeAlphaFlagBitsKHR      compositeAlpha;
	VkPresentModeKHR                 presentMode;
	VkBool32                         clipped;
	VkSwapchainKHR                   oldSwapchain;*/

	VkSwapchainKHR swapchain = 0;
	VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	
	
	glfwDestroyWindow(window);
	vkDestroyInstance(instance, 0);



}