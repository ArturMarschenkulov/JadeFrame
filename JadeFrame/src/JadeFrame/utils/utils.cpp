#include "pch.h"
#include "utils.h"

#include <cstdlib>
#include <ctime>

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

} // namespace JadeFrame