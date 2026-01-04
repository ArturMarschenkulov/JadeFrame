#include "platform_shared.h"
#if defined(JF_PLATFORM_LINUX)
    #include <sys/mman.h>
#endif
#if defined(JF_PLATFORM_LINUX)
    #include <dlfcn.h>
#endif
namespace JadeFrame {
auto get_program_path() -> std::string {
#if defined(JF_PLATFORM_LINUX)
    char    buf[1024] = {0};
    ssize_t buf_size = readlink("/proc/self/exe", buf, sizeof(buf) /*- 1*/);

    if (buf_size == -1) {
        auto  error_code = errno;
        char* error_str = std::strerror(error_code);
        Logger::err(
            "readlink(/proc/self/exe) failed with error code: {}: {}",
            error_code,
            error_str
        );
    }
    std::string path = buf;
    std::size_t path_end = path.find_last_of('/');
    if (path_end != std::string::npos) { path.resize(path_end + 1); }
    return path;
#elif defined(JF_PLATFORM_WINDOWS)
    return "";
#endif
}

auto load_module(const char* path) -> void* {
#if defined(JF_PLATFORM_LINUX)
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#elif defined(JF_PLATFORM_WINDOWS)
    return LoadLibraryA(path);
#else
    JF_UNIMPLEMENTED("");
#endif
}

auto alloc_page(u64 page_amount) -> void* {
    constexpr u64 page_size = 4096;
#if defined(JF_PLATFORM_LINUX)
    return mmap(
        nullptr,
        page_amount * page_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
#elif defined(JF_PLATFORM_WINDOWS)
    return VirtualAlloc(
        nullptr, page_amount * page_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE
    );
#else
    JF_UNIMPLEMENTED("");
#endif
}
} // namespace JadeFrame
