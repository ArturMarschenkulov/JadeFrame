#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

namespace JadeFrame {

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/, VkDebugUtilsMessageTypeFlagsEXT /*messageType*/, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* /*pUserData*/);
auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo) -> void;
}
auto vkCreateDebugUtilsMessengerEXT_(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)->VkResult;
auto vkDestroyDebugUtilsMessengerEXT_(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) -> void;
