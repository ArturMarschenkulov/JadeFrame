#include "linux_input_manager.h"

namespace JadeFrame {

auto translate_key(KeySym keysym) -> KEY {
    switch (keysym) {
        case XK_a: return KEY::A;
        case XK_b: return KEY::B;
        case XK_c: return KEY::C;
        case XK_d: return KEY::D;
        case XK_e: return KEY::E;
        case XK_f: return KEY::F;
        case XK_g: return KEY::G;
        case XK_h: return KEY::H;
        case XK_i: return KEY::I;
        case XK_j: return KEY::J;
        case XK_k: return KEY::K;
        case XK_l: return KEY::L;
        case XK_m: return KEY::M;
        case XK_n: return KEY::N;
        case XK_o: return KEY::O;
        case XK_p: return KEY::P;
        case XK_q: return KEY::Q;
        case XK_r: return KEY::R;
        case XK_s: return KEY::S;
        case XK_t: return KEY::T;
        case XK_u: return KEY::U;
        case XK_v: return KEY::V;
        case XK_w: return KEY::W;
        case XK_x: return KEY::X;
        case XK_y: return KEY::Y;
        case XK_z: return KEY::Z;
        case XK_0: return KEY::ZERO;
        case XK_1: return KEY::ONE;
        case XK_2: return KEY::TWO;
        case XK_3: return KEY::THREE;
        case XK_4: return KEY::FOUR;
        case XK_5: return KEY::FIVE;
        case XK_6: return KEY::SIX;
        case XK_7: return KEY::SEVEN;
        case XK_8: return KEY::EIGHT;
        case XK_9: return KEY::NINE;
        case XK_F1: return KEY::F1;
        case XK_F2: return KEY::F2;
        case XK_F3: return KEY::F3;
        case XK_F4: return KEY::F4;
        case XK_F5: return KEY::F5;
        case XK_F6: return KEY::F6;
        case XK_F7: return KEY::F7;
        case XK_F8: return KEY::F8;
        case XK_F9: return KEY::F9;
        case XK_F10: return KEY::F10;
        case XK_F11: return KEY::F11;
        case XK_F12: return KEY::F12;
        case XK_Shift_L: return KEY::LEFT_SHIFT;
        case XK_Control_L: return KEY::LEFT_CONTROL;
        case XK_Alt_L: return KEY::LEFT_ALT;
        case XK_Shift_R: return KEY::RIGHT_SHIFT;
        case XK_Control_R: return KEY::RIGHT_CONTROL;
        case XK_Alt_R: return KEY::RIGHT_ALT;
        case XK_grave: return KEY::GRAVE;
        case XK_slash: return KEY::SLASH;
        case XK_backslash: return KEY::BACKSLASH;
        case XK_Super_L: return KEY::LEFT_SUPER;
        case XK_space: return KEY::SPACE;
        case XK_Return: return KEY::ENTER;
        case XK_Escape: return KEY::ESCAPE;
        case XK_Tab: return KEY::TAB;
        case XK_BackSpace: return KEY::BACKSPACE;
        case XK_Up: return KEY::UP;
        case XK_Down: return KEY::DOWN;
        case XK_Left: return KEY::LEFT;
        case XK_Right: return KEY::RIGHT;
        case XK_Insert: return KEY::INSERT;
        case XK_Delete: return KEY::DELET;
        case XK_Home: return KEY::HOME;
        case XK_End: return KEY::END;
        case XK_Page_Up: return KEY::PAGE_UP;
        case XK_Page_Down: return KEY::PAGE_DOWN;
        case XK_Caps_Lock: return KEY::CAPS_LOCK;
        case XK_Num_Lock: return KEY::NUM_LOCK;
        case XK_Scroll_Lock: return KEY::SCROLL_LOCK;
        case XK_Print: return KEY::PRINT_SCREEN;
        case XK_Pause: return KEY::PAUSE;
        default: return KEY::MAX;
    }
}
} // namespace JadeFrame