#pragma once

#include <map>
#include <deque>
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <optional>
#include <variant>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#if defined(_WIN32)
    #include <Windows.h>
    #define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
