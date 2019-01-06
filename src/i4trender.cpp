// i4trender.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <assert.h>

#include <GLFW/glfw3.h>
#if 1
#include <vulkan/vulkan.h>
#else
#include <vulkan/vulkan.hpp>
#endif

int main()
{
	printf("Hello Vulkan gfx universe!\n");

	assert(glfwInit());

	GLFWwindow *window = glfwCreateWindow(1024, 768, "i4t_Vulkan", 0, 0);
	assert(window);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	glfwDestroyWindow(window);


}