# moca

My Own Coding Agent, yet another coding agent, a simple one with no UI actually, build solely for learning purposes.

## Requirements

- A C++23-compatible compiler (GCC or Clang)
- CMake
- Ninja
- Git
- [vcpkg](https://github.com/microsoft/vcpkg)
- [Just](https://just.systems/)

## Installation

### 1. Install system dependencies

On Arch Linux:

```bash
sudo pacman -S --needed base-devel cmake ninja git just clang
```

### 2. Install vcpkg

Clone vcpkg and bootstrap it:

```bash
git clone https://github.com/microsoft/vcpkg.git ~/.local/share/vcpkg
~/.local/share/vcpkg/bootstrap-vcpkg.sh
```

If you already have vcpkg installed elsewhere, use its existing path instead.

Set `VCPKG_ROOT` in your shell configuration.

Restart your terminal or reload the configuration.

### 3. Clone the project

```bash
git clone git@github.com:sbalghari/moca.git
cd git@github.com:sbalghari/moca.git
```

### 4. Build the project

Configure and build the debug version:

```bash
just build
```

The first build may take some time because vcpkg downloads and builds the project's C++ dependencies automatically.

## Usage

Run the application:
```bash
just run -p 'your prompt here...'
```

The project uses:

- **CMake** for build configuration
- **Ninja** as the build system
- **vcpkg** for dependency management
- **Just** for common development commands
- **clang-format** for code formatting

## Dependencies

C++ dependencies are declared in `vcpkg.json` and installed automatically during CMake configuration.

Current dependencies:

- [nlohmann/json](https://github.com/nlohmann/json) — JSON parsing and serialization
- [cpr](https://github.com/libcpr/cpr) — HTTP requests
