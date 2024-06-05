#pragma once
#include <JadeFrame.h>

namespace JadeFrame {
struct Example_Empty : public BaseApp {
    explicit Example_Empty(const Desc& desc);
    ~Example_Empty() override = default;

    auto on_init() -> void override;
    auto on_update() -> void override;
    auto on_draw() -> void override;

public:
    // std::deque<Mesh> m_meshes;
    // std::vector<Object> m_objs;
};

inline Example_Empty::Example_Empty(const Desc& desc)
    : BaseApp(desc) {
    // m_current_app_p = reinterpret_cast<Example_Empty*>(this);
}

inline auto Example_Empty::on_init()->void {}

inline auto Example_Empty::on_update()->void {}

inline auto Example_Empty::on_draw()->void {}

using TestApp = Example_Empty;
} // namespace JadeFrame