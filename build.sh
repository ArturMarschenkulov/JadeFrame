#!/bin/bash

# #!/bin/sh
# exec python3 "$(dirname "$0")/build.py" "$@"

set -e  # Exit on error

build_dir="build"
examples_dir="examples"

# Display usage
usage() {
    echo "Usage: $0 [build|build_test|test|examples|list_examples|run <example> [args...]|clear|remove]"
    echo
    echo "Examples:"
    echo "  $0 examples              Build all examples"
    echo "  $0 list_examples         List available examples"
    echo "  $0 run hello_triangle    Build and run one example"
    exit 1
}

configure_project() {
    mkdir -p "$build_dir"
    cmake -S . -B "$build_dir" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_EXAMPLES=ON
}

list_examples() {
    for example_dir in "$examples_dir"/*; do
        if [ -d "$example_dir" ] && [ -f "$example_dir/CMakeLists.txt" ]; then
            basename "$example_dir"
        fi
    done
}

normalize_example_name() {
    local example_name="$1"
    example_name="${example_name#Example_}"
    echo "$example_name"
}

example_target() {
    echo "Example_$(normalize_example_name "$1")"
}

example_binary() {
    local example_name
    example_name="$(normalize_example_name "$1")"
    echo "$build_dir/$examples_dir/$example_name/$(example_target "$example_name")"
}

ensure_example_exists() {
    local example_name
    example_name="$(normalize_example_name "$1")"

    if [ ! -f "$examples_dir/$example_name/CMakeLists.txt" ]; then
        echo "Example not found: $example_name"
        echo
        echo "Available examples:"
        list_examples
        exit 1
    fi
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

    list_examples)
        list_examples
        ;;
    
    build)
        echo "Configuring and building project..."
        configure_project
        if cmake --build "$build_dir" --parallel; then
            cd "$build_dir"
            echo "Build successful"
        else
            echo "Build failed"
            exit 1
        fi
        ;;

    examples)
        echo "Configuring and building examples..."
        configure_project
        cmake --build "$build_dir" --parallel
        echo "Examples build successful"
        ;;

    run)
        if [ $# -lt 2 ]; then
            echo "Missing example name."
            echo
            echo "Available examples:"
            list_examples
            exit 1
        fi

        example_name="$(normalize_example_name "$2")"
        ensure_example_exists "$example_name"
        target_name="$(example_target "$example_name")"
        binary_path="$(example_binary "$example_name")"

        echo "Configuring project..."
        configure_project
        echo "Building $target_name..."
        cmake --build "$build_dir" --target "$target_name" --parallel

        if [ ! -x "$binary_path" ]; then
            echo "Example binary not found or not executable: $binary_path"
            exit 1
        fi

        echo "Running $example_name..."
        cd "$build_dir/$examples_dir/$example_name"
        shift 2
        "./$target_name" "$@"
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
        echo "Invalid option: $1"
        usage
        ;;
esac
