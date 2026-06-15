# JadeFrame

JadeFrame is a C++ Graphics Framework. This is a personal project, where I program a graphics framework mostly from scratch. With this my goal is to understand and apply graphics programming concepts better, as well as programming large projects in general.

It started off as an attempt to replicate the p5.js (Javascript) and Processing (Java) frameworks in C++ using OpenGL, while trying to do [The Coding Train](https://www.youtube.com/@TheCodingTrain)'s challenges. It has evolved into a more general graphics framework, which ideally should be able to compete with [raylib](https://github.com/raysan5/raylib). Maybe one day, it could be used as the foundation for a game engine (JadeEngine???).

Currently, a major goal is to create a Graphics API agnostic front-end, which can be used with different back-ends, like OpenGL, Vulkan, D3D11, D3D12, Software Rendering, Terminal Rendering, etc, akin to what [LLGL](https://github.com/LukasBanana/LLGL) does. As of right now, the only back-end is OpenGL and Vulkan. In a way I regard this as a proof of concept that I understand the concepts behind graphics programming instead of simply knowing the graphics API.

Other goals include:
- Designing a "good" API
- Learning how to organize a large C++ project
- Learning and applying C++ best practices
- Writing clean code
- Writing as much from scratch as possible to better understand the internals

# Building and running it
Use the repository build script from the project root:

```sh
./build.sh build
```

This configures CMake into `build/` and builds the default targets. Examples are built by default and can be run from their generated directories, for example:

```sh
./build/examples/hello_triangle/Example_hello_triangle
```

The build script supports these commands:

```sh
./build.sh build       # Configure and build into build/
./build.sh test        # Run ctest --verbose from build/
./build.sh build_test  # Configure build/ and invoke CMake's test target
./build.sh clear       # Remove build/CMakeCache.txt
./build.sh remove      # Remove the build/ directory
```

Run `./build.sh build` before `./build.sh test`.

To build JadeFrame-owned targets with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -S . -B build-sanitize \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DJADEFRAME_ENABLE_SANITIZERS=ON \
  -DJADEFRAME_SANITIZERS=address,undefined
cmake --build build-sanitize --parallel
ctest --test-dir build-sanitize --output-on-failure
```


# Directory structure
The whole project should be organized into modules (aka namespaces), which should be as modular and independent as possible. Roughly speaking, a folder should correspond to one module. Every folder should have a `README.md` file, which should contain a short description of the module.


- `JadeFrame`: The main module. This is where the main code is.
- `docs`: Contains meta information about this project.
- `examples`: Contains example programs.
- `cmake`: Contains shared CMake helper modules.
- `scripts`: Contains project helper scripts.
