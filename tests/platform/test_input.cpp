#include <gtest/gtest.h>

#include "JadeFrame/platform/window.h"
#include "JadeFrame/platform/window_event.h"

// NOTE: The X11 headers define macros that clash with gtest, so they have to come last.
#if defined(__linux__)
    #include "JadeFrame/platform/linux/linux_input_manager.h"
#endif

using namespace JadeFrame;

namespace {

constexpr u32 g_key_count = static_cast<u32>(KEY::MAX);
constexpr u32 g_button_count = static_cast<u32>(BUTTON::MAX);

const KEY    g_first_key = static_cast<KEY>(0);
const KEY    g_last_key = static_cast<KEY>(g_key_count - 1);
const BUTTON g_first_button = static_cast<BUTTON>(0);
const BUTTON g_last_button = static_cast<BUTTON>(g_button_count - 1);

// Everything that is not a real key or button: the MAX sentinel and values that can only
// be produced through a cast.
const std::array<KEY, 3> g_invalid_keys = {
    KEY::MAX,
    static_cast<KEY>(g_key_count + 1),
    static_cast<KEY>(0xFF),
};
const std::array<BUTTON, 3> g_invalid_buttons = {
    BUTTON::MAX,
    static_cast<BUTTON>(g_button_count + 1),
    static_cast<BUTTON>(0xFF),
};

auto press(InputState& input, KEY key) -> bool {
    return input.process_event(WindowEvent::make_key_event(key, INPUT_STATE::PRESSED));
}

auto release(InputState& input, KEY key) -> bool {
    return input.process_event(WindowEvent::make_key_event(key, INPUT_STATE::RELEASED));
}

auto press(InputState& input, BUTTON button) -> bool {
    return input.process_event(
        WindowEvent::make_button_event(button, INPUT_STATE::PRESSED)
    );
}

auto release(InputState& input, BUTTON button) -> bool {
    return input.process_event(
        WindowEvent::make_button_event(button, INPUT_STATE::RELEASED)
    );
}

// An input state in which every key and button is held, so that a stray write of either
// PRESSED or RELEASED would be noticed.
auto make_busy_state() -> InputState {
    InputState input;
    for (u32 i = 0; i < g_key_count; i += 2) { press(input, static_cast<KEY>(i)); }
    for (u32 i = 0; i < g_button_count; i += 2) { press(input, static_cast<BUTTON>(i)); }
    input.process_event(WindowEvent::make_mouse_event(12, 34));
    input.update();
    for (u32 i = 1; i < g_key_count; i += 2) { press(input, static_cast<KEY>(i)); }
    for (u32 i = 1; i < g_button_count; i += 2) { press(input, static_cast<BUTTON>(i)); }
    return input;
}

auto expect_same_state(const InputState& actual, const InputState& expected) -> void {
    EXPECT_EQ(actual.m_curr_key_state, expected.m_curr_key_state);
    EXPECT_EQ(actual.m_prev_key_state, expected.m_prev_key_state);
    EXPECT_EQ(actual.m_curr_button_state, expected.m_curr_button_state);
    EXPECT_EQ(actual.m_prev_button_state, expected.m_prev_button_state);
    EXPECT_EQ(actual.m_mouse_pos.x, expected.m_mouse_pos.x);
    EXPECT_EQ(actual.m_mouse_pos.y, expected.m_mouse_pos.y);
}

} // namespace

TEST(InputValidation, AcceptsExactlyTheEnumeratorsBelowMax) {
    for (u32 i = 0; i <= 0xFF; i++) {
        EXPECT_EQ(is_valid(static_cast<KEY>(i)), i < g_key_count) << "key " << i;
        EXPECT_EQ(is_valid(static_cast<BUTTON>(i)), i < g_button_count) << "button " << i;
    }
    EXPECT_FALSE(is_valid(KEY::MAX));
    EXPECT_FALSE(is_valid(BUTTON::MAX));
}

TEST(InputState, InvalidKeyEventsAreIgnored) {
    const InputState expected = make_busy_state();
    for (const KEY key : g_invalid_keys) {
        InputState input = expected;
        EXPECT_FALSE(press(input, key));
        expect_same_state(input, expected);
        EXPECT_FALSE(release(input, key));
        expect_same_state(input, expected);
    }
}

TEST(InputState, InvalidButtonEventsAreIgnored) {
    const InputState expected = make_busy_state();
    for (const BUTTON button : g_invalid_buttons) {
        InputState input = expected;
        EXPECT_FALSE(press(input, button));
        expect_same_state(input, expected);
        EXPECT_FALSE(release(input, button));
        expect_same_state(input, expected);
    }
}

TEST(InputState, InvalidKeyQueriesReturnFalse) {
    const InputState input = make_busy_state();
    for (const KEY key : g_invalid_keys) {
        EXPECT_FALSE(input.is_key_down(key));
        EXPECT_FALSE(input.is_key_up(key));
        EXPECT_FALSE(input.is_key_pressed(key));
        EXPECT_FALSE(input.is_key_released(key));
    }
}

TEST(InputState, InvalidButtonQueriesReturnFalse) {
    const InputState input = make_busy_state();
    for (const BUTTON button : g_invalid_buttons) {
        EXPECT_FALSE(input.is_button_down(button));
        EXPECT_FALSE(input.is_button_up(button));
        EXPECT_FALSE(input.is_button_pressed(button));
        EXPECT_FALSE(input.is_button_released(button));
    }
}

TEST(InputState, KeyTransitionsAcrossFrames) {
    InputState input;
    EXPECT_TRUE(input.is_key_up(KEY::W));
    EXPECT_FALSE(input.is_key_down(KEY::W));
    EXPECT_FALSE(input.is_key_pressed(KEY::W));
    EXPECT_FALSE(input.is_key_released(KEY::W));

    // Frame 1: the key goes down.
    input.update();
    EXPECT_TRUE(press(input, KEY::W));
    EXPECT_TRUE(input.is_key_down(KEY::W));
    EXPECT_FALSE(input.is_key_up(KEY::W));
    EXPECT_TRUE(input.is_key_pressed(KEY::W));
    EXPECT_FALSE(input.is_key_released(KEY::W));

    // Frame 2: the key is held.
    input.update();
    EXPECT_TRUE(input.is_key_down(KEY::W));
    EXPECT_FALSE(input.is_key_pressed(KEY::W));
    EXPECT_FALSE(input.is_key_released(KEY::W));

    // Frame 3: the key goes up.
    input.update();
    EXPECT_TRUE(release(input, KEY::W));
    EXPECT_FALSE(input.is_key_down(KEY::W));
    EXPECT_TRUE(input.is_key_up(KEY::W));
    EXPECT_FALSE(input.is_key_pressed(KEY::W));
    EXPECT_TRUE(input.is_key_released(KEY::W));

    // Frame 4: the key stays up.
    input.update();
    EXPECT_TRUE(input.is_key_up(KEY::W));
    EXPECT_FALSE(input.is_key_released(KEY::W));

    // Other keys were never affected.
    EXPECT_TRUE(input.is_key_up(KEY::S));
}

TEST(InputState, ButtonTransitionsAcrossFrames) {
    InputState input;
    EXPECT_TRUE(input.is_button_up(BUTTON::RIGHT));
    EXPECT_FALSE(input.is_button_pressed(BUTTON::RIGHT));

    input.update();
    EXPECT_TRUE(press(input, BUTTON::RIGHT));
    EXPECT_TRUE(input.is_button_down(BUTTON::RIGHT));
    EXPECT_FALSE(input.is_button_up(BUTTON::RIGHT));
    EXPECT_TRUE(input.is_button_pressed(BUTTON::RIGHT));
    EXPECT_FALSE(input.is_button_released(BUTTON::RIGHT));

    input.update();
    EXPECT_TRUE(input.is_button_down(BUTTON::RIGHT));
    EXPECT_FALSE(input.is_button_pressed(BUTTON::RIGHT));

    input.update();
    EXPECT_TRUE(release(input, BUTTON::RIGHT));
    EXPECT_TRUE(input.is_button_up(BUTTON::RIGHT));
    EXPECT_TRUE(input.is_button_released(BUTTON::RIGHT));

    input.update();
    EXPECT_FALSE(input.is_button_released(BUTTON::RIGHT));
    EXPECT_TRUE(input.is_button_up(BUTTON::LEFT));
}

TEST(InputState, MouseAndWindowEventsAreAccepted) {
    InputState input;
    EXPECT_TRUE(input.process_event(WindowEvent::make_mouse_event(640, 480)));
    EXPECT_EQ(input.get_mouse_position().x, 640.0F);
    EXPECT_EQ(input.get_mouse_position().y, 480.0F);

    const InputState expected = input;
    EXPECT_TRUE(input.process_event(WindowEvent::make_resize_event(800, 600)));
    EXPECT_TRUE(input.process_event(WindowEvent::make_close_event()));
    expect_same_state(input, expected);
}

TEST(InputState, FirstAndLastKeysAndButtonsAreUsable) {
    InputState input;
    for (const KEY key : {g_first_key, g_last_key}) {
        EXPECT_TRUE(press(input, key));
        EXPECT_TRUE(input.is_key_down(key));
        EXPECT_TRUE(input.is_key_pressed(key));
    }
    for (const BUTTON button : {g_first_button, g_last_button}) {
        EXPECT_TRUE(press(input, button));
        EXPECT_TRUE(input.is_button_down(button));
        EXPECT_TRUE(input.is_button_pressed(button));
    }
    EXPECT_EQ(input.m_curr_key_state.front(), INPUT_STATE::PRESSED);
    EXPECT_EQ(input.m_curr_key_state.back(), INPUT_STATE::PRESSED);
    EXPECT_EQ(input.m_curr_button_state.front(), INPUT_STATE::PRESSED);
    EXPECT_EQ(input.m_curr_button_state.back(), INPUT_STATE::PRESSED);

    // One past the last element is rejected.
    const InputState expected = input;
    EXPECT_FALSE(press(input, static_cast<KEY>(g_key_count)));
    EXPECT_FALSE(press(input, static_cast<BUTTON>(g_button_count)));
    expect_same_state(input, expected);
}

TEST(InputState, EveryValidKeyAndButtonMapsToItsOwnSlot) {
    for (u32 i = 0; i < g_key_count; i++) {
        InputState input;
        EXPECT_TRUE(press(input, static_cast<KEY>(i)));
        for (u32 j = 0; j < g_key_count; j++) {
            EXPECT_EQ(input.is_key_down(static_cast<KEY>(j)), i == j);
        }
    }
    for (u32 i = 0; i < g_button_count; i++) {
        InputState input;
        EXPECT_TRUE(press(input, static_cast<BUTTON>(i)));
        for (u32 j = 0; j < g_button_count; j++) {
            EXPECT_EQ(input.is_button_down(static_cast<BUTTON>(j)), i == j);
        }
    }
}

#if defined(__linux__)

TEST(LinuxInputTranslation, KnownKeySymbolsAndButtonsTranslate) {
    EXPECT_EQ(translate_key(XK_a), KEY::A);
    EXPECT_EQ(translate_key(XK_z), KEY::Z);
    EXPECT_EQ(translate_key(XK_space), KEY::SPACE);
    EXPECT_EQ(translate_key(XK_Escape), KEY::ESCAPE);
    EXPECT_EQ(translate_button(Button1), BUTTON::LEFT);
    EXPECT_EQ(translate_button(Button2), BUTTON::MIDDLE);
    EXPECT_EQ(translate_button(Button3), BUTTON::RIGHT);
}

TEST(LinuxInputTranslation, UnknownKeySymbolsAreIgnored) {
    const std::array<KeySym, 6> unknown_keysyms = {
        NoSymbol, XK_Hyper_L, XK_Menu, XK_KP_Enter, 0x1008FF11 /* XF86 volume */,
        ~static_cast<KeySym>(0),
    };
    const InputState expected = make_busy_state();
    for (const KeySym keysym : unknown_keysyms) {
        const KEY key = translate_key(keysym);
        EXPECT_FALSE(is_valid(key)) << "keysym " << keysym;

        InputState input = expected;
        EXPECT_FALSE(press(input, key));
        EXPECT_FALSE(release(input, key));
        expect_same_state(input, expected);
    }
}

TEST(LinuxInputTranslation, UnknownButtonCodesAreIgnored) {
    // 4-7 are the scroll buttons, 8 and 9 the usual back and forward buttons.
    const std::array<unsigned int, 9> unknown_buttons = {
        0, Button4, Button5, 6, 7, 8, 9, 255, ~0U,
    };
    const InputState expected = make_busy_state();
    for (const unsigned int x11_button : unknown_buttons) {
        const BUTTON button = translate_button(x11_button);
        EXPECT_FALSE(is_valid(button)) << "button " << x11_button;

        InputState input = expected;
        EXPECT_FALSE(press(input, button));
        EXPECT_FALSE(release(input, button));
        expect_same_state(input, expected);
    }
}

#endif
