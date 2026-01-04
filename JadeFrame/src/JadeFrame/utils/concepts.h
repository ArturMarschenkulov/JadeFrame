#pragma once

#include <concepts>

// This file has general user defined concepts.

namespace JadeFrame {

template<class T>
using BaseType = std::remove_const_t<std::remove_reference_t<T>>;

template<class T>
concept Copy =
    std::is_copy_constructible_v<BaseType<T>> &&
    (std::is_copy_assignable_v<BaseType<T>> || !std::is_move_assignable_v<BaseType<T>>);

template<class T>
concept TrivialCopy = Copy<T> && std::is_trivially_copyable_v<BaseType<T>>;

template<class T>
concept Move =
    std::is_move_constructible_v<BaseType<T>> && std::is_move_assignable_v<BaseType<T>>;

template<class T>
concept MoveOrRef = Move<T> || std::is_reference_v<T>;

template<class T>
concept MoveOrRefOrVoid = MoveOrRef<T> || std::is_void_v<T>;
} // namespace JadeFrame