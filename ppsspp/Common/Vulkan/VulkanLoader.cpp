// Copyright (c) 2016- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#include "Common/Vulkan/VulkanLoader.h"
#include <vector>
#include <string>

#include "base/logging.h"
#include "base/basictypes.h"
#include "base/NativeApp.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

PFN_PvkCreateInstance PvkCreateInstance;
PFN_PvkDestroyInstance PvkDestroyInstance;
PFN_PvkEnumeratePhysicalDevices PvkEnumeratePhysicalDevices;
PFN_PvkGetPhysicalDeviceFeatures PvkGetPhysicalDeviceFeatures;
PFN_PvkGetPhysicalDeviceFormatProperties PvkGetPhysicalDeviceFormatProperties;
PFN_PvkGetPhysicalDeviceImageFormatProperties PvkGetPhysicalDeviceImageFormatProperties;
PFN_PvkGetPhysicalDeviceProperties PvkGetPhysicalDeviceProperties;
PFN_PvkGetPhysicalDeviceQueueFamilyProperties PvkGetPhysicalDeviceQueueFamilyProperties;
PFN_PvkGetPhysicalDeviceMemoryProperties PvkGetPhysicalDeviceMemoryProperties;
PFN_PvkGetInstanceProcAddr PvkGetInstanceProcAddr;
PFN_PvkGetDeviceProcAddr PvkGetDeviceProcAddr;
PFN_PvkCreateDevice PvkCreateDevice;
PFN_PvkDestroyDevice PvkDestroyDevice;
PFN_PvkEnumerateInstanceExtensionProperties PvkEnumerateInstanceExtensionProperties;
PFN_PvkEnumerateDeviceExtensionProperties PvkEnumerateDeviceExtensionProperties;
PFN_PvkEnumerateInstanceLayerProperties PvkEnumerateInstanceLayerProperties;
PFN_PvkEnumerateDeviceLayerProperties PvkEnumerateDeviceLayerProperties;
PFN_PvkGetDeviceQueue PvkGetDeviceQueue;
PFN_PvkQueueSubmit PvkQueueSubmit;
PFN_PvkQueueWaitIdle PvkQueueWaitIdle;
PFN_PvkDeviceWaitIdle PvkDeviceWaitIdle;
PFN_PvkAllocateMemory PvkAllocateMemory;
PFN_PvkFreeMemory PvkFreeMemory;
PFN_PvkMapMemory PvkMapMemory;
PFN_PvkUnmapMemory PvkUnmapMemory;
PFN_PvkFlushMappedMemoryRanges PvkFlushMappedMemoryRanges;
PFN_PvkInvalidateMappedMemoryRanges PvkInvalidateMappedMemoryRanges;
PFN_PvkGetDeviceMemoryCommitment PvkGetDeviceMemoryCommitment;
PFN_PvkBindBufferMemory PvkBindBufferMemory;
PFN_PvkBindImageMemory PvkBindImageMemory;
PFN_PvkGetBufferMemoryRequirements PvkGetBufferMemoryRequirements;
PFN_PvkGetImageMemoryRequirements PvkGetImageMemoryRequirements;
PFN_PvkGetImageSparseMemoryRequirements PvkGetImageSparseMemoryRequirements;
PFN_PvkGetPhysicalDeviceSparseImageFormatProperties PvkGetPhysicalDeviceSparseImageFormatProperties;
PFN_PvkQueueBindSparse PvkQueueBindSparse;
PFN_PvkCreateFence PvkCreateFence;
PFN_PvkDestroyFence PvkDestroyFence;
PFN_PvkGetFenceStatus PvkGetFenceStatus;
PFN_PvkCreateSemaphore PvkCreateSemaphore;
PFN_PvkDestroySemaphore PvkDestroySemaphore;
PFN_PvkCreateEvent PvkCreateEvent;
PFN_PvkDestroyEvent PvkDestroyEvent;
PFN_PvkGetEventStatus PvkGetEventStatus;
PFN_PvkSetEvent PvkSetEvent;
PFN_PvkResetEvent PvkResetEvent;
PFN_PvkCreateQueryPool PvkCreateQueryPool;
PFN_PvkDestroyQueryPool PvkDestroyQueryPool;
PFN_PvkGetQueryPoolResults PvkGetQueryPoolResults;
PFN_PvkCreateBuffer PvkCreateBuffer;
PFN_PvkDestroyBuffer PvkDestroyBuffer;
PFN_PPvkCreateBufferView PPvkCreateBufferView;
PFN_PPvkDestroyBufferView PPvkDestroyBufferView;
PFN_PvkCreateImage PvkCreateImage;
PFN_PvkDestroyImage PvkDestroyImage;
PFN_PvkGetImageSubresourceLayout PvkGetImageSubresourceLayout;
PFN_PPvkCreateImageView PPvkCreateImageView;
PFN_PPvkDestroyImageView PPvkDestroyImageView;
PFN_PvkCreateShaderModule PvkCreateShaderModule;
PFN_PvkDestroyShaderModule PvkDestroyShaderModule;
PFN_PvkCreatePipelineCache PvkCreatePipelineCache;
PFN_PPvkDestroyPipelineCache PPvkDestroyPipelineCache;
PFN_PvkGetPipelineCacheData PvkGetPipelineCacheData;
PFN_PvkMergePipelineCaches PvkMergePipelineCaches;
PFN_PvkCreateGraphicsPipelines PvkCreateGraphicsPipelines;
PFN_PvkCreateComputePipelines PvkCreateComputePipelines;
PFN_PvkDestroyPipeline PvkDestroyPipeline;
PFN_PvkCreatePipelineLayout PvkCreatePipelineLayout;
PFN_PPvkDestroyPipelineLayout PPvkDestroyPipelineLayout;
PFN_PvkCreateSampler PvkCreateSampler;
PFN_PvkDestroySampler PvkDestroySampler;
PFN_PvkCreateDescriptorSetLayout PvkCreateDescriptorSetLayout;
PFN_PvkDestroyDescriptorSetLayout PvkDestroyDescriptorSetLayout;
PFN_PvkCreateDescriptorPool PvkCreateDescriptorPool;
PFN_PvkDestroyDescriptorPool PvkDestroyDescriptorPool;
PFN_PvkResetDescriptorPool PvkResetDescriptorPool;
PFN_PvkAllocateDescriptorSets PvkAllocateDescriptorSets;
PFN_PvkFreeDescriptorSets PvkFreeDescriptorSets;
PFN_PvkUpdateDescriptorSets PvkUpdateDescriptorSets;
PFN_PvkCreateFramebuffer PvkCreateFramebuffer;
PFN_PvkDestroyFramebuffer PvkDestroyFramebuffer;
PFN_PvkCreateRenderPass PvkCreateRenderPass;
PFN_PvkDestroyRenderPass PvkDestroyRenderPass;
PFN_PvkGetRenderAreaGranularity PvkGetRenderAreaGranularity;
PFN_PvkCreateCommandPool PvkCreateCommandPool;
PFN_PvkDestroyCommandPool PvkDestroyCommandPool;
PFN_PvkResetCommandPool PvkResetCommandPool;
PFN_PvkAllocateCommandBuffers PvkAllocateCommandBuffers;
PFN_PvkFreeCommandBuffers PvkFreeCommandBuffers;

// Used frequently together
PFN_PvkCmdBindPipeline PvkCmdBindPipeline;
PFN_PvkCmdSetViewport PvkCmdSetViewport;
PFN_PvkCmdSetScissor PvkCmdSetScissor;
PFN_PvkCmdSetBlendConstants PvkCmdSetBlendConstants;
PFN_PvkCmdSetStencilCompareMask PvkCmdSetStencilCompareMask;
PFN_PvkCmdSetStencilWriteMask PvkCmdSetStencilWriteMask;
PFN_PvkCmdSetStencilReference PvkCmdSetStencilReference;
PFN_PvkCmdBindDescriptorSets PvkCmdBindDescriptorSets;
PFN_PvkCmdBindIndexBuffer PvkCmdBindIndexBuffer;
PFN_PvkCmdBindVertexBuffers PvkCmdBindVertexBuffers;
PFN_PvkCmdDraw PvkCmdDraw;
PFN_PPvkCmdDrawIndexed PPvkCmdDrawIndexed;
PFN_PvkCmdPipelineBarrier PvkCmdPipelineBarrier;
PFN_PvkCmdPushConstants PvkCmdPushConstants;

// Every frame to a few times per frame
PFN_PvkWaitForFences PvkWaitForFences;
PFN_PvkResetFences PvkResetFences;
PFN_PvkBeginCommandBuffer PvkBeginCommandBuffer;
PFN_PvkEndCommandBuffer PvkEndCommandBuffer;
PFN_PvkResetCommandBuffer PvkResetCommandBuffer;
PFN_PvkCmdClearAttachments PvkCmdClearAttachments;
PFN_PvkCmdSetEvent PvkCmdSetEvent;
PFN_PvkCmdResetEvent PvkCmdResetEvent;
PFN_PvkCmdWaitEvents PvkCmdWaitEvents;
PFN_PvkCmdBeginRenderPass PvkCmdBeginRenderPass;
PFN_PvkCmdEndRenderPass PvkCmdEndRenderPass;
PFN_PvkCmdCopyBuffer PvkCmdCopyBuffer;
PFN_PvkCmdCopyImage PvkCmdCopyImage;
PFN_PvkCmdBlitImage PvkCmdBlitImage;
PFN_PPvkCmdCopyBufferToImage PPvkCmdCopyBufferToImage;
PFN_PPvkCmdCopyImageToBuffer PPvkCmdCopyImageToBuffer;

// Rare or not used
PFN_PvkCmdSetDepthBounds PvkCmdSetDepthBounds;
PFN_PvkCmdSetLineWidth PvkCmdSetLineWidth;
PFN_PvkCmdSetDepthBias PvkCmdSetDepthBias;
PFN_PPvkCmdDrawIndirect PPvkCmdDrawIndirect;
PFN_PPPvkCmdDrawIndexedIndirect PPPvkCmdDrawIndexedIndirect;
PFN_PvkCmdDispatch PvkCmdDispatch;
PFN_PPvkCmdDispatchIndirect PPvkCmdDispatchIndirect;
PFN_PvkCmdUpdateBuffer PvkCmdUpdateBuffer;
PFN_PvkCmdFillBuffer PvkCmdFillBuffer;
PFN_PvkCmdClearColorImage PvkCmdClearColorImage;
PFN_PvkCmdClearDepthStencilImage PvkCmdClearDepthStencilImage;
PFN_PvkCmdResolveImage PvkCmdResolveImage;
PFN_PvkCmdBeginQuery PvkCmdBeginQuery;
PFN_PvkCmdEndQuery PvkCmdEndQuery;
PFN_PvkCmdResetQueryPool PvkCmdResetQueryPool;
PFN_PvkCmdWriteTimestamp PvkCmdWriteTimestamp;
PFN_PvkCmdCopyQueryPoolResults PvkCmdCopyQueryPoolResults;
PFN_PvkCmdNextSubpass PvkCmdNextSubpass;
PFN_PvkCmdExecuteCommands PvkCmdExecuteCommands;

#ifdef __ANDROID__
PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
#elif defined(_WIN32)
PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
PFN_PvkCreateXlibSurfaceKHR PvkCreateXlibSurfaceKHR;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
#endif

PFN_PvkDestroySurfaceKHR PvkDestroySurfaceKHR;

// WSI extension.
PFN_PvkGetPhysicalDeviceSurfaceSupportKHR PvkGetPhysicalDeviceSurfaceSupportKHR;
PFN_PvkGetPhysicalDeviceSurfaceCapabilitiesKHR PvkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_PvkGetPhysicalDeviceSurfaceFormatsKHR PvkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_PvkGetPhysicalDeviceSurfacePresentModesKHR PvkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_PvkCreateSwapchainKHR PvkCreateSwapchainKHR;
PFN_PvkDestroySwapchainKHR PvkDestroySwapchainKHR;
PFN_PvkGetSwapchainImagesKHR PvkGetSwapchainImagesKHR;
PFN_PvkAcquireNextImageKHR PvkAcquireNextImageKHR;
PFN_PvkQueuePresentKHR PvkQueuePresentKHR;

// And the DEBUG_REPORT extension. We dynamically load this.
PFN_PvkCreateDebugReportCallbackEXT PvkCreateDebugReportCallbackEXT;
PFN_PvkDestroyDebugReportCallbackEXT PvkDestroyDebugReportCallbackEXT;

PFN_vkCreateDebugUtilsMessengerEXT   vkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT	 vkDestroyDebugUtilsMessengerEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT	 vkCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT		 vkCmdEndDebugUtilsLabelEXT;
PFN_vkCmdInsertDebugUtilsLabelEXT	 vkCmdInsertDebugUtilsLabelEXT;
PFN_vkSetDebugUtilsObjectNameEXT     vkSetDebugUtilsObjectNameEXT;
PFN_vkSetDebugUtilsObjectTagEXT      vkSetDebugUtilsObjectTagEXT;

PFN_vkGetMemoryHostPointerPropertiesEXT vkGetMemoryHostPointerPropertiesEXT;
PFN_PvkGetBufferMemoryRequirements2KHR PvkGetBufferMemoryRequirements2KHR;
PFN_PvkGetImageMemoryRequirements2KHR PvkGetImageMemoryRequirements2KHR;
PFN_PvkGetPhysicalDeviceProperties2KHR PvkGetPhysicalDeviceProperties2KHR;
PFN_PvkGetPhysicalDeviceFeatures2KHR PvkGetPhysicalDeviceFeatures2KHR;

#ifdef _WIN32
static HINSTANCE vulkanLibrary;
#define dlsym(x, y) GetProcAddress(x, y)
#else
static void *vulkanLibrary;
#endif
const char *VulkanResultToString(VkResult res);

bool g_vulkanAvailabilityChecked = false;
bool g_vulkanMayBeAvailable = false;

#define LOAD_INSTANCE_FUNC(instance, x) x = (PFN_ ## x)PvkGetInstanceProcAddr(instance, #x); if (!x) {ILOG("Missing (instance): %s", #x);}
#define LOAD_DEVICE_FUNC(instance, x) x = (PFN_ ## x)PvkGetDeviceProcAddr(instance, #x); if (!x) {ILOG("Missing (device): %s", #x);}
#define LOAD_GLOBAL_FUNC(x) x = (PFN_ ## x)dlsym(vulkanLibrary, #x); if (!x) {ILOG("Missing (global): %s", #x);}

#define LOAD_GLOBAL_FUNC_LOCAL(lib, x) (PFN_ ## x)dlsym(lib, #x);

static const char *device_name_blacklist[] = {
	"NVIDIA:SHIELD Tablet K1",
};

static const char *so_names[] = {
	"libvulkan.so",
#if !defined(__ANDROID__)
	"libvulkan.so.1",
#endif
};

void VulkanSetAvailable(bool available) {
	g_vulkanAvailabilityChecked = true;
	g_vulkanMayBeAvailable = available;
}

bool VulkanMayBeAvailable() {
	if (g_vulkanAvailabilityChecked) {
		return g_vulkanMayBeAvailable;
	}

	std::string name = System_GetProperty(SYSPROP_NAME);
	for (const char *blacklisted_name : device_name_blacklist) {
		if (!strcmp(name.c_str(), blacklisted_name)) {
			ILOG("VulkanMayBeAvailable: Device blacklisted ('%s')", name.c_str());
			g_vulkanAvailabilityChecked = true;
			g_vulkanMayBeAvailable = false;
			return false;
		}
	}
	ILOG("VulkanMayBeAvailable: Device allowed ('%s')", name.c_str());

#ifndef _WIN32
	void *lib = nullptr;
	for (int i = 0; i < ARRAY_SIZE(so_names); i++) {
		lib = dlopen(so_names[i], RTLD_NOW | RTLD_LOCAL);
		if (lib) {
			ILOG("VulkanMayBeAvailable: Library loaded ('%s')", so_names[i]);
			break;
		}
	}
#else
	// LoadLibrary etc
	HINSTANCE lib = LoadLibrary(L"vulkan-1.dll");
#endif
	if (!lib) {
		ILOG("Vulkan loader: Library not available");
		g_vulkanAvailabilityChecked = true;
		g_vulkanMayBeAvailable = false;
		return false;
	}

	// Do a hyper minimal initialization and teardown to figure out if there's any chance
	// that any sort of Vulkan will be usable.
	PFN_PvkEnumerateInstanceExtensionProperties localEnumerateInstanceExtensionProperties = LOAD_GLOBAL_FUNC_LOCAL(lib, PvkEnumerateInstanceExtensionProperties);
	PFN_PvkCreateInstance localCreateInstance = LOAD_GLOBAL_FUNC_LOCAL(lib, PvkCreateInstance);
	PFN_PvkEnumeratePhysicalDevices localEnumerate = LOAD_GLOBAL_FUNC_LOCAL(lib, PvkEnumeratePhysicalDevices);
	PFN_PvkDestroyInstance localDestroyInstance = LOAD_GLOBAL_FUNC_LOCAL(lib, PvkDestroyInstance);
	PFN_PvkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties = LOAD_GLOBAL_FUNC_LOCAL(lib, PvkGetPhysicalDeviceProperties);

	// Need to predeclare all this because of the gotos...
	VkInstanceCreateInfo ci{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	VkApplicationInfo info{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	std::vector<VkPhysicalDevice> devices;
	bool anyGood = false;
	const char *instanceExtensions[10]{};
	VkInstance instance = VK_NULL_HANDLE;
	VkResult res = VK_SUCCESS;
	uint32_t physicalDeviceCount = 0;
	uint32_t instanceExtCount = 0;
	bool surfaceExtensionFound = false;
	bool platformSurfaceExtensionFound = false;
	std::vector<VkExtensionProperties> instanceExts;
	ci.enabledExtensionCount = 0;  // Should have been reset by struct initialization anyway, just paranoia.

#ifdef _WIN32
	const char * const platformSurfaceExtension = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(__ANDROID__)
	const char *platformSurfaceExtension = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#else
	const char *platformSurfaceExtension = 0;
#endif

	if (!localEnumerateInstanceExtensionProperties || !localCreateInstance || !localEnumerate || !localDestroyInstance || !localGetPhysicalDeviceProperties) {
		WLOG("VulkanMayBeAvailable: Function pointer missing, bailing");
		goto bail;
	}

	ILOG("VulkanMayBeAvailable: Enumerating instance extensions");
	res = localEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, nullptr);
	// Maximum paranoia.
	if (res != VK_SUCCESS) {
		ELOG("Enumerating VK extensions failed (%s)", VulkanResultToString(res));
		goto bail;
	}
	if (instanceExtCount == 0) {
		ELOG("No VK instance extensions - won't be able to present.");
		goto bail;
	}
	ILOG("VulkanMayBeAvailable: Instance extension count: %d", instanceExtCount);
	instanceExts.resize(instanceExtCount);
	res = localEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, instanceExts.data());
	if (res != VK_SUCCESS) {
		ELOG("Enumerating VK extensions failed (%s)", VulkanResultToString(res));
		goto bail;
	}
	for (auto iter : instanceExts) {
		ILOG("VulkanMaybeAvailable: Instance extension found: %s (%08x)", iter.extensionName, iter.specVersion);
		if (platformSurfaceExtension && !strcmp(iter.extensionName, platformSurfaceExtension)) {
			ILOG("VulkanMayBeAvailable: Found platform surface extension '%s'", platformSurfaceExtension);
			instanceExtensions[ci.enabledExtensionCount++] = platformSurfaceExtension;
			platformSurfaceExtensionFound = true;
			break;
		} else if (!strcmp(iter.extensionName, VK_KHR_SURFACE_EXTENSION_NAME)) {
			instanceExtensions[ci.enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			surfaceExtensionFound = true;
		}
	}
	if (platformSurfaceExtension) {
		if (!platformSurfaceExtensionFound || !surfaceExtensionFound) {
			ELOG("Platform surface extension not found");
			goto bail;
		}
	} else {
		if (!surfaceExtensionFound) {
			ELOG("Surface extension not found");
			goto bail;
		}
	}

	// This can't happen unless the driver is double-reporting a surface extension.
	if (ci.enabledExtensionCount > 2) {
		ELOG("Unexpected number of enabled instance extensions");
		goto bail;
	}

	ci.ppEnabledExtensionNames = instanceExtensions;
	ci.enabledLayerCount = 0;
	info.apiVersion = VK_API_VERSION_1_0;
	info.applicationVersion = 1;
	info.engineVersion = 1;
	info.pApplicationName = "VulkanChecker";
	info.pEngineName = "VulkanCheckerEngine";
	ci.pApplicationInfo = &info;
	ci.flags = 0;
	ILOG("VulkanMayBeAvailable: Calling PvkCreateInstance");
	res = localCreateInstance(&ci, nullptr, &instance);
	if (res != VK_SUCCESS) {
		instance = nullptr;
		ELOG("VulkanMayBeAvailable: Failed to create vulkan instance (%s)", VulkanResultToString(res));
		goto bail;
	}
	ILOG("VulkanMayBeAvailable: Vulkan test instance created successfully.");
	res = localEnumerate(instance, &physicalDeviceCount, nullptr);
	if (res != VK_SUCCESS) {
		ELOG("VulkanMayBeAvailable: Failed to count physical devices (%s)", VulkanResultToString(res));
		goto bail;
	}
	if (physicalDeviceCount == 0) {
		ELOG("VulkanMayBeAvailable: No physical Vulkan devices (count = 0).");
		goto bail;
	}
	devices.resize(physicalDeviceCount);
	res = localEnumerate(instance, &physicalDeviceCount, devices.data());
	if (res != VK_SUCCESS) {
		ELOG("VulkanMayBeAvailable: Failed to enumerate physical devices (%s)", VulkanResultToString(res));
		goto bail;
	}
	anyGood = false;
	for (auto device : devices) {
		VkPhysicalDeviceProperties props;
		localGetPhysicalDeviceProperties(device, &props);
		switch (props.deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			anyGood = true;
			break;
		default:
			ILOG("VulkanMayBeAvailable: Ineligible device found and ignored: '%s'", props.deviceName);
			break;
		}
		// TODO: Should also check queuefamilyproperties for a GRAPHICS queue family? Oh well.
	}

	if (!anyGood) {
		WLOG("VulkanMayBeAvailable: Found Vulkan API, but no good Vulkan device!");
		g_vulkanMayBeAvailable = false;
	} else {
		ILOG("VulkanMayBeAvailable: Found working Vulkan API!");
		g_vulkanMayBeAvailable = true;
	}

bail:
	g_vulkanAvailabilityChecked = true;
	if (instance) {
		ILOG("VulkanMayBeAvailable: Destroying instance");
		localDestroyInstance(instance, nullptr);
	}
	if (lib) {
#ifndef _WIN32
		dlclose(lib);
#else
		FreeLibrary(lib);
#endif
	} else {
		ELOG("Vulkan with working device not detected.");
	}
	return g_vulkanMayBeAvailable;
}

bool VulkanLoad() {
	if (!vulkanLibrary) {
#ifndef _WIN32
		for (int i = 0; i < ARRAY_SIZE(so_names); i++) {
			vulkanLibrary = dlopen(so_names[i], RTLD_NOW | RTLD_LOCAL);
			if (vulkanLibrary) {
				ILOG("VulkanLoad: Found library '%s'", so_names[i]);
				break;
			}
		}
#else
		// LoadLibrary etc
		vulkanLibrary = LoadLibrary(L"vulkan-1.dll");
#endif
		if (!vulkanLibrary) {
			return false;
		}
	}

	LOAD_GLOBAL_FUNC(PvkCreateInstance);
	LOAD_GLOBAL_FUNC(PvkGetInstanceProcAddr);
	LOAD_GLOBAL_FUNC(PvkGetDeviceProcAddr);

	LOAD_GLOBAL_FUNC(PvkEnumerateInstanceExtensionProperties);
	LOAD_GLOBAL_FUNC(PvkEnumerateInstanceLayerProperties);

	if (PvkCreateInstance && PvkGetInstanceProcAddr && PvkGetDeviceProcAddr && PvkEnumerateInstanceExtensionProperties && PvkEnumerateInstanceLayerProperties) {
		WLOG("VulkanLoad: Base functions loaded.");
		return true;
	} else {
		ELOG("VulkanLoad: Failed to load Vulkan base functions.");
#ifndef _WIN32
		dlclose(vulkanLibrary);
#else
		FreeLibrary(vulkanLibrary);
#endif
		vulkanLibrary = nullptr;
		return false;
	}
}

void VulkanLoadInstanceFunctions(VkInstance instance, const VulkanDeviceExtensions &enabledExtensions) {
	// OK, let's use the above functions to get the rest.
	LOAD_INSTANCE_FUNC(instance, PvkDestroyInstance);
	LOAD_INSTANCE_FUNC(instance, PvkEnumeratePhysicalDevices);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceFeatures);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceFormatProperties);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceImageFormatProperties);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceProperties);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceQueueFamilyProperties);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceMemoryProperties);
	LOAD_INSTANCE_FUNC(instance, PvkCreateDevice);
	LOAD_INSTANCE_FUNC(instance, PvkDestroyDevice);
	LOAD_INSTANCE_FUNC(instance, PvkEnumerateDeviceExtensionProperties);
	LOAD_INSTANCE_FUNC(instance, PvkEnumerateDeviceLayerProperties);
	LOAD_INSTANCE_FUNC(instance, PvkGetDeviceQueue);
	LOAD_INSTANCE_FUNC(instance, PvkDeviceWaitIdle);

	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceSurfaceSupportKHR);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceSurfaceFormatsKHR);
	LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceSurfacePresentModesKHR);

	LOAD_INSTANCE_FUNC(instance, PvkCreateSwapchainKHR);
	LOAD_INSTANCE_FUNC(instance, PvkDestroySwapchainKHR);
	LOAD_INSTANCE_FUNC(instance, PvkGetSwapchainImagesKHR);
	LOAD_INSTANCE_FUNC(instance, PvkAcquireNextImageKHR);
	LOAD_INSTANCE_FUNC(instance, PvkQueuePresentKHR);

#ifdef _WIN32
	LOAD_INSTANCE_FUNC(instance, vkCreateWin32SurfaceKHR);
#elif defined(__ANDROID__)
	LOAD_INSTANCE_FUNC(instance, vkCreateAndroidSurfaceKHR);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	LOAD_INSTANCE_FUNC(instance, PvkCreateXlibSurfaceKHR);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	LOAD_INSTANCE_FUNC(instance, vkCreateWaylandSurfaceKHR);
#endif

	LOAD_INSTANCE_FUNC(instance, PvkDestroySurfaceKHR);

	if (enabledExtensions.KHR_get_physical_device_properties2) {
		LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceProperties2KHR);
		LOAD_INSTANCE_FUNC(instance, PvkGetPhysicalDeviceFeatures2KHR);
	}

	if (enabledExtensions.EXT_debug_report) {
		LOAD_INSTANCE_FUNC(instance, PvkCreateDebugReportCallbackEXT);
		LOAD_INSTANCE_FUNC(instance, PvkDestroyDebugReportCallbackEXT);
	}

	if (enabledExtensions.EXT_debug_utils) {
		LOAD_INSTANCE_FUNC(instance, vkCreateDebugUtilsMessengerEXT);
		LOAD_INSTANCE_FUNC(instance, vkDestroyDebugUtilsMessengerEXT);
		LOAD_INSTANCE_FUNC(instance, vkCmdBeginDebugUtilsLabelEXT);
		LOAD_INSTANCE_FUNC(instance, vkCmdEndDebugUtilsLabelEXT);
		LOAD_INSTANCE_FUNC(instance, vkCmdInsertDebugUtilsLabelEXT);
		LOAD_INSTANCE_FUNC(instance, vkSetDebugUtilsObjectNameEXT);
		LOAD_INSTANCE_FUNC(instance, vkSetDebugUtilsObjectTagEXT);
	}

	WLOG("Vulkan instance functions loaded.");
}

// On some implementations, loading functions (that have Device as their first parameter) via PvkGetDeviceProcAddr may
// increase performance - but then these function pointers will only work on that specific device. Thus, this loader is not very
// good for multi-device - not likely we'll ever try that anyway though.
void VulkanLoadDeviceFunctions(VkDevice device, const VulkanDeviceExtensions &enabledExtensions) {
	WLOG("Vulkan device functions loaded.");

	LOAD_DEVICE_FUNC(device, PvkQueueSubmit);
	LOAD_DEVICE_FUNC(device, PvkQueueWaitIdle);
	LOAD_DEVICE_FUNC(device, PvkAllocateMemory);
	LOAD_DEVICE_FUNC(device, PvkFreeMemory);
	LOAD_DEVICE_FUNC(device, PvkMapMemory);
	LOAD_DEVICE_FUNC(device, PvkUnmapMemory);
	LOAD_DEVICE_FUNC(device, PvkFlushMappedMemoryRanges);
	LOAD_DEVICE_FUNC(device, PvkInvalidateMappedMemoryRanges);
	LOAD_DEVICE_FUNC(device, PvkGetDeviceMemoryCommitment);
	LOAD_DEVICE_FUNC(device, PvkBindBufferMemory);
	LOAD_DEVICE_FUNC(device, PvkBindImageMemory);
	LOAD_DEVICE_FUNC(device, PvkGetBufferMemoryRequirements);
	LOAD_DEVICE_FUNC(device, PvkGetImageMemoryRequirements);
	LOAD_DEVICE_FUNC(device, PvkGetImageSparseMemoryRequirements);
	LOAD_DEVICE_FUNC(device, PvkGetPhysicalDeviceSparseImageFormatProperties);
	LOAD_DEVICE_FUNC(device, PvkQueueBindSparse);
	LOAD_DEVICE_FUNC(device, PvkCreateFence);
	LOAD_DEVICE_FUNC(device, PvkDestroyFence);
	LOAD_DEVICE_FUNC(device, PvkResetFences);
	LOAD_DEVICE_FUNC(device, PvkGetFenceStatus);
	LOAD_DEVICE_FUNC(device, PvkWaitForFences);
	LOAD_DEVICE_FUNC(device, PvkCreateSemaphore);
	LOAD_DEVICE_FUNC(device, PvkDestroySemaphore);
	LOAD_DEVICE_FUNC(device, PvkCreateEvent);
	LOAD_DEVICE_FUNC(device, PvkDestroyEvent);
	LOAD_DEVICE_FUNC(device, PvkGetEventStatus);
	LOAD_DEVICE_FUNC(device, PvkSetEvent);
	LOAD_DEVICE_FUNC(device, PvkResetEvent);
	LOAD_DEVICE_FUNC(device, PvkCreateQueryPool);
	LOAD_DEVICE_FUNC(device, PvkDestroyQueryPool);
	LOAD_DEVICE_FUNC(device, PvkGetQueryPoolResults);
	LOAD_DEVICE_FUNC(device, PvkCreateBuffer);
	LOAD_DEVICE_FUNC(device, PvkDestroyBuffer);
	LOAD_DEVICE_FUNC(device, PPvkCreateBufferView);
	LOAD_DEVICE_FUNC(device, PPvkDestroyBufferView);
	LOAD_DEVICE_FUNC(device, PvkCreateImage);
	LOAD_DEVICE_FUNC(device, PvkDestroyImage);
	LOAD_DEVICE_FUNC(device, PvkGetImageSubresourceLayout);
	LOAD_DEVICE_FUNC(device, PPvkCreateImageView);
	LOAD_DEVICE_FUNC(device, PPvkDestroyImageView);
	LOAD_DEVICE_FUNC(device, PvkCreateShaderModule);
	LOAD_DEVICE_FUNC(device, PvkDestroyShaderModule);
	LOAD_DEVICE_FUNC(device, PvkCreatePipelineCache);
	LOAD_DEVICE_FUNC(device, PPvkDestroyPipelineCache);
	LOAD_DEVICE_FUNC(device, PvkGetPipelineCacheData);
	LOAD_DEVICE_FUNC(device, PvkMergePipelineCaches);
	LOAD_DEVICE_FUNC(device, PvkCreateGraphicsPipelines);
	LOAD_DEVICE_FUNC(device, PvkCreateComputePipelines);
	LOAD_DEVICE_FUNC(device, PvkDestroyPipeline);
	LOAD_DEVICE_FUNC(device, PvkCreatePipelineLayout);
	LOAD_DEVICE_FUNC(device, PPvkDestroyPipelineLayout);
	LOAD_DEVICE_FUNC(device, PvkCreateSampler);
	LOAD_DEVICE_FUNC(device, PvkDestroySampler);
	LOAD_DEVICE_FUNC(device, PvkCreateDescriptorSetLayout);
	LOAD_DEVICE_FUNC(device, PvkDestroyDescriptorSetLayout);
	LOAD_DEVICE_FUNC(device, PvkCreateDescriptorPool);
	LOAD_DEVICE_FUNC(device, PvkDestroyDescriptorPool);
	LOAD_DEVICE_FUNC(device, PvkResetDescriptorPool);
	LOAD_DEVICE_FUNC(device, PvkAllocateDescriptorSets);
	LOAD_DEVICE_FUNC(device, PvkFreeDescriptorSets);
	LOAD_DEVICE_FUNC(device, PvkUpdateDescriptorSets);
	LOAD_DEVICE_FUNC(device, PvkCreateFramebuffer);
	LOAD_DEVICE_FUNC(device, PvkDestroyFramebuffer);
	LOAD_DEVICE_FUNC(device, PvkCreateRenderPass);
	LOAD_DEVICE_FUNC(device, PvkDestroyRenderPass);
	LOAD_DEVICE_FUNC(device, PvkGetRenderAreaGranularity);
	LOAD_DEVICE_FUNC(device, PvkCreateCommandPool);
	LOAD_DEVICE_FUNC(device, PvkDestroyCommandPool);
	LOAD_DEVICE_FUNC(device, PvkResetCommandPool);
	LOAD_DEVICE_FUNC(device, PvkAllocateCommandBuffers);
	LOAD_DEVICE_FUNC(device, PvkFreeCommandBuffers);
	LOAD_DEVICE_FUNC(device, PvkBeginCommandBuffer);
	LOAD_DEVICE_FUNC(device, PvkEndCommandBuffer);
	LOAD_DEVICE_FUNC(device, PvkResetCommandBuffer);
	LOAD_DEVICE_FUNC(device, PvkCmdBindPipeline);
	LOAD_DEVICE_FUNC(device, PvkCmdSetViewport);
	LOAD_DEVICE_FUNC(device, PvkCmdSetScissor);
	LOAD_DEVICE_FUNC(device, PvkCmdSetLineWidth);
	LOAD_DEVICE_FUNC(device, PvkCmdSetDepthBias);
	LOAD_DEVICE_FUNC(device, PvkCmdSetBlendConstants);
	LOAD_DEVICE_FUNC(device, PvkCmdSetDepthBounds);
	LOAD_DEVICE_FUNC(device, PvkCmdSetStencilCompareMask);
	LOAD_DEVICE_FUNC(device, PvkCmdSetStencilWriteMask);
	LOAD_DEVICE_FUNC(device, PvkCmdSetStencilReference);
	LOAD_DEVICE_FUNC(device, PvkCmdBindDescriptorSets);
	LOAD_DEVICE_FUNC(device, PvkCmdBindIndexBuffer);
	LOAD_DEVICE_FUNC(device, PvkCmdBindVertexBuffers);
	LOAD_DEVICE_FUNC(device, PvkCmdDraw);
	LOAD_DEVICE_FUNC(device, PPvkCmdDrawIndexed);
	LOAD_DEVICE_FUNC(device, PPvkCmdDrawIndirect);
	LOAD_DEVICE_FUNC(device, PPPvkCmdDrawIndexedIndirect);
	LOAD_DEVICE_FUNC(device, PvkCmdDispatch);
	LOAD_DEVICE_FUNC(device, PPvkCmdDispatchIndirect);
	LOAD_DEVICE_FUNC(device, PvkCmdCopyBuffer);
	LOAD_DEVICE_FUNC(device, PvkCmdCopyImage);
	LOAD_DEVICE_FUNC(device, PvkCmdBlitImage);
	LOAD_DEVICE_FUNC(device, PPvkCmdCopyBufferToImage);
	LOAD_DEVICE_FUNC(device, PPvkCmdCopyImageToBuffer);
	LOAD_DEVICE_FUNC(device, PvkCmdUpdateBuffer);
	LOAD_DEVICE_FUNC(device, PvkCmdFillBuffer);
	LOAD_DEVICE_FUNC(device, PvkCmdClearColorImage);
	LOAD_DEVICE_FUNC(device, PvkCmdClearDepthStencilImage);
	LOAD_DEVICE_FUNC(device, PvkCmdClearAttachments);
	LOAD_DEVICE_FUNC(device, PvkCmdResolveImage);
	LOAD_DEVICE_FUNC(device, PvkCmdSetEvent);
	LOAD_DEVICE_FUNC(device, PvkCmdResetEvent);
	LOAD_DEVICE_FUNC(device, PvkCmdWaitEvents);
	LOAD_DEVICE_FUNC(device, PvkCmdPipelineBarrier);
	LOAD_DEVICE_FUNC(device, PvkCmdBeginQuery);
	LOAD_DEVICE_FUNC(device, PvkCmdEndQuery);
	LOAD_DEVICE_FUNC(device, PvkCmdResetQueryPool);
	LOAD_DEVICE_FUNC(device, PvkCmdWriteTimestamp);
	LOAD_DEVICE_FUNC(device, PvkCmdCopyQueryPoolResults);
	LOAD_DEVICE_FUNC(device, PvkCmdPushConstants);
	LOAD_DEVICE_FUNC(device, PvkCmdBeginRenderPass);
	LOAD_DEVICE_FUNC(device, PvkCmdNextSubpass);
	LOAD_DEVICE_FUNC(device, PvkCmdEndRenderPass);
	LOAD_DEVICE_FUNC(device, PvkCmdExecuteCommands);

	if (enabledExtensions.EXT_external_memory_host) {
		LOAD_DEVICE_FUNC(device, vkGetMemoryHostPointerPropertiesEXT);
	}
	if (enabledExtensions.KHR_dedicated_allocation) {
		LOAD_DEVICE_FUNC(device, PvkGetBufferMemoryRequirements2KHR);
		LOAD_DEVICE_FUNC(device, PvkGetImageMemoryRequirements2KHR);
	}
}

void VulkanFree() {
	if (vulkanLibrary) {
#ifdef _WIN32
		FreeLibrary(vulkanLibrary);
#else
		dlclose(vulkanLibrary);
#endif
		vulkanLibrary = nullptr;
	}
}
