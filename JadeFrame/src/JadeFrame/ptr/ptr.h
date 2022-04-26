#include <memory>
namespace JadeFrame {
namespace ptr {
template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
auto make_scope(T* p) {
    return std::make_unique<T>(p);
    return std::unique_ptr<T>(p);
}

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
class NonNull {};




} // namespace ptr
} // namespace JadeFrame