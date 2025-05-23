#!/bin/bash

set -e  # Exit on error

build_dir="build"

# Display usage
usage() {
    echo "Usage: $0 [build|build_test|test|clear|remove]"
    exit 1
}

# Make sure argument is passed
if [ $# -eq 0 ]; then
    usage
fi

case "$1" in
    remove)
        if [ -d "$build_dir" ]; then
            echo "Removing build directory..."
            rm -rf "$build_dir"
        else
            echo "No build directory to remove."
        fi
        ;;
    
    clear)
        if [ -d "$build_dir" ]; then
            cd "$build_dir"
            if [ -f CMakeCache.txt ]; then
                echo "Removing CMake cache..."
                rm CMakeCache.txt
            else
                echo "CMake cache not found."
            fi
        else
            echo "Build directory not found."
        fi
        ;;
    
    test)
        if [ -d "$build_dir" ]; then
            cd "$build_dir"
            ctest --verbose
        else
            echo "Build directory not found. Run build first."
            exit 1
        fi
        ;;
    
    build)
        mkdir -p "$build_dir"
        cd "$build_dir"
        echo "Configuring and building project..."
        cmake ../. -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        cmake --build . -j10
        echo "Generating compilation database..."
        compiledb -n make
        echo "Build successful"
        ;;
    
    build_test)
        mkdir -p "$build_dir"
        cd "$build_dir"
        echo "Configuring and building test target..."
        cmake ../.
        cmake --build . --target test -j10
        echo "Tests build successful"
        ;;
    
    *)
        echo "Running full build and application..."
        mkdir -p "$build_dir"
        cd "$build_dir"
        if cmake ../. && cmake --build . -j10; then
            echo "Build successful"
            compiledb -n make
            echo "Compiledb generated"
            echo "Running Application"
            # cd examples/rotating_primitive/
            # ./Example_rotating_primitive
        else
            echo "Build failed"
            exit 1
        fi
        ;;
esac
