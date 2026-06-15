#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parent
BUILD_DIR = ROOT_DIR / "build"
EXAMPLES_DIR = ROOT_DIR / "examples"


def run_command(args: list[str], *, cwd: Path = ROOT_DIR) -> None:
    subprocess.run(args, cwd=cwd, check=True)


def configure_project() -> None:
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    run_command(
        [
            "cmake",
            "-S",
            ".",
            "-B",
            str(BUILD_DIR),
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
            "-DBUILD_EXAMPLES=ON",
        ]
    )


def list_examples() -> list[str]:
    if not EXAMPLES_DIR.is_dir():
        return []

    return sorted(
        example_dir.name
        for example_dir in EXAMPLES_DIR.iterdir()
        if example_dir.is_dir() and (example_dir / "CMakeLists.txt").is_file()
    )


def print_examples() -> None:
    for example_name in list_examples():
        print(example_name)


def normalize_example_name(example_name: str) -> str:
    return example_name.removeprefix("Example_")


def example_target(example_name: str) -> str:
    return f"Example_{normalize_example_name(example_name)}"


def example_binary(example_name: str) -> Path:
    normalized_name = normalize_example_name(example_name)
    executable_name = example_target(normalized_name)

    if os.name == "nt":
        executable_name += ".exe"

    return BUILD_DIR / "examples" / normalized_name / executable_name


def ensure_example_exists(example_name: str) -> None:
    normalized_name = normalize_example_name(example_name)

    if not (EXAMPLES_DIR / normalized_name / "CMakeLists.txt").is_file():
        print(f"Example not found: {normalized_name}", file=sys.stderr)
        print(file=sys.stderr)
        print("Available examples:", file=sys.stderr)
        for available_example in list_examples():
            print(available_example, file=sys.stderr)
        raise SystemExit(1)


def command_remove(_args: argparse.Namespace) -> None:
    if BUILD_DIR.is_dir():
        print("Removing build directory...")
        shutil.rmtree(BUILD_DIR)
    else:
        print("No build directory to remove.")


def command_clear(_args: argparse.Namespace) -> None:
    if not BUILD_DIR.is_dir():
        print("Build directory not found.")
        return

    cmake_cache = BUILD_DIR / "CMakeCache.txt"
    if cmake_cache.is_file():
        print("Removing CMake cache...")
        cmake_cache.unlink()
    else:
        print("CMake cache not found.")


def command_test(_args: argparse.Namespace) -> None:
    if not BUILD_DIR.is_dir():
        print("Build directory not found. Run build first.", file=sys.stderr)
        raise SystemExit(1)

    run_command(["ctest", "--verbose"], cwd=BUILD_DIR)


def command_build(_args: argparse.Namespace) -> None:
    print("Configuring and building project...")
    configure_project()
    run_command(["cmake", "--build", str(BUILD_DIR), "--parallel"])
    print("Build successful")


def command_examples(_args: argparse.Namespace) -> None:
    print("Configuring and building examples...")
    configure_project()
    run_command(["cmake", "--build", str(BUILD_DIR), "--parallel"])
    print("Examples build successful")


def command_run(args: argparse.Namespace) -> None:
    if args.example is None:
        print("Missing example name.", file=sys.stderr)
        print(file=sys.stderr)
        print("Available examples:", file=sys.stderr)
        for available_example in list_examples():
            print(available_example, file=sys.stderr)
        raise SystemExit(1)

    example_name = normalize_example_name(args.example)
    ensure_example_exists(example_name)

    target_name = example_target(example_name)
    binary_path = example_binary(example_name)

    print("Configuring project...")
    configure_project()
    print(f"Building {target_name}...")
    run_command(["cmake", "--build", str(BUILD_DIR), "--target", target_name, "--parallel"])

    if not binary_path.is_file():
        print(f"Example binary not found: {binary_path}", file=sys.stderr)
        raise SystemExit(1)

    print(f"Running {example_name}...")
    run_command([str(binary_path), *args.example_args], cwd=binary_path.parent)


def command_build_test(_args: argparse.Namespace) -> None:
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    print("Configuring and building test target...")
    run_command(["cmake", "../."], cwd=BUILD_DIR)
    run_command(["cmake", "--build", ".", "--target", "test", "-j10"], cwd=BUILD_DIR)
    print("Tests build successful")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Build, test, and run JadeFrame examples.",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    remove_parser = subparsers.add_parser("remove", help="Remove the build directory.")
    remove_parser.set_defaults(func=command_remove)

    clear_parser = subparsers.add_parser("clear", help="Remove build/CMakeCache.txt.")
    clear_parser.set_defaults(func=command_clear)

    test_parser = subparsers.add_parser("test", help="Run ctest from the build directory.")
    test_parser.set_defaults(func=command_test)

    build_parser = subparsers.add_parser("build", help="Configure and build the project.")
    build_parser.set_defaults(func=command_build)

    build_test_parser = subparsers.add_parser("build_test", help="Build the test target.")
    build_test_parser.set_defaults(func=command_build_test)

    examples_parser = subparsers.add_parser("examples", help="Build all examples.")
    examples_parser.set_defaults(func=command_examples)

    list_examples_parser = subparsers.add_parser("list_examples", help="List available examples.")
    list_examples_parser.set_defaults(func=lambda _args: print_examples())

    run_parser = subparsers.add_parser("run", help="Build and run one example.")
    run_parser.add_argument("example", nargs="?", help="Example name, with or without the Example_ prefix.")
    run_parser.add_argument("example_args", nargs=argparse.REMAINDER, help="Arguments passed to the example.")
    run_parser.set_defaults(func=command_run)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        args.func(args)
    except subprocess.CalledProcessError as error:
        return error.returncode

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
