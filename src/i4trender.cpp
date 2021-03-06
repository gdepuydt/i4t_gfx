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

uint32_t getGraphicsQueueFamily(VkPhysicalDevice physicalDevice)
{
	VkQueueFamilyProperties queues[64];
	uint32_t queueCount = sizeof(queues) / sizeof(queues[0]);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queues);

	for (uint32_t i = 0; i < queueCount; ++i)
		if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			return i;

	// TODO: this can be used in pickPhysicalDevice to pick rasterization-capable device
	assert(!"No queue families support graphics, is this a compute-only device?");
	return ~0u;
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

VkFormat getSwapchainFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	VkSurfaceFormatKHR formats[32];
	uint32_t formatCount = sizeof(formats) / sizeof(formats[0]);
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats);
	assert(formatCount > 0); //TODO: this code needs to handle either the formatCount being 0, or first element reporting VK_FORMAT_UNDEFINED
	assert(result == VK_INCOMPLETE || result == VK_SUCCESS);
	return formats[8].format;
}


VkSwapchainKHR createSwapchain(VkDevice device, VkSurfaceKHR surface, uint32_t width, uint32_t height, uint32_t familyIndex, VkFormat format) {
	
	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = format;//VK_FORMAT_R8G8B8A8_UNORM; //Shortcut: some devices BGRA, not RGB, you need to find this out...
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent.width = width;
	createInfo.imageExtent.height = height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.queueFamilyIndexCount = 1;
	createInfo.pQueueFamilyIndices = &familyIndex;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	VkSwapchainKHR swapchain = 0;
	VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, 0, &swapchain));
	return swapchain;
}


VkSemaphore createSemaphore(VkDevice device) {
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkSemaphore semaphore = 0;
	VK_CHECK(vkCreateSemaphore(device, &createInfo, 0, &semaphore));
	return semaphore;
}

VkCommandPool createCommandPool(VkDevice device, uint32_t familyIndex) {
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	createInfo.queueFamilyIndex = familyIndex;
	VkCommandPool commandPool = 0;
	VK_CHECK(vkCreateCommandPool(device, &createInfo, 0, &commandPool));
	return commandPool;
}
	

VkRenderPass createRenderPass(VkDevice device, VkFormat format) {
	
	VkAttachmentDescription attachments[1] = {};
	attachments[0].format = format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachments = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachments;

	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
	createInfo.pAttachments = attachments;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;

	VkRenderPass renderPass = 0;
	VK_CHECK(vkCreateRenderPass(device, &createInfo, 0, &renderPass));
	assert(renderPass);
	return renderPass;
}

VkFramebuffer createFramebuffer(VkDevice device, VkRenderPass renderPass, VkImageView imageView, uint32_t width, uint32_t height) {
	
	VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &imageView;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	VkFramebuffer framebuffer = 0;
	VK_CHECK(vkCreateFramebuffer(device, &createInfo, 0, &framebuffer));
	return framebuffer;
	
}

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format) {
	
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.layerCount = 1;
	VkImageView imageView = 0;
	VK_CHECK(vkCreateImageView(device, &createInfo, 0, &imageView));
	assert(imageView);
	return imageView;

}

VkShaderModule loadShader(VkDevice device, const char* path) {
	FILE* file = fopen(path, "rb");
	assert(file); // TODO: don't assert on file in production code
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	assert(length >= 0);
	fseek(file, 0, SEEK_SET);

	char *buffer = new char[length];
	assert(buffer);

	size_t rc = fread(buffer, 1, length, file);
	assert(rc == size_t(length));
	fclose(file);

	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = length;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

	VkShaderModule shaderModule = 0;
	VK_CHECK(vkCreateShaderModule(device, &createInfo, 0, &shaderModule));
	assert(shaderModule);

	return shaderModule;
}

VkPipelineLayout createPipelineLayout(VkDevice device)
{
	VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

	VkPipelineLayout layout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &createInfo, 0, &layout));

	return layout;
}

VkPipeline createGraphicsPipeline(VkDevice device, VkPipelineCache pipelineCache, VkRenderPass renderPass, VkShaderModule vs, VkShaderModule fs, VkPipelineLayout layout) {
	
	
	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = vs;
	stages[0].pName = "main";
	stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = fs;
	stages[1].pName = "main";

	createInfo.stageCount = sizeof(stages) / sizeof(stages[0]);
	createInfo.pStages = stages;

	VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	createInfo.pVertexInputState = &vertexInput;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	createInfo.pInputAssemblyState = &inputAssembly;

	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

	VkPipelineRasterizationStateCreateInfo rasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationState.lineWidth = 1.f;
	createInfo.pRasterizationState = &rasterizationState;

	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.pMultisampleState = &multisampleState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	createInfo.pDepthStencilState = &depthStencilState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorAttachmentState;
	createInfo.pColorBlendState = &colorBlendState;

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
	dynamicState.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicState;

	createInfo.layout = layout;
	createInfo.renderPass = renderPass;

	VkPipeline graphicsPipeline = 0;

	vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, 0, &graphicsPipeline);
	assert(graphicsPipeline);
	return graphicsPipeline;
}


int main()
{
	assert(glfwInit());

	VkInstance instance = createInstance();
	assert(instance);

	VkPhysicalDevice physicalDevices[16];
	uint32_t physicalDeviceCount = sizeof(physicalDevices) / sizeof(physicalDevices[0]);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

	VkPhysicalDevice physicalDevice = pickPhysicalDevice(physicalDevices, physicalDeviceCount);
	assert(physicalDevice);

	uint32_t familyIndex = getGraphicsQueueFamily(physicalDevice);
	VkDevice device = createDevice(instance, physicalDevice, &familyIndex);
	assert(device);

	GLFWwindow *window = glfwCreateWindow(1024, 768, "i4t_Vulkan", 0, 0);
	assert(window);

	VkSurfaceKHR surface = createSurface(instance, window);
	assert(surface);

	VkBool32 presentSupported = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &presentSupported));
	//assert(presentSupported); // TODO: do we want to deal with this?


	int windowWidth = 0, windowHeight = 0;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	
	VkFormat swapchainFormat = getSwapchainFormat(physicalDevice, surface);
	
	VkSwapchainKHR swapchain = createSwapchain(device, surface, windowWidth, windowHeight, familyIndex, swapchainFormat);
	assert(swapchain);

	VkSemaphore aquireSemaphore = createSemaphore(device);
	assert(aquireSemaphore);
	
	VkSemaphore releaseSemaphore = createSemaphore(device);
	assert(releaseSemaphore);

	VkQueue queue = 0;
	vkGetDeviceQueue(device, familyIndex, 0, &queue);

	VkShaderModule triangleVS = loadShader(device, "shaders/triangle.vert.spv");
	assert(triangleVS);
	VkShaderModule triangleFS = loadShader(device, "shaders/triangle.frag.spv");
	assert(triangleFS);

	VkRenderPass renderPass = createRenderPass(device, swapchainFormat);
	assert(renderPass);
	
	// TODO: this is critical for performance!
	VkPipelineCache pipelineCache= 0;
	VkPipelineLayout triangleLayout = createPipelineLayout(device);
	assert(triangleLayout);
	
	VkPipeline trianglePipeline = createGraphicsPipeline(device, pipelineCache, renderPass,triangleVS, triangleFS, triangleLayout);
	assert(trianglePipeline);

	VkImage swapchainImages[16];
	uint32_t swapchainImageCount = sizeof(swapchainImages) / sizeof(swapchainImages[0]);
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages));
	VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, 0));

	VkImageView swapchainImageViews[16];
	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		swapchainImageViews[i] = createImageView(device, swapchainImages[i], swapchainFormat);

		assert(swapchainImageViews[i]);
	}

	VkFramebuffer swapchainFramebuffers[16];
	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		swapchainFramebuffers[i] = createFramebuffer(device, renderPass, swapchainImageViews[i], windowWidth, windowHeight);
		assert(swapchainFramebuffers[i]);
	}

	VkCommandPool commandPool = createCommandPool(device, familyIndex);
	assert(commandPool);

	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer = 0;
	VK_CHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		
		uint32_t imageIndex = 0;

		VK_CHECK(vkAcquireNextImageKHR(device, swapchain, ~0ull, aquireSemaphore, VK_NULL_HANDLE, &imageIndex));

		VK_CHECK(vkResetCommandPool(device, commandPool, 0));

		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkClearColorValue color = { 48.f/255.f, 10.f / 255.f, 36.f/255.f, 1 };
		VkClearValue clearColor = { color };

		VkRenderPassBeginInfo passBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		passBeginInfo.renderPass = renderPass;
		passBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
		passBeginInfo.renderArea.extent.width = windowWidth;
		passBeginInfo.renderArea.extent.height = windowHeight;
		passBeginInfo.clearValueCount = 1;
		passBeginInfo.pClearValues = &clearColor;


		vkCmdBeginRenderPass(commandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		VkViewport viewport = { 0, float(windowHeight), float(windowWidth), -float(windowHeight), 0, 1 };
		VkRect2D scissor = { {0, 0}, {uint32_t(windowWidth), uint32_t(windowHeight)} };

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		//draw calls go here

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);


		vkCmdEndRenderPass(commandBuffer);
		
		/*VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.levelCount = 1;
		range.layerCount = 1;
		
		vkCmdClearColorImage(commandBuffer, swapchainImages[imageIndex], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &range);*/ //--> required somehow to clear the window with te color... in the official stream this isn't necessary anymore?? 

		VK_CHECK(vkEndCommandBuffer(commandBuffer));

		VkPipelineStageFlags submitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &aquireSemaphore;
		submitInfo.pWaitDstStageMask = &submitStageMask;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &releaseSemaphore;
			   
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		
		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &releaseSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &imageIndex;
		
		VK_CHECK(vkQueuePresentKHR(queue, &presentInfo));

		VK_CHECK(vkDeviceWaitIdle(device));

	}

	glfwDestroyWindow(window);
	vkDestroyInstance(instance, 0);
}