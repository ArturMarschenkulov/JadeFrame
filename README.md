# JadeFrame

JadeFrame is a C++ Graphics Framework. The point of project is to understand low-level graphics programming better as well as programming in general.

One of the main goals is to use as few external code as possible.

Design goals:
- as little external code as possible
- clear code
- performance is not critical
- own abstractions
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