
#include <iostream>
#include <assert.h>
#include <vulkan\vulkan.h>
#include <GLFW\glfw3.h>
#include <vector>
#include <optional>
#define VK_CHECK(call) \
	do {\
		VkResult _r =call; \
		assert(_r==VK_SUCCESS);\
	}while(0)

void keyCallBack(GLFWwindow* win, int key , int scancode, int actions, int mods) {

	if (key == GLFW_KEY_ESCAPE && actions == GLFW_PRESS) {
		glfwSetWindowShouldClose(win, GLFW_TRUE);
	}

}

std::vector<const char*> validationLayers =
{ "VK_LAYER_KHRONOS_validation",

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
	
	std::cerr << "validation layer: " << pCallbackData->pMessage  << std::endl;
	return VK_FALSE;
}


void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo) {

	createInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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



void initInstance(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger ) {

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

struct QueueFamilyIndex {
	std::optional<uint32_t> graphicsFamilyIndex;
};
struct PhsicalDeviceInfo {
	QueueFamilyIndex queuefamilyindex; 
	VkPhysicalDevice phyDevice;
};
QueueFamilyIndex isBestDevice(VkPhysicalDevice device) {
	uint32_t queueFamilyCount = 0;
	QueueFamilyIndex indices;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	uint32_t i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags &VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamilyIndex = i;
			return indices;
		}
		i++;
	}
	return indices;
}

PhsicalDeviceInfo pickPhysicalDevice(VkInstance& instance) {

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
		printf("GPU[%d]:%s Vram: %dmb\n",i, deviceprop.deviceName , deviceprop.limits.maxMemoryAllocationCount);
		chosen.queuefamilyindex = isBestDevice(device);
		if (chosen.queuefamilyindex.graphicsFamilyIndex.has_value()) {

			chosen.phyDevice = device;

		}
		i++;
	}

	VkPhysicalDeviceProperties deviceprop;
	vkGetPhysicalDeviceProperties(chosen.phyDevice, &deviceprop);
	printf("chosen device: %s\n", deviceprop.deviceName);
	return  chosen ;
}


void createLogicalDevice(VkInstance instance,PhsicalDeviceInfo
& phydeviceInfo, VkDevice& device) {

	VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
	queueCreateInfo.queueFamilyIndex = phydeviceInfo.queuefamilyindex.graphicsFamilyIndex.value();
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceInfo= {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	deviceInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceInfo.queueCreateInfoCount = 1;
	//deviceInfo.pEnabledFeatures = &deviceFeatures;

	VK_CHECK(vkCreateDevice(phydeviceInfo.phyDevice, &deviceInfo, 0, &device));

}


int main() {

	int rc =glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	assert(rc==GLFW_TRUE );

	VkInstance instance = 0;
	VkDebugUtilsMessengerEXT debugMessenger;
	initInstance(instance , debugMessenger);

    auto deviceInfo=pickPhysicalDevice(instance);
	VkDevice logicalDevice;
	createLogicalDevice(instance, deviceInfo, logicalDevice);
	VkQueue graphicsQueue;
	vkGetDeviceQueue(logicalDevice, deviceInfo.queuefamilyindex.graphicsFamilyIndex.value(), 0, &graphicsQueue);

	
	
	GLFWwindow* win = glfwCreateWindow(1024, 768, "vulkan!", NULL, NULL);
	assert(win);

	glfwSetKeyCallback(win, keyCallBack);

	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();
	}



	// clean up
	vkDestroyDevice(logicalDevice, 0);
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroyInstance(instance, 0);
	

	glfwDestroyWindow(win);
	glfwTerminate();
	system("pause");
	system("pause");
	return 0;
}