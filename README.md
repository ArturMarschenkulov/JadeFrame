# JadeFrame

JadeFrame is a C++ Graphics Framework. This is a personal project, where I program a graphics framework mostly from scratch. With this my goal is to understand and apply graphics programming concepts better, as well as programming large projects in general.

It started off as an attempt to replicate the p5.js (Javascript) and Processing (Java) frameworks in C++ using OpenGL, while trying to do [The Coding Train](https://www.youtube.com/@TheCodingTrain)'s challenges. It has evolved into a more general graphics framework, which ideally should be able to compete with [raylib](https://github.com/raysan5/raylib). Maybe one day, it could be used as the foundation for a game engine (JadeEngine???).

Currently, a major goal is to create a Graphics API agnostic front-end, which can be used with different back-ends, like OpenGL, Vulkan, D3D11, D3D12, Software Rendering, Terminal Rendering, etc. As of right now, the only back-end is OpenGL and Vulkan. In a way I regard this as a proof of concept that I understand the concepts behind graphics programming instead of simply knowing the graphics API.

Other goals include:
- Designing a "good" API
- Learning how to organize a large C++ project
- Learning and applying C++ best practices
- Writing clean code
- trying to write as many things from scratch as possible, to better understand the internals of the things I'm using


# Building and running it
The most generic way to build the project is:
```
mkdir build
cd build
cmake ../.
cmake --build . -j10
./Application/Application
```

If you have `ninja` you can also build it like this on Windows or Linux:
```
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE={Debug|Release|RelWithDebInfo} $SOURCE_DIR
ninja
./Application/Application
```

A quick way to compile and run it if success in PowerShell is:
```
cmake --build . -j8; if ($?) { .\Application\Application }
```


# Directory structure
The whole project should be organized into modules (aka namespaces), which should be as modular and independent as possible. Roughly speaking, a folder should correspond to one module. Every folder should have a `README.md` file, which should contain a short description of the module.


- `JadeFrame`: The main module. This is where the main code is.
- `doc`: Contains meta information about this project.
- `Application`: The main application. This is where the entry point is. (should be replaced by `examples`)
- `examples`: Contains example code. (will replace `Application`)