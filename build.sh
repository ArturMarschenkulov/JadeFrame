#!/bin/bash

build_dir="build"


# if clean is passed as an argument, remove the build directory
if [ "$1" == "clean" ]; then
    if [ -d $build_dir ]; then
        rm -r $build_dir
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
    ./examples/0/Example_0
else
    echo "Build failed"
fi
