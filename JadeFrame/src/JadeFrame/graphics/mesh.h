#pragma once
#include "JadeFrame/math/vec.h"
#include "JadeFrame/math/mat_4.h"

#include <vector>

namespace JadeFrame {

class RGBAColor {
public:
    RGBAColor() = default;

    RGBAColor(f32 r, f32 g, f32 b, f32 a)
        : r(r)
        , g(g)
        , b(b)
        , a(a) {}

    RGBAColor(f32 r, f32 g, f32 b)
        : RGBAColor(r, g, b, 1.0f) {}

    RGBAColor(u8 r, u8 g, u8 b)
        : RGBAColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f) {}

    RGBAColor(u8 r, u8 g, u8 b, u8 a)
        : r(r / 256.0f)
        , g(g / 256.0f)
        , b(b / 256.0f)
        , a(a / 256.0f) {}

    auto operator==(const RGBAColor& color) const -> bool {
        return r == color.r && g == color.g && b == color.b && a == color.a;
    }

    auto operator!=(const RGBAColor& color) const -> bool { return !(*this == color); }

    static auto from_hex(u32 hex) -> RGBAColor {
        auto red = (hex >> 24) & 0xFF;
        auto green = (hex >> 16) & 0xFF;
        auto blue = (hex >> 8) & 0xFF;
        auto alpha = (hex >> 0) & 0xFF;
        return RGBAColor((u8)red, (u8)green, (u8)blue, (u8)alpha);
    }

    static auto from_hsv(f32 h, f32 s, f32 v) -> RGBAColor {
        auto c = v * s;
        auto x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
        auto m = v - c;
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

    // linear to srgb
    auto gamma_decode() const -> RGBAColor {
        constexpr const f32 ratio = 2.2f;
        return RGBAColor(std::pow(r, ratio), std::pow(g, ratio), std::pow(b, ratio), a);
    }

    // srgb to linear
    auto gamma_encode() const -> RGBAColor {
        constexpr const f32 ratio = 1.0f / 2.2f;
        return RGBAColor(std::pow(r, ratio), std::pow(g, ratio), std::pow(b, ratio), a);
    }

    float convert_srgb_from_linear(float lin_val) {
        constexpr const f32 ratio = 1.0f / 2.4f;
        constexpr const f32 threshold = 0.0031308f;

        return lin_val <= threshold ? lin_val * 12.92f : powf(lin_val, ratio) * 1.055f - 0.055f;
    }

    float convert_srgb_to_linear(float rgba_val) {
        constexpr const f32 ratio = 2.4f;
        constexpr const f32 threshold = 0.04045f;
        return rgba_val <= threshold ? rgba_val / 12.92f : powf((rgba_val + 0.055f) / 1.055f, ratio);
    }

    static auto from_hsl(f32 h, f32 s, f32 l) -> RGBAColor {
        auto c = (1.0f - abs(2.0f * l - 1.0f)) * s;
        auto x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
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

    auto to_hex() const -> u32 {
        auto red = (u8)(r * 256.0f);
        auto green = (u8)(g * 256.0f);
        auto blue = (u8)(b * 256.0f);
        auto alpha = (u8)(a * 256.0f);
        return (red << 24) | (green << 16) | (blue << 8) | (alpha << 0);
    }

    auto as_rgb() const -> RGBAColor { return RGBAColor(r, g, b, 1.0f); }

    // Inspirations for color names
    // https://www.w3schools.com/colors/colors_names.asp
    // http://www.novact.info/id40.html
    // https://margaret2.github.io/pantone-colors/
    // http://people.csail.mit.edu/jaffer/Color/Dictionaries

    static auto solid_black() -> RGBAColor { return RGBAColor(0.0f, 0.0f, 0.0f, 1.0f); }

    static auto solid_grey() -> RGBAColor { return RGBAColor(0.5f, 0.5f, 0.5f, 1.0f); }

    static auto solid_white() -> RGBAColor { return RGBAColor(1.0f, 1.0f, 1.0f, 1.0f); }

    static auto solid_red() -> RGBAColor { return RGBAColor(1.0f, 0.0f, 0.0f, 1.0f); }

    static auto solid_green() -> RGBAColor { return RGBAColor(0.0f, 1.0f, 0.0f, 1.0f); }

    static auto solid_blue() -> RGBAColor { return RGBAColor(0.0f, 0.0f, 1.0f, 1.0f); }

    static auto solid_yellow() -> RGBAColor { return RGBAColor(1.0f, 1.0f, 0.0f, 1.0f); }

    static auto solid_cyan() -> RGBAColor { return RGBAColor(0.0f, 1.0f, 1.0f, 1.0f); }

    static auto solid_magenta() -> RGBAColor { return RGBAColor(1.0f, 0.0f, 1.0f, 1.0f); }

    static auto solid_magenta_2() -> RGBAColor { return RGBAColor(1.0f, 0.0f, 0.5f, 1.0f); }

    static auto solid_orange() -> RGBAColor { return RGBAColor(1.0f, 0.5f, 0.0f, 1.0f); }

    static auto solid_transparent() -> RGBAColor { return RGBAColor(0.0f, 0.0f, 0.0f, 0.0f); }

    auto set_opacity(f32 opacity) -> RGBAColor { return RGBAColor(r, g, b, opacity); }

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

    static auto make_line(const v3& pos1, const v3& pos2) -> VertexData;

    static auto make_rectangle(const v3& pos, const v3& size, const Desc desc = Desc{true, true, false, true})
        -> VertexData;
    static auto make_triangle(const v3& pos1, const v3& pos2, const v3& pos3) -> VertexData;
    static auto make_circle(const v3& position, const f32 radius, const u32 numSegments) -> VertexData;

    static auto make_cube(const v3& pos, const v3& size) -> VertexData;
};

auto convert_into_data(const VertexData& vertex_data, const bool interleaved) -> std::vector<f32>;

} // namespace JadeFrame