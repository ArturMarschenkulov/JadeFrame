#include "pch.h"
#include "vulkan_debug.h"
#include "JadeFrame/utils/logger.h"

namespace JadeFrame {
namespace vulkan {
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    Logger::LEVEL level = Logger::LEVEL::TRACE;
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: level = Logger::LEVEL::TRACE; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: level = Logger::LEVEL::INFO; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: level = Logger::LEVEL::WARN; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: level = Logger::LEVEL::ERR; break;
        default: assert(false);
    }
    std::string msg_type = "";
    switch (message_type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: msg_type = "Gen."; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: msg_type = "Val."; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: msg_type = "Per."; break;
        default: assert(false);
    }
    Logger::log(level, "Vulkan {}:\n{}\n-------------", msg_type, pCallbackData->pMessage);

    return VK_FALSE;
}
auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo) -> void {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = //
                                 // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | // Trace
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |    // Info
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | // Warn
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;    // Error
    createInfo.messageType =                              //
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |     //
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |  //
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;  //
    createInfo.pfnUserCallback = debug_callback;
}
} // namespace vulkan
} // namespace JadeFrame

auto vkCreateDebugUtilsMessengerEXT_(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) -> VkResult {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

auto vkDestroyDebugUtilsMessengerEXT_(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) -> void {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) { func(instance, debugMessenger, pAllocator); }
}
