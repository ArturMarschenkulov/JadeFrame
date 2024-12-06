#!/bin/bash

build_dir="build"


# if `remove` is passed as an argument, remove the build directory
if [ "$1" == "remove" ]; then
    if [ -d $build_dir ]; then
        rm -r $build_dir
    fi
    exit 0
fi

# if `clear` is passed as an argument, remove the cmake cache called CMakeCache.txt
if [ "$1" == "clear" ]; then
    cd $build_dir
    if [ -f CMakeCache.txt ]; then
        rm CMakeCache.txt
    fi
    exit 0
fi

if [ ! -d $build_dir ]; then
    mkdir $build_dir
fi
cd $build_dir

# cmake ../. && cmake --build . -j10 && ./Application/Application

if cmake ../. && cmake --build . -j10; then
    echo "Build successful"
    compiledb -n make
    echo "Compiledb generated"
    echo "Running Application"
    cd examples/0/
    ./Example_0
else
    echo "Build failed"
fi
