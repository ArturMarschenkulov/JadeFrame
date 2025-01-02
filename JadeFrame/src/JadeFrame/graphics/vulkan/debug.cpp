#include "debug.h"

#include <span>
#include <array>

#include "JadeFrame/utils/logger.h"
#include "JadeFrame/utils/utils.h"


namespace JadeFrame {
namespace vulkan {

// make the variable below atomic
static u32           g_error_count = 0;
static constexpr u32 g_max_error_count = 10;

// static u32 g_max_error_count = 10;

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {
    Logger::LEVEL level = Logger::LEVEL::TRACE;
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            level = Logger::LEVEL::TRACE;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            level = Logger::LEVEL::INFO;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            level = Logger::LEVEL::WARN;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            level = Logger::LEVEL::ERR;
            g_error_count += 1;
            break;
        default: assert(false);
    }
    std::string msg_type = "";
    switch (message_type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: msg_type = "Gen."; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: msg_type = "Val."; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: msg_type = "Per."; break;
        default: assert(false);
    }
    Logger::log(
        level, "Vulkan {}:\n{}\n-------------", msg_type, pCallbackData->pMessage
    );

    if (g_error_count >= g_max_error_count) {
        Logger::err("Too many vulkan errors, aborting...");
        assert(false);
        return VK_TRUE;
    }

    return VK_FALSE;
}

auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& info
) -> void {
    info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    auto severity_flags = std::array<u32, 4>{
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    };

    auto type_flags = std::array<u32, 3>{
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    };
    info.messageSeverity = bit::set_flags(severity_flags);
    info.messageType = bit::set_flags(type_flags);
    info.pfnUserCallback = debug_callback;
}
} // namespace vulkan
} // namespace JadeFrame

auto vkCreateDebugUtilsMessengerEXT_(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pDebugMessenger
) -> VkResult {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT
    )vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

auto vkDestroyDebugUtilsMessengerEXT_(
    VkInstance                   instance,
    VkDebugUtilsMessengerEXT     debugMessenger,
    const VkAllocationCallbacks* pAllocator
) -> void {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
    )vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) { func(instance, debugMessenger, pAllocator); }
}
