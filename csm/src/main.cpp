#include "Vulkan/Enumerate.h"
#include "Vulkan/Strings.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/Version.h"
#include "Utilities/Console.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "renderer.h"

namespace
{
	void PrintVulkanSdkInformation();
	void PrintVulkanInstanceInformation(const vk::Application& application);
	void PrintVulkanLayersInformation(const vk::Application& application);
	void PrintVulkanDevices(const vk::Application& application);
	void PrintVulkanSwapChainInformation(const vk::Application& application);
	void SetVulkanDevice(vk::Application& application);
}

int main(int argc, const char* argv[]) noexcept
{
	try
	{
		const vk::WindowConfig windowConfig
		{
			"Vulkan Window",		// title
			1280,					// width
			800,					// height
			false,					// CursorDisabled
			false,					// Fullscreen
			true					// Resizable
		};

#ifndef NDEBUG
		bool enableValidationLayers = true;
#else
		bool enableValidationLayers = false;
#endif 
		
		Renderer application(windowConfig, static_cast<VkPresentModeKHR>(2), enableValidationLayers);

		PrintVulkanSdkInformation();

#ifdef _DEBUG
		PrintVulkanInstanceInformation(application);
		PrintVulkanLayersInformation(application);
#endif 
		PrintVulkanDevices(application);

		SetVulkanDevice(application);

		PrintVulkanSwapChainInformation(application);

		application.Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


namespace {
	void PrintVulkanSdkInformation()
	{
		std::cout << "Vulkan SDK Header Version: " << VK_HEADER_VERSION << "\n";
		std::cout << std::endl;
	}

	void PrintVulkanInstanceInformation(const vk::Application& application)
	{

		std::cout << "Vulkan Instance Extensions: \n";

		for (const auto& extension : application.Extensions())
		{
			std::cout << "- " << extension.extensionName << " (" << vk::Version(extension.specVersion) << ")\n";
		}

		std::cout << std::endl;
	}

	void PrintVulkanLayersInformation(const vk::Application& application)
	{

		std::cout << "Vulkan Instance Layers: " << std::endl;

		for (const auto& layer : application.Layers())
		{
			std::cout
				<< "- " << layer.layerName
				<< " (" << vk::Version(layer.specVersion) << ")"
				<< " : " << layer.description << "\n";
		}

		std::cout << std::endl;
	}

	void PrintVulkanDevices(const vk::Application& application)
	{
		std::cout << "Vulkan Devices: " << "\n";

		for (const auto& device : application.PhysicalDevices())
		{
			VkPhysicalDeviceDriverProperties driverProp{};
			driverProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

			VkPhysicalDeviceProperties2 deviceProp{};
			deviceProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			deviceProp.pNext = &driverProp;

			vkGetPhysicalDeviceProperties2(device, &deviceProp);

			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(device, &features);

			const auto& prop = deviceProp.properties;

			const vk::Version vulkanVersion(prop.apiVersion);
			const vk::Version driverVersion(prop.driverVersion, prop.vendorID);

			std::cout << "- [" << prop.deviceID << "] ";
			std::cout << vk::Strings::VendorId(prop.vendorID) << " '" << prop.deviceName;
			std::cout << "' (";
			std::cout << vk::Strings::DeviceType(prop.deviceType) << ": ";
			std::cout << "vulkan " << vulkanVersion << ", ";
			std::cout << "driver " << driverProp.driverName << " " << driverProp.driverInfo << " - " << driverVersion;
			std::cout << ")" << "\n";
		}

		std::cout << std::endl;
	}

	void PrintVulkanSwapChainInformation(const vk::Application& application)
	{
		const auto& swapChain = application.SwapChain();

		std::cout << "Swap Chain: " << "\n";
		std::cout << "- image count: " << swapChain.Images().size() << "\n";
		std::cout << "- present mode: " << swapChain.PresentMode() << "\n";
		std::cout << std::endl;
	}

	void SetVulkanDevice(vk::Application& application)
	{
		const auto& physicalDevices = application.PhysicalDevices();
		const auto result = std::find_if(physicalDevices.begin(), physicalDevices.end(), [&](const VkPhysicalDevice& device)
			{
				VkPhysicalDeviceProperties2 prop{};
				prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
				vkGetPhysicalDeviceProperties2(device, &prop);

				// We want a device with geometry shader support.
				VkPhysicalDeviceFeatures deviceFeatures;
				vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

				if (!deviceFeatures.geometryShader)
				{
					return false;
				}

				// We want a device that supports the ray tracing extension.
				const auto extensions = vk::GetEnumerateVector(device, static_cast<const char*>(nullptr), vkEnumerateDeviceExtensionProperties);

				// We want a device with a graphics queue.
				const auto queueFamilies = vk::GetEnumerateVector(device, vkGetPhysicalDeviceQueueFamilyProperties);
				const auto hasGraphicsQueue = std::any_of(queueFamilies.begin(), queueFamilies.end(), [](const VkQueueFamilyProperties& queueFamily)
					{
						return queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
					});

				return hasGraphicsQueue;
			});

		if (result == physicalDevices.end())
		{
			throw std::runtime_error("cannot find a suitable device");
		}

		VkPhysicalDeviceProperties2 deviceProp{};
		deviceProp.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		vkGetPhysicalDeviceProperties2(*result, &deviceProp);

		std::cout << "Setting Device [" << deviceProp.properties.deviceID << "]:\n";

		application.SetPhysicalDevice(*result);

		std::cout << std::endl;
	}
}