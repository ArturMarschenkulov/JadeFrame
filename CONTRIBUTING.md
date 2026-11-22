# Coding conventions

These conventions follow the existing code and the naming rules in
[docs/design.md](docs/design.md). Older code has exceptions; avoid unrelated
renaming or reformatting when working nearby.

## Naming and files

- Put framework code in the `JadeFrame` namespace and appropriate nested namespaces.
- Use `PascalCase` for classes and structs, such as `AssetLoader` and `RendererError`.
  Primitive-like math types retain names such as `v3` and `mat4x4`.
- Use `snake_case` for functions, parameters, and local variables.
- Prefix data members with `m_`, globals with `g_`, and static data members with
  `s_`, following the surrounding module.
- Use scoped enums (`enum class`). Existing enum type names include both
  `GRAPHICS_API` and `RendererErrorCode`; retain the convention of the API being
  extended. Likewise, match its enumerator spelling rather than renaming it.
- Use lowercase `snake_case` filenames with `.h` and `.cpp` extensions. Headers
  use `#pragma once`.

## C++ style

- Use C++20 and trailing return types, including `auto update() -> void`.
- Use the numeric aliases from `JadeFrame/types.h`, such as `u32`, `i32`, and
  `f32`, where they describe framework data. Keep external API types where required
  by an interface and use appropriate size types for container sizes.
- Use C++ casts such as `static_cast`, rather than C-style casts. Make narrowing
  conversions explicit and validate input ranges where needed.
- Use `const` for values that do not change and const-qualified member functions
  for read-only operations. Mark meaningful results `[[nodiscard]]` when ignoring
  them would likely be a mistake.
- Initialize members and make ownership explicit. Follow the resource type's
  established RAII or explicit initialization/cleanup model, including cleanup
  after partial initialization. Use `noexcept` only when the operation guarantees it.
- Prefer descriptor structs for complex construction instead of long sequences
  of positional arguments.
- Include the headers a file uses directly; do not depend on the framework's
  private precompiled header or incidental transitive includes. A `.cpp` file
  should normally include its matching header first. Follow nearby include
  grouping; the formatter deliberately does not sort includes.

## Formatting

[.clang-format](.clang-format) is the formatting reference: four-space indentation,
a 90-column limit, attached opening braces, and left-aligned pointer/reference
markers (`T*`, `const T&`). It also controls wrapping and alignment.

Format the files you changed, using a clang-format version that accepts the
checked-in configuration:

```sh
clang-format -i JadeFrame/src/JadeFrame/graphics/camera.h \
  JadeFrame/src/JadeFrame/graphics/camera.cpp
```

Substitute your changed files. Keep broad formatting cleanups separate from
behavior changes and do not recursively format third-party code.
[.clang-tidy](.clang-tidy) provides additional analysis settings; it is not
currently a mandatory CI check.

## Errors and API contracts

Follow existing `Result<T, E>` and `Option<T>` APIs for fallible operations and
optional values. Preserve structured error codes and useful diagnostic context.
Handle returned failures instead of ignoring them or replacing them with success
defaults. Assertions are suitable for internal invariants, not recoverable file,
input, or driver failures. Do not introduce a blanket exception policy across
modules that already have different contracts.

Document ownership, lifetime, failure behavior, and graphics conventions at public
API boundaries. For graphics changes, be explicit about coordinate spaces,
handedness, matrix order, depth ranges, image row order, and synchronization when
they affect callers. Use `///` comments for public API documentation, and ordinary
comments to explain non-obvious implementation decisions. Avoid comments that
only restate the code.
