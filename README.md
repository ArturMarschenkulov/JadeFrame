# JadeFrame

JadeFrame is a C++ Graphics Framework. The point of project is to understand low-level graphics programming better as well as programming in general.

One of the main goals is to use as few external code as possible.

# Building and running it
The most generic way to build the project is:
```
mkdir build
cd build
cmake ../.
cmake --build . -j10
./Application/Application
```

If you have `ninja` you can also build it like this on windows or linux:
```
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE={Debug|Release|RelWithDebInfo} $SOURCE_DIR
ninja
./Application/Application
```

A quick way to compile and run it if success in powershell is:
```
cmake --build . -j8; if ($?) { .\Application\Application }
```


# Directory structure
The whole project should be organized into modules (aka namespaces), which should be as modular and independent as possible. Roughly speaking, a folder should correspond to one module. Every folder should have a `README.md` file, which should contain a short description of the module.

# Conventions
## Naming convention
The whole project should conform to the following naming convention:

- All code written should be in the `JadeFrame` namespace.
- Classes/structs should be be in `PascalCase` style. Exception exist mainly for "primitive-like" classes, mainly math related ones, that is Matrix and Vector classes, e.g. `mat4` and `v3`.
- Local variables should be in `snake_case`.
- Global variables should be in `snake_case`, but with a `g_` prefix.
- Member variables should be in `snake_case`, but with a `m_` prefix.
- Functions and member functions should be in `snake_case`.
- For more complex classes, one should use Descriptor structs on constructors.
## Coding convention
- trailing return type
- use of own datatypes, especially primitive ones `i32`, `u8`, etc.
- C++ style type casting, no C style
# Inspirations
The following is a list of interesting projects and resources in general, which may be relevant to the project.
## Engine-like
Those are projects which have an editor abstraction. Meaning in normal use one does not write actual code.
- [Blender](https://github.com/blender/blender)
- [Godot](https://github.com/blender/blender)
- [Unreal Engine](https://github.com/EpicGames/UnrealEngine)
- [CryEngine](https://github.com/ValtoGameEngines/CryEngine)
- [Banshee Engine](https://github.com/ValtoGameEngines/Banshee-Engine)
- [FlaxEngine](https://github.com/FlaxEngine/FlaxEngine)
- [SpartanEngine](https://github.com/PanosK92/SpartanEngine)
- [limonEngine](https://github.com/enginmanap/limonEngine)
- [Horde3D](https://github.com/horde3d/Horde3D)

## Framework-like
Those are projects where one has to actually write code.
- [Ogre](https://github.com/OGRECave/ogre): Has multiple Graphics APIs. Opengl, Vulkan,...
- [Urho3D](https://github.com/urho3d/Urho3D)
- [Handmade Hero](https://github.com/HandmadeHero/cpp)
- [CrystalSpace3D](https://github.com/crystalspace/CS)
- [JUCE](https://github.com/juce-framework/JUCE)
- [NoahGameFrame](https://github.com/ketoo/NoahGameFrame)
- [olcConsoleGameEngine](https://github.com/OneLoneCoder/videos)
- [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine)
- [McEngine](https://github.com/McKay42/McEngine)
- [HorizonEngine](https://github.com/FergusGriggs/HorizonEngine)
- [SFML](https://github.com/SFML/SFML)
- [Cell](https://github.com/JoeyDeVries/Cell)
- [MagnumEngine](https://github.com/mosra/magnum)
- [Hazel](https://github.com/TheCherno/Hazel)
- [Sparky](https://github.com/TheCherno/Sparky)
- [raylib](https://github.com/raysan5/raylib)
- [toy](https://github.com/hugoam/toy)
- [cinder](https://github.com/cinder/Cinder)
