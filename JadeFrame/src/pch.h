
// STD
#include <vector>
#include <array>
#include <deque>
#include <tuple>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <unordered_set>
#include <stack>
#include <queue>

#include <variant>
#include <utility>

#include <iostream>
#include <string>
#include <cstring>
//#include <format>

#include <cassert>

#include <chrono>

#include <thread>
#include <future>

#include <type_traits>
#include <concepts>

// Windows
#if defined _WIN32
#include <Windows.h>
#endif

#include "JadeFrame/prelude.h"



// Graphics API specific
// JF_PRAGMA_PUSH
// // #pragma warning(disable:4006)
// #include <shaderc/shaderc.hpp>
// #include <shaderc/env.h>
// #include <shaderc/shaderc.h>
// #include <shaderc/status.h>
// #include <shaderc/visibility.h>
// JF_PRAGMA_POP

#if defined _WIN32
#define VK_USE_PLATFORM_WIN32_KHR

#endif

// #include <vulkan/vulkan.h>
// JadeFrame specific

//#include "JadeFrame/utils/logger.h"
