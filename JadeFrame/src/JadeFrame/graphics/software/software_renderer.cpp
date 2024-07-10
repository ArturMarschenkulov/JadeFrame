#include "software_renderer.h"
#include "JadeFrame/math/vec.h"

enum COLOUR {
    FG_BLACK = 0x0000,
    FG_DARK_BLUE = 0x0001,
    FG_DARK_GREEN = 0x0002,
    FG_DARK_CYAN = 0x0003,
    FG_DARK_RED = 0x0004,
    FG_DARK_MAGENTA = 0x0005,
    FG_DARK_YELLOW = 0x0006,
    FG_GREY = 0x0007, // Thanks MS :-/
    FG_DARK_GREY = 0x0008,
    FG_BLUE = 0x0009,
    FG_GREEN = 0x000A,
    FG_CYAN = 0x000B,
    FG_RED = 0x000C,
    FG_MAGENTA = 0x000D,
    FG_YELLOW = 0x000E,
    FG_WHITE = 0x000F,
    BG_BLACK = 0x0000,
    BG_DARK_BLUE = 0x0010,
    BG_DARK_GREEN = 0x0020,
    BG_DARK_CYAN = 0x0030,
    BG_DARK_RED = 0x0040,
    BG_DARK_MAGENTA = 0x0050,
    BG_DARK_YELLOW = 0x0060,
    BG_GREY = 0x0070,
    BG_DARK_GREY = 0x0080,
    BG_BLUE = 0x0090,
    BG_GREEN = 0x00A0,
    BG_CYAN = 0x00B0,
    BG_RED = 0x00C0,
    BG_MAGENTA = 0x00D0,
    BG_YELLOW = 0x00E0,
    BG_WHITE = 0x00F0,
};

namespace JadeFrame {
class Sprite {
public:
    Sprite() = default;

    Sprite(i32 w, i32 h) { this->create(w, h); }

    explicit Sprite(const std::string& file) {
        if (!this->load(file)) {
            constexpr i32 default_size = 8;
            this->create(default_size, default_size);
        }
    }

    i32 m_width = 0;
    i32 m_height = 0;

private:
    i16* m_glyphs = nullptr;
    i16* m_colors = nullptr;

    void create(i32 w, i32 h) {
        m_width = w;
        m_height = h;
        m_glyphs = new i16[w * h];
        m_colors = new i16[w * h];
        for (i32 i = 0; i < w * h; i++) {
            m_glyphs[i] = L' ';
            m_colors[i] = FG_BLACK;
        }
    }

public:
    void set_glyph(i32 x, i32 y, i16 c) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        } else {
            m_glyphs[y * m_width + x] = c;
        }
    }

    void set_color(i32 x, i32 y, i16 c) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        } else {
            m_colors[y * m_width + x] = c;
        }
    }

    auto get_glyph(i32 x, i32 y) -> i16 {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return L' ';
        } else {
            return m_glyphs[y * m_width + x];
        }
    }

    auto get_color(i32 x, i32 y) -> i16 {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return FG_BLACK;
        } else {
            return m_colors[y * m_width + x];
        }
    }

    auto sample_glyph(f32 x, f32 y) -> i16 {
        i32 sx = (i32)(x * (f32)m_width);
        i32 sy = (i32)(y * (f32)m_height - 1.0F);
        if (sx < 0 || sx >= m_width || sy < 0 || sy >= m_height) {
            return L' ';
        } else {
            return m_glyphs[sy * m_width + sx];
        }
    }

    auto sample_color(f32 x, f32 y) -> i16 {
        i32 sx = (i32)(x * (f32)m_width);
        i32 sy = (i32)(y * (f32)m_height - 1.0F);
        if (sx < 0 || sx >= m_width || sy < 0 || sy >= m_height) {
            return FG_BLACK;
        } else {
            return m_colors[sy * m_width + sx];
        }
    }

    auto save(const std::string& file) -> bool {
        FILE* f = fopen(file.c_str(), "wb");
        if (f == nullptr) { return false; }

        bool res = true;
        res &= fwrite(&m_width, sizeof(i32), 1, f) == 1;
        res &= fwrite(&m_height, sizeof(i32), 1, f) == 1;
        res &= fwrite(m_colors, sizeof(i16), m_width * m_height, f) == m_width * m_height;
        res &= fwrite(m_glyphs, sizeof(i16), m_width * m_height, f) == m_width * m_height;

        int fclose_result = fclose(f);
        return res && (fclose_result == 0);
    }

    auto load(const std::string& file) -> bool {
        delete[] m_glyphs;
        delete[] m_colors;
        m_width = 0;
        m_height = 0;

        FILE* f = fopen(file.c_str(), "rb");
        if (f == nullptr) { return false; }

        std::fread(&m_width, sizeof(i32), 1, f);
        std::fread(&m_height, sizeof(i32), 1, f);

        this->create(m_width, m_height);

        std::fread(m_colors, sizeof(i16), m_width * m_height, f);
        std::fread(m_glyphs, sizeof(i16), m_width * m_height, f);

        std::fclose(f);
        return true;
    }
};

class BufferToDrawTo {
public:
    auto draw(u32 x, u32 y, i16 c) const -> void {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) { return; }
        m_buffer[x + y * m_width] = c;
    }

    auto draw(v2u32 pos, i16 c) const -> void {
        if (pos.x < 0 || pos.x >= m_width || pos.y < 0 || pos.y >= m_height) { return; }
        m_buffer[pos.x + pos.y * m_width] = c;
    }

    auto draw_line(v2u32 pos_0, v2u32 pos_1, short c = 0x2588) const -> void {
        // v2u32 dpos = pos_1 - pos_0;
        // v2u32 dpos1 = dpos.abs();
        // v2u32 px = 2 * dpos1.y - dpos1.x;
        // v2u32 py = 2 * dpos1.x - dpos1.y;

    }

    auto draw_line(int x1, int y1, int x2, int y2, short c = 0x2588) const -> void {

        int dx = x2 - x1;
        int dy = y2 - y1;
        int dx1 = abs(dx);
        int dy1 = abs(dy);
        int px = 2 * dy1 - dx1;
        int py = 2 * dx1 - dy1;
        if (dy1 <= dx1) {
            int x = 0;
            int y = 0;
            int xe = 0;
            int ye = 0;
            if (dx >= 0) {
                x = x1;
                y = y1;
                xe = x2;
            } else {
                x = x2;
                y = y2;
                xe = x1;
            }

            this->draw(x, y, c);

            for (int i = 0; x < xe; i++) {
                x = x + 1;
                if (px < 0) {
                    px = px + 2 * dy1;
                } else {
                    if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                        y = y + 1;
                    } else {
                        y = y - 1;
                    }
                    px = px + 2 * (dy1 - dx1);
                }
                this->draw(x, y, c);
            }
        } else {
            int x = 0;
            int y = 0;
            int xe = 0;
            int ye = 0;
            if (dy >= 0) {
                x = x1;
                y = y1;
                ye = y2;
            } else {
                x = x2;
                y = y2;
                ye = y1;
            }

            this->draw(x, y, c);

            for (int i = 0; y < ye; i++) {
                y = y + 1;
                if (py <= 0) {
                    py = py + 2 * dx1;
                } else {
                    if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
                        x = x + 1;
                    } else {
                        x = x - 1;
                    }
                    py = py + 2 * (dx1 - dy1);
                }
                this->draw(x, y, c);
            }
        }
    }

public:
    u32 m_width = 0;
    u32 m_height = 0;
    u8* m_buffer = nullptr;
};
} // namespace JadeFrame