#include "platform_shared.h"
#include "JadeFrame/utils/logger.h"
#if defined(JF_PLATFORM_LINUX)
    #include <sys/mman.h>
    #include <unistd.h>
#endif
#if defined(JF_PLATFORM_LINUX)
    #include <dlfcn.h>
#endif
#if defined(JF_PLATFORM_WINDOWS)
    #include <Windows.h>
#endif
namespace JadeFrame {
auto get_program_path() -> std::string {
#if defined(JF_PLATFORM_LINUX)
    std::vector<char> buffer(1024);
    std::string       executable_path;

    for (;;) {
        const ssize_t path_size = readlink(
            "/proc/self/exe", buffer.data(), buffer.size() - 1
        );
        if (path_size == -1) {
            const auto  error_code = errno;
            const char* error_str = std::strerror(error_code);
            Logger::err(
                "readlink(/proc/self/exe) failed with error code: {}: {}",
                error_code,
                error_str
            );
            return {};
        }

        // readlink() does not append a terminator. Reserve one byte and add it
        // explicitly before treating the buffer as a C string.
        buffer[static_cast<std::size_t>(path_size)] = '\0';
        executable_path.assign(buffer.data(), static_cast<std::size_t>(path_size));

        if (static_cast<std::size_t>(path_size) < buffer.size() - 1) { break; }
        buffer.resize(buffer.size() * 2);
    }

    std::string path = std::move(executable_path);
    std::size_t path_end = path.find_last_of('/');
    if (path_end != std::string::npos) { path.resize(path_end + 1); }
    return path;
#elif defined(JF_PLATFORM_WINDOWS)
    return "";
#endif
}

auto load_module(const char* path) -> void* {
    if (path == nullptr || path[0] == '\0') { return nullptr; }

#if defined(JF_PLATFORM_LINUX)
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#elif defined(JF_PLATFORM_WINDOWS)
    return LoadLibraryA(path);
#else
    (void)path;
    return nullptr;
#endif
}

auto unload_module(void* module) -> bool {
    if (module == nullptr) { return true; }

#if defined(JF_PLATFORM_LINUX)
    return dlclose(module) == 0;
#elif defined(JF_PLATFORM_WINDOWS)
    return FreeLibrary(static_cast<HMODULE>(module)) != FALSE;
#else
    (void)module;
    return false;
#endif
}

auto alloc_page(u64 page_amount) -> void* {
    constexpr u64 page_size = 4096;
    if (page_amount == 0) { return nullptr; }
#if defined(JF_PLATFORM_LINUX)
    void* address = mmap(
        nullptr,
        page_amount * page_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    return address == MAP_FAILED ? nullptr : address;
#elif defined(JF_PLATFORM_WINDOWS)
    return VirtualAlloc(
        nullptr, page_amount * page_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
#else
    (void)page_amount;
    return nullptr;
#endif
}
} // namespace JadeFrame
