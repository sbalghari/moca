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
cd moca
```

### 4. Build the project

Configure and build the debug version:

```bash
just build
```

The first build may take some time because vcpkg downloads and builds the project's C++ dependencies automatically.

## Usage

The build produces a `moca` binary inside `build/debug/`. Start with the help output:

```bash
./build/debug/moca --help
```

```text
Moca - A lightweight AI coding agent

Usage:
  moca [OPTIONS]

Options:
  -p, --prompt <TEXT>     Prompt for the agent to start a session with
  -m, --model <MODEL>     Select a model for the session
  -k, --api-key <KEY>     Pass an API key explicitly for this session
  -L, --list-models       List available models
      --init              Create the default config file and exit
  -h, --help              Print this help message and exit
  -v, --version           Print the app's version and exit

The API key is read from MOCA_API_KEY or from the config file. Run
`moca --init` to create the config file.
```

Common invocations:

```bash
# Create the config file (only needed once)
./build/debug/moca --init

# Ask the agent something
./build/debug/moca -p 'your prompt here...'

# Override the model and API key for a single run
./build/debug/moca -p 'your prompt here...' -m openrouter/free -k sk-or-...

# Discover the model ids accepted by -m/--model
./build/debug/moca --list-models
```

## Tools

The agent can call three tools:

- **Read** — return the contents of a file.
- **Write** — create or overwrite a file. Missing parent directories are
  created automatically.
- **Bash** — run a shell command and return its combined output (stdout and
  stderr), truncated to 8000 characters.

The model decides when to use a tool. The agent loop sends the conversation
back to the model after each tool result, so the model can chain several calls
(for example: write a file, read it back, then run a command on it). The loop
stops when the model answers without requesting a tool, or after 20 steps.

Tool calling depends on the model. If the model ignores the tools or returns an
error, try another one from `moca --list-models` (for example
`dots-studio/dots-3-note-preview:free`).

> **Warning:** the Bash tool is **not sandboxed**. It runs commands with your
> user permissions in the current directory, so only use it with prompts you
> trust.

## Configuration

Moca stores its settings in a JSON file:

- Linux/macOS: `$XDG_CONFIG_HOME/moca/config.json` (usually `~/.config/moca/config.json`)
- Windows: `%APPDATA%\Moca\config.json`

Run `moca --init` to create it. The file is created automatically on first run
as well. A default file looks like this:

```json
{
  "version": 1,
  "provider": "openrouter",
  "model": "openrouter/free",
  "api_key": ""
}
```

A value can come from several places. When more than one defines a setting, the
highest one in this list wins:

| Setting | Order |
| --- | --- |
| API key | `-k` flag, then `MOCA_API_KEY`, then `api_key` in the config file |
| Model | `-m` flag, then `model` in the config file, then the provider default |
| Provider | `provider` in the config file, then `openrouter` |
| Base URL | `base_url` in the config file, then the provider default |

Values passed with `-k` or `MOCA_API_KEY` are never written back to the config
file. On Linux/macOS the file is stored with `0600` permissions because it may
contain an API key.

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
