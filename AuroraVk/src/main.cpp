
#include <iostream>
#include <assert.h>
#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <vector>
#include <optional>
#include <set>
#define VK_CHECK(call) \
	do {\
		VkResult _r =call; \
		assert(_r==VK_SUCCESS);\
	}while(0)

void keyCallBack(GLFWwindow* win, int key, int scancode, int actions, int mods) {

	if (key == GLFW_KEY_ESCAPE && actions == GLFW_PRESS) {
		glfwSetWindowShouldClose(win, GLFW_TRUE);
	}

}

std::vector<const char*> validationLayers =
{ "VK_LAYER_KHRONOS_validation",

};

std::vector <const char * > deviceExtensions = {
VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool checkValidationLayerSupport() {
	uint32_t layerCount;

	vkEnumerateInstanceLayerProperties(&layerCount, 0);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const char* layerName : validationLayers) {

		bool layerFound = false;

		for (const auto& layerProprties : availableLayers) {

			if (strcmp(layerName, layerProprties.layerName) == 0)
			{
				return true;
			}

		}
	}

	return false;
}


std::vector<const char*> getRequiredExtensions() {

	uint32_t glfwExtensionCount = 0;

	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef _DEBUG 
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL   debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
	void*                                            pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}


void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo) {

	createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}



void initInstance(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger) {

	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	createInfo.pApplicationInfo = &appInfo;
#ifdef _DEBUG

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (checkValidationLayerSupport()) {
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.enabledLayerCount = validationLayers.size();
		fillDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
#endif


	auto extensions = getRequiredExtensions();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = extensions.size();




	VK_CHECK(vkCreateInstance(&createInfo, 0, &instance));


#ifdef _DEBUG
	VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, 0, &debugMessenger));
#endif // _DEBUG


}

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamilyIndex;
	std::optional<uint32_t> presentFamilyIndex;

	bool isReady() {

		return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
	}
};


bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());



	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		//printf("deviceExtenionName: %s\n ", extension.extensionName);
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


struct PhsicalDeviceInfo {
	QueueFamilyIndices queuefamilyindices;
	VkPhysicalDevice phyDevice;
};
QueueFamilyIndices getPhysicalDeviceQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t queueFamilyCount = 0;
	QueueFamilyIndices indices;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	uint32_t i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentIsSUpported = false;

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags &VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamilyIndex = i;

		}
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentIsSUpported);
		if (queueFamily.queueCount > 0 && presentIsSUpported) {
			indices.presentFamilyIndex = i;
		}

		if (indices.isReady())
		{
			return indices;
		}

		i++;
	}
	return indices;
}


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails  details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	assert(formatCount);

	details.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());


	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	assert(presentModeCount);
	details.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());



	return details;
}



VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}


VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {

		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR
		;
}


VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	assert(0);

}

PhsicalDeviceInfo pickPhysicalDevice(VkInstance& instance, VkSurfaceKHR surface) {

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, 0);
	assert(deviceCount);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	int i = 0;
	PhsicalDeviceInfo chosen;
	for (const auto& device : devices) {
		VkPhysicalDeviceProperties deviceprop;
		vkGetPhysicalDeviceProperties(device, &deviceprop);
		printf("GPU[%d]:%s Vram: %dmb\n", i, deviceprop.deviceName, deviceprop.limits.maxMemoryAllocationCount);
		QueueFamilyIndices queueIndices = getPhysicalDeviceQueueFamilies(device, surface);
		bool isDeviceextAvailable = checkDeviceExtensionSupport(device);
		SwapChainSupportDetails swapChainDetails = querySwapChainSupport(device, surface);
		bool swapChainIsAdequate = !swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty();
		if (queueIndices.isReady() && isDeviceextAvailable && swapChainIsAdequate) {

			chosen.phyDevice = device;
			chosen.queuefamilyindices = queueIndices;
		}
		i++;
	}

	VkPhysicalDeviceProperties deviceprop;
	vkGetPhysicalDeviceProperties(chosen.phyDevice, &deviceprop);
	printf("chosen device: %s\n", deviceprop.deviceName);
	return  chosen;
}


void createLogicalDeviceAndQueueFamilies(VkInstance instance, PhsicalDeviceInfo
	& phydeviceInfo, VkDevice& device) {


	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { phydeviceInfo.queuefamilyindices.graphicsFamilyIndex.value(),
		phydeviceInfo.queuefamilyindices.presentFamilyIndex.value()
	};
	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCreateInfo.queueFamilyIndex = phydeviceInfo.queuefamilyindices.graphicsFamilyIndex.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}




	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	VK_CHECK(vkCreateDevice(phydeviceInfo.phyDevice, &deviceInfo, 0, &device));

}


void createSurface(const VkInstance & instance, GLFWwindow* window, VkSurfaceKHR& surface) {
	VK_CHECK(glfwCreateWindowSurface(instance, window, 0, &surface));

}


VkInstance instance = 0;

VkSwapchainKHR swapChain;
VkDevice logicalDevice;
VkSurfaceKHR surface;
VkQueue graphicsQueue;
VkQueue presentQueue;

std::vector<VkImage> swapChainImages;
std::vector<VkImageView> swapChainImageViews;
VkFormat swapChainImageFormat; 
VkExtent2D swapChainExtent;

void createImageViews() {

	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		VK_CHECK(vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]));
	}
}


void createSwapChain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}


	VkSwapchainCreateInfoKHR  createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	QueueFamilyIndices indices = getPhysicalDeviceQueueFamilies(physicalDevice, surface);

	uint32_t queueFamilyIndices[] = { indices.graphicsFamilyIndex.value() ,	indices.presentFamilyIndex.value() };
	if (indices.graphicsFamilyIndex != indices.presentFamilyIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}


	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain));

	VK_CHECK(vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr));
	assert(imageCount);
	swapChainImages.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data()));

	swapChainExtent = extent;
	swapChainImageFormat = surfaceFormat.format;

}
void createGraphicsPipeline() {}
int main() {

	int rc = glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	assert(rc == GLFW_TRUE);
	GLFWwindow* win = glfwCreateWindow(1024, 768, "vulkan!", NULL, NULL);
	assert(win);
	
	VkDebugUtilsMessengerEXT debugMessenger;
	initInstance(instance, debugMessenger);
	
	createSurface(instance, win, surface);
	auto deviceInfo = pickPhysicalDevice(instance, surface);

	createLogicalDeviceAndQueueFamilies(instance, deviceInfo, logicalDevice);

	
	vkGetDeviceQueue(logicalDevice, deviceInfo.queuefamilyindices.graphicsFamilyIndex.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, deviceInfo.queuefamilyindices.presentFamilyIndex.value(), 0, &presentQueue);

	createSwapChain(deviceInfo.phyDevice, surface);
	createImageViews();
	createGraphicsPipeline();


	glfwSetKeyCallback(win, keyCallBack);

	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();
	}



	// clean up
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}
	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
	vkDestroySurfaceKHR(instance, surface, 0);
	vkDestroyDevice(logicalDevice, 0);
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroyInstance(instance, 0);


	glfwDestroyWindow(win);
	glfwTerminate();

	system("pause");
	return 0;
}