set dotenv-load := false

default:
    @just --list

# Configure and build the debug target
build:
    cmake --preset debug
    cmake --build --preset debug

# Build and run the debug target
run *ARGS: build
    ./build/debug/moca {{ARGS}}

# Configure and build the release target
release:
    cmake --preset release
    cmake --build --preset release

# Build and run the release target
run-release: release
    ./build/release/moca

# Format C++ source files
format:
    clang-format -i src/*.cpp

# Remove generated build files
clean:
    cmake -E rm -rf build

check-env:
    printenv OPENROUTER_API_KEY