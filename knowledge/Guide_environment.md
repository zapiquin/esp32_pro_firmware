
# ESP32-S3 Development Environment Guide
## Ubuntu + Docker + Dev Containers + ESP-IDF + C++

## 1. Objective

This guide explains how to set up a reproducible firmware development environment for the **ESP32-S3** using:

- **Ubuntu** as the host OS
- **Docker** to isolate the toolchain
- **VS Code + Dev Containers** to work inside the container
- **ESP-IDF** as the official framework
- **C++** for firmware development
- **Clangd / clang-format / clang-tidy** for code quality
- **Poetry** for auxiliary Python tools on the host machine

The goal is to make it possible for anyone to **clone the repository and start working with exactly the same environment**, without installing Espressif’s SDK or toolchains directly on the host system.

## 2. Solution Philosophy

The Ubuntu host acts only as the **host machine**.
Everything related to compilation, toolchains, and ESP-IDF runs inside the container.

### Benefits

- Reproducible environment across different machines
- Fewer dependency issues
- Easy to pin a specific framework version
- Ready to scale into CI/CD later
- Fast onboarding for other developers

## 3. Preparing the Ubuntu Host
Install only the base tools required to work with the repository, containers, and auxiliary scripts.
```bash
sudo apt-get update
sudo apt-get install -y git docker.io curl python3-pip
```
### Install Poetry
Install only the base tools required to work with the repository, containers, and auxiliary scripts.

```bash
curl -sSL https://install.python-poetry.org | python3 -
```
### Docker and serial port permissions
Add your user to the apropiate groups to allow running Docker and accessing serial ports without `sudo`:
```bash
sudo usermod -aG docker $USER
sudo usermod -aG dialout $USER
```
> **Important:** after this, you must log out and back in again (or reboot) for the group changes to take effect.
### Visual Studio Code
Install:
- **VS Code** from https://code.visualstudio.com/.
- Microsoft's **Dev Containers** extension from the VS Code marketplace.

## 4. Creating the repository
Create the project folder and initialize Git:
```bash
mkdir esp32_pro_firmware
cd esp32_pro_firmware
git init
```

## 5. Dev Container Configuration
In the repository root, create the `.devcontainer` folder:
```bash
mkdir .devcontainer
touch .devcontainer/devcontainer.json
```
### devcontainer.json
Edit `.devcontainer/devcontainer.json` with the following content:
```json
{
    "name": "ESP32-S3 Professional Env",
    // Official Espressif Docker image containing the full ESP-IDF framework
    "image": "espressif/idf:release-v5.2",
    // Privileges to allow the container to access host hardware (USB ports for flashing)
    "runArgs": [
        "--privileged",
        "-v",
        "/dev:/dev"
    ],
    "postCreateCommand": "echo 'source /opt/esp/idf/export.sh' >> ~/.bashrc",
    "customizations": {
        "vscode": {
            // VS Code extensions that will be installed ONLY inside this container
            "extensions": [
                "ms-vscode.cpptools", // Core C/C++ debugging tools
                "llvm-vs-code-extensions.vscode-clangd", // Advanced IntelliSense and Formatter
                "twxs.cmake" // CMake syntax highlighting
            ],
            // Workspace-specific settings applied automatically
            "settings": {
                // Enable auto-formatting every time you save a file (Ctrl + S)
                "editor.formatOnSave": true,
                // Disable Microsoft's default IntelliSense to prevent conflicts with Clangd
                "C_Cpp.intelliSenseEngine": "disabled",
                // Clangd configuration to parse ESP-IDF components correctly
                "clangd.arguments": [
                    "--background-index",
                    "--compile-commands-dir=${workspaceFolder}/build",
                    "--header-insertion=iwyu"
                ],
                // Clean code practices: ensure trailing newline and remove extra whitespaces
                "files.insertFinalNewline": true,
                "files.trimTrailingWhitespace": true,
                // Force VS Code to use Clangd as the default formatter for C and C++ files
                // This ensures it reads your .clang-format file perfectly
                "[cpp]": {
                    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
                    "editor.formatOnSave": true
                },
                "[c]": {
                    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
                    "editor.formatOnSave": true
                }
            }
        }
    }
}
```
#### What this configuration does:
- Uses the official Espressif Docker image with ESP-IDF already installed.
- Grants access to USB/serial devices so the board can be flashed.
- Installs C/C++ extensions inside the container.
- Uses `clangd` as the main analysis and autocomplete engine.
- Automatically formats code on save using `clang-format`.
