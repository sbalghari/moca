set dotenv-load := false

default:
    @just --list

# Configure and build the debug target
build:
    cmake --preset debug
    cmake --build --preset debug

# Configure and build the release target
release:
    cmake --preset release
    cmake --build --preset release

# Format C++ source files
format:
    find src -type f \( -name '*.cpp' -o -name '*.hpp' \) -exec clang-format -i {} +

# Remove generated build files
clean:
    cmake -E rm -rf build
