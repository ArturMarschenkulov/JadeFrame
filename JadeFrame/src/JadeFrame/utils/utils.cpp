#include "pch.h"
#include "utils.h"

#include <cstdlib>
#include <ctime>

#include "option.h"

namespace JadeFrame {
auto custom_simple_hash_0(const std::string& str) -> u32 {
    u32 hash = 0;
    for (auto& it : str) { hash = 37 * hash + 17 * static_cast<char>(it); }
    return hash;
}
static bool is_srand = false;

auto get_random_number(i32 begin, i32 end) -> i32 {
    if (is_srand == false) {
        srand(static_cast<u32>(time(0)));
        is_srand = true;
    }
    return (rand() % end) + begin;
}

auto map_range(const f64 x, const f64 in_min, const f64 in_max, const f64 out_min, const f64 out_max) -> f64 {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// Here I experiment with ideas


namespace T1 {

template<class T>
    requires std::same_as<T, bool>
auto init_memory(T& data) -> void {
    static_assert(!std::is_pointer<T>::value, "'init_memory' does not allow pointertypes");

    static_assert(
        /*std::is_pod<T>::value*/ std::is_standard_layout<T>() && std::is_trivial<T>(),
        "'init_memory'does only allow plain-old-data (POD)");
    ::memset(&data, 0, sizeof(T));
}

class RenderCommandQueue {
public:
    typedef void (*RenderCommandFn)(void*);
    RenderCommandQueue() {
        const auto buffer_size = 10 * 1024 * 1024;
        m_command_buffer = new u8[buffer_size];
        m_command_buffer_ptr = m_command_buffer;
        std::memset(m_command_buffer, 0, buffer_size);
    }
    ~RenderCommandQueue() { delete[] m_command_buffer; }
    auto allocate(RenderCommandFn func, u32 size) {
        // TODO: alignment
        *(RenderCommandFn*)m_command_buffer_ptr = func;
        m_command_buffer_ptr += sizeof(RenderCommandFn);

        *(u32*)m_command_buffer_ptr = size;
        m_command_buffer_ptr += sizeof(u32);

        void* memory = m_command_buffer_ptr;
        m_command_buffer_ptr += size;

        m_command_count++;
        return memory;
    }
    auto execute() -> void {

        u8* buffer = m_command_buffer;

        for (uint32_t i = 0; i < m_command_count; i++) {
            RenderCommandFn function = *(RenderCommandFn*)buffer;
            buffer += sizeof(RenderCommandFn);

            u32 size = *(u32*)buffer;
            buffer += sizeof(u32);
            function(buffer);
            buffer += size;
        }

        m_command_buffer_ptr = m_command_buffer;
        m_command_count = 0;
    }

private:
    u8* m_command_buffer;
    u8* m_command_buffer_ptr;
    u32 m_command_count = 0;
};

template<typename FuncT>
static auto submit(FuncT&& func) -> void {
    RenderCommandQueue rcq;
    auto               render_cmd = [](void* ptr) {
        FuncT* p_func = (FuncT*)ptr;
        (*p_func)();

        p_func->~FuncT();
    };
    auto storage_buffer = rcq.allocate(render_cmd, sizeof(func));
    new (storage_buffer) FuncT(std::forward<FuncT>(func));

    //__debugbreak();
    rcq.execute();
    //__debugbreak();
}

// template<typename BaseType, typename SubType>
// static auto take_ownership(std::set<ptr::Scope<BaseType>>& object_set, ptr::Scope<SubType>&& object) -> SubType*
// {
//     SubType* ref = object.get();
//     object_set.emplace(std::forward<ptr::Scope<SubType>>(object));
//     return ref;
// }

template<typename BaseType, typename SubType>
static auto take_ownership(std::set<std::unique_ptr<BaseType>>& object_set, std::unique_ptr<SubType>&& object)
    -> SubType* {
    SubType* ref = object.get();
    object_set.emplace(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}
template<typename BaseType, typename SubType>
static auto take_ownership(std::vector<std::unique_ptr<BaseType>>& object_set, std::unique_ptr<SubType>&& object)
    -> SubType* {
    SubType* ref = object.get();
    object_set.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}
template<typename BaseType, typename SubType>
static auto take_ownership(std::list<std::unique_ptr<BaseType>>& object_set, std::unique_ptr<SubType>&& object)
    -> SubType* {
    SubType* ref = object.get();
    object_set.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}

template<typename Left, typename Right>
class Either {
public:
    Either(const Left& left) {
        m_left = left;
        m_is_left = true;
    }
    Either(const Right& right) {
        m_right = right;
        m_is_left = false;
    }

    // auto
    // match()
    // ->
    // T {
    //
    // }
private:
    // Left
    // m_left;
    // Right
    // m_right;
    union {
        Left  m_left;
        Right m_right;
    };
    bool m_is_left;
};

// struct Error {
//     std::error_code type;
//     VkResult        vk_result = VK_SUCCESS; // optional error value if a vulkan call failed
// };
// template<typename T>
// class Result {
// public:
//     Result(const T& value)
//         : m_value{value}
//         , m_init{true} {}

//     Result(T&& value)
//         : m_value{std::move(value)}
//         , m_init{true} {}

//     Result(Error error)
//         : m_error{error}
//         , m_init{false} {}

//     Result(std::error_code error_code, VkResult result = VK_SUCCESS)
//         : m_error{error_code, result}
//         , m_init{false} {}

//     ~Result() { destroy(); }
//     Result(Result const& expected)
//         : m_init(expected.m_init) {
//         if (m_init) {
//             new (&m_value) T{expected.m_value};
//         } else {
//             m_error = expected.m_error;
//         }
//     }
//     Result(Result&& expected)
//         : m_init(expected.m_init) {
//         if (m_init) {
//             new (&m_value) T{std::move(expected.m_value)};
//         } else {
//             m_error = std::move(expected.m_error);
//         }
//         expected.destroy();
//     }

//     Result& operator=(const T& expect) {
//         destroy();
//         m_init = true;
//         new (&m_value) T{expect};
//         return *this;
//     }
//     Result& operator=(T&& expect) {
//         destroy();
//         m_init = true;
//         new (&m_value) T{std::move(expect)};
//         return *this;
//     }
//     Result& operator=(const Error& error) {
//         destroy();
//         m_init = false;
//         m_error = error;
//         return *this;
//     }
//     Result& operator=(Error&& error) {
//         destroy();
//         m_init = false;
//         m_error = error;
//         return *this;
//     }

//     const T* operator->() const {
//         assert(m_init);
//         return &m_value;
//     }
//     T* operator->() {
//         assert(m_init);
//         return &m_value;
//     }
//     const T& operator*() const& {
//         assert(m_init);
//         return m_value;
//     }
//     T& operator*() & {
//         assert(m_init);
//         return m_value;
//     }
//     T&& operator*() && {
//         assert(m_init);
//         return std::move(m_value);
//     }
//     const T& value() const& {
//         assert(m_init);
//         return m_value;
//     }
//     T& value() & {
//         assert(m_init);
//         return m_value;
//     }
//     const T&& value() const&& {
//         assert(m_init);
//         return std::move(m_value);
//     }
//     T&& value() && {
//         assert(m_init);
//         return std::move(m_value);
//     }

//     std::error_code error() const {
//         assert(!m_init);
//         return m_error.type;
//     }

//     bool     has_value() const { return m_init; }
//     explicit operator bool() const { return m_init; }

// private:
//     void destroy() {
//         if (m_init) { m_value.~T(); }
//     }
//     union {
//         T     m_value;
//         Error m_error;
//     };
//     bool m_init;
// };

constexpr static auto hash(const char* str) -> size_t {
    const i64 p = 131;
    const i64 m = 4294967291; // 2^32 - 5, largest 32 bit prime
    i64       total = 0;
    i64       current_multiplier = 1;
    for (i64 i = 0; str[i] != '\0'; ++i) {
        total = (total + current_multiplier * str[i]) % m;
        current_multiplier = (current_multiplier * p) % m;
    }
    return total;
}

template<typename T>
class UniquePointer {};
} // namespace
  // T1

void* malloc(size_t size);
template<typename T>
auto jf_new() -> Option<T> {
    T* p = new T;
    return Option<T>{p};
}

} // namespace JadeFrame