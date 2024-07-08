#pragma once
#include "JadeFrame/math/vec.h"
#include "JadeFrame/math/mat_4.h"

#include <vector>

namespace JadeFrame {

class RGBAColor {
public:
    constexpr RGBAColor() = default;

    constexpr RGBAColor(f32 red, f32 green, f32 blue, f32 alpha)
        : r(red)
        , g(green)
        , b(blue)
        , a(alpha) {}

    constexpr RGBAColor(f32 red, f32 green, f32 blue)
        : RGBAColor{red, green, blue, 1.0F} {}

    constexpr static float COLOR_SCALE = 256.0F;

    constexpr RGBAColor(u8 red, u8 green, u8 blue)
        : RGBAColor{red, green, blue, 1} {}

    constexpr RGBAColor(u8 red, u8 green, u8 blue, u8 alpha)
        : r(static_cast<float>(red) / COLOR_SCALE)
        , g(static_cast<float>(green) / COLOR_SCALE)
        , b(static_cast<float>(blue) / COLOR_SCALE)
        , a(static_cast<float>(alpha) / COLOR_SCALE) {}

    auto operator==(const RGBAColor& color) const -> bool {
        return r == color.r && g == color.g && b == color.b && a == color.a;
    }

    auto operator!=(const RGBAColor& color) const -> bool { return !(*this == color); }

    constexpr static auto from_hex(u32 hex) -> RGBAColor {
        constexpr u32 RED_SHIFT = 24;
        constexpr u32 GREEN_SHIFT = 16;
        constexpr u32 BLUE_SHIFT = 8;
        constexpr u32 ALPHA_SHIFT = 0;
        constexpr u32 MASK = 0xFF;

        u32 red = (hex >> RED_SHIFT) & MASK;
        u32 green = (hex >> GREEN_SHIFT) & MASK;
        u32 blue = (hex >> BLUE_SHIFT) & MASK;
        u32 alpha = (hex >> ALPHA_SHIFT) & MASK;

        return {(u8)red, (u8)green, (u8)blue, (u8)alpha};
    }

    static auto from_hsv(f32 h, f32 s, f32 v) -> RGBAColor {
        f32  c = v * s;
        f32  x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        auto m = v - c;
        auto r = 0.0f;
        f32  g = 0.0f;
        auto b = 0.0f;
        if (h >= 0.0f && h < 60.0f) {
            r = c;
            g = x;
            b = 0.0f;
        } else if (h >= 60.0f && h < 120.0f) {
            r = x;
            g = c;
            b = 0.0f;
        } else if (h >= 120.0f && h < 180.0f) {
            r = 0.0f;
            g = c;
            b = x;
        } else if (h >= 180.0f && h < 240.0f) {
            r = 0.0f;
            g = x;
            b = c;
        } else if (h >= 240.0f && h < 300.0f) {
            r = x;
            g = 0.0f;
            b = c;
        } else if (h >= 300.0f && h < 360.0f) {
            r = c;
            g = 0.0f;
            b = x;
        }
        return RGBAColor(r + m, g + m, b + m, 1.0f);
    }

    // linear to srgb
    [[nodiscard]] auto gamma_decode() const -> RGBAColor {
        constexpr const f32 ratio = 2.2F;
        return RGBAColor(std::pow(r, ratio), std::pow(g, ratio), std::pow(b, ratio), a);
    }

    // srgb to linear
    [[nodiscard]] auto gamma_encode() const -> RGBAColor {
        constexpr const f32 ratio = 1.0F / 2.2F;
        return RGBAColor(std::pow(r, ratio), std::pow(g, ratio), std::pow(b, ratio), a);
    }

    static auto convert_srgb_from_linear(float lin_val) -> float {
        constexpr const f32 ratio = 1.0F / 2.4F;
        constexpr const f32 threshold = 0.0031308F;

        return lin_val <= threshold ? lin_val * 12.92F
                                    : powf(lin_val, ratio) * 1.055F - 0.055F;
    }

    static auto convert_srgb_to_linear(float rgba_val) -> float {
        constexpr const f32 ratio = 2.4F;
        constexpr const f32 threshold = 0.04045F;
        return rgba_val <= threshold ? rgba_val / 12.92F
                                     : powf((rgba_val + 0.055F) / 1.055F, ratio);
    }

    static auto from_hsl(f32 h, f32 s, f32 l) -> RGBAColor {
        f32  c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
        f32  x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        auto m = l - c / 2.0f;
        auto r = 0.0f;
        auto g = 0.0f;
        auto b = 0.0f;
        if (h >= 0.0f && h < 60.0f) {
            r = c;
            g = x;
            b = 0.0f;
        } else if (h >= 60.0f && h < 120.0f) {
            r = x;
            g = c;
            b = 0.0f;
        } else if (h >= 120.0f && h < 180.0f) {
            r = 0.0f;
            g = c;
            b = x;
        } else if (h >= 180.0f && h < 240.0f) {
            r = 0.0f;
            g = x;
            b = c;
        } else if (h >= 240.0f && h < 300.0f) {
            r = x;
            g = 0.0f;
            b = c;
        } else if (h >= 300.0f && h < 360.0f) {
            r = c;
            g = 0.0f;
            b = x;
        }
        return RGBAColor(r + m, g + m, b + m, 1.0f);
    }

    [[nodiscard]] auto to_hex() const -> u32 {
        auto red = static_cast<u32>(r * COLOR_SCALE);
        auto green = static_cast<u32>(g * COLOR_SCALE);
        auto blue = static_cast<u32>(b * COLOR_SCALE);
        auto alpha = static_cast<u32>(a * COLOR_SCALE);

        constexpr u32 RED_SHIFT = 24;
        constexpr u32 GREEN_SHIFT = 16;
        constexpr u32 BLUE_SHIFT = 8;
        constexpr u32 ALPHA_SHIFT = 0;

        return (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT) |
               (alpha << ALPHA_SHIFT);
    }

    [[nodiscard]] auto as_rgb() const -> RGBAColor { return RGBAColor(r, g, b, 1.0F); }

    // Inspirations for color names
    // https://www.w3schools.com/colors/colors_names.asp
    // http://www.novact.info/id40.html
    // https://margaret2.github.io/pantone-colors/
    // http://people.csail.mit.edu/jaffer/Color/Dictionaries

    static auto solid_black() -> RGBAColor { return {0.0F, 0.0F, 0.0F}; }

    static auto solid_grey() -> RGBAColor { return {0.5F, 0.5F, 0.5F}; }

    static auto solid_white() -> RGBAColor { return {1.0F, 1.0F, 1.0F}; }

    static auto solid_red() -> RGBAColor { return {1.0F, 0.0F, 0.0F}; }

    static auto solid_green() -> RGBAColor { return {0.0F, 1.0F, 0.0F}; }

    static auto solid_blue() -> RGBAColor { return {0.0F, 0.0F, 1.0F}; }

    static auto solid_yellow() -> RGBAColor { return {1.0F, 1.0F, 0.0F}; }

    static auto solid_cyan() -> RGBAColor { return {0.0F, 1.0F, 1.0F}; }

    static auto solid_magenta() -> RGBAColor { return {1.0F, 0.0F, 1.0F}; }

    static auto solid_magenta_2() -> RGBAColor { return {1.0F, 0.0F, 0.5F}; }

    static auto solid_orange() -> RGBAColor { return {1.0F, 0.5F, 0.0F}; }

    static auto solid_transparent() -> RGBAColor { return {0.0F, 0.0F, 0.0F, 0.0F}; }

    [[nodiscard]] constexpr auto set_opacity(f32 opacity) const -> RGBAColor {
        return {r, g, b, opacity};
    }

public:
    f32 r, g, b, a;
};

struct Vertex {
    v3        position;
    RGBAColor color;
    v2        tex_coord;
};

// struct v3f32;
class VertexData {
public:
    std::vector<v3>        m_positions;
    std::vector<RGBAColor> m_colors;
    std::vector<v2>        m_texture_coordinates;
    std::vector<v3>        m_normals;

    std::vector<u32> m_indices;

    struct Desc {
        bool has_position = true; // NOTE: Probably unneccessary
        bool has_texture_coordinates = true;
        bool has_indices = false;
        bool has_normals = true;
    };

    auto set_color(const RGBAColor& color) -> void {
        auto num_vertices = m_positions.size();
        m_colors.clear();
        m_colors.resize(num_vertices, color);
    }

    static auto make_line(const v3& pos1, const v3& pos2) -> VertexData;

    static auto make_rectangle(
        const v3&  pos,
        const v3&  size,
        const Desc desc = Desc{true, true, false, true}
    ) -> VertexData;
    static auto make_triangle(const v3& pos1, const v3& pos2, const v3& pos3)
        -> VertexData;
    static auto make_circle(const v3& position, const f32 radius, const u32 numSegments)
        -> VertexData;

    static auto make_cube(const v3& pos, const v3& size) -> VertexData;
};

auto convert_into_data(const VertexData& vertex_data, const bool interleaved)
    -> std::vector<f32>;

} // namespace JadeFrame