
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
- Espressif documents that the espressif/idf image contains ESP-IDF, the required build tools, and an entrypoint that configures the environment for immediate use. Tags such as release-vX.Y track the corresponding release branch.

## 6. Opening the Project in the Container
1. Open VS Code in the project folder (`esp32_pro_firmware`).
2. Press `F1` and select **Remote-Containers: Open Folder in Container**.
3. Wait for the image to be downloaded and the environment start.
From this point on, the integrated terminal in VS Code will be running **inside the container**.

## 7. Creating the Base ESP-IDF Project
Inside the container terminal (in VS Code), run the following commands to create a new ESP-IDF project skeleton :
```bash
idf.py create-project app
```
> **NOTE:** The `idf.py` create-project <project_name> command is documented by Espressif as the standard way to start a new ESP-IDF project.

Then move the generated files to the root of the repository:
```bash
mv app/* .
rm -rf app
```
This leaves the ESP-IDF project directly at the repository root.

## 8. Converting the project to C++
Rename the main source file from `app.c` to `app.cpp` to enable C++ compilation:
```bash
mv main/app.c main/main.cpp
```
Then, edit `main/CMakeLists.txt` so it points to the C++ source file:
```cmake
idf_component_register(SRCS "app.cpp"
                    INCLUDE_DIRS ".")
```
ESP-IDF officially supports C++ application development. Current documentation states that ESP-IDF compiles C++ code using GNU++23 by default, so no additional configuration is needed to enable C++17 or later features.

## 9. Code Style and Formatting Rules
**`clang-format`**
Create a `.clang-format` file in the repository root with the following content:
```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
SortIncludes: true
AllowShortFunctionsOnASingleLine: Empty
PointerAlignment: Left
```

**`.clang-tidy`**
Create a `.clang-tidy` file in the repository root with the following content:
```yaml
Checks: '-*,readability-identifier-naming,modernize-*,bugprone-*'
CheckOptions:
  - key:             readability-identifier-naming.VariableCase
    value:           camelBack
  - key:             readability-identifier-naming.ClassCase
    value:           CamelCase
  - key:             readability-identifier-naming.StructCase
    value:           CamelCase
  - key:             readability-identifier-naming.MacroDefinitionCase
    value:           UPPER_CASE
```
This gives you a solid baseline for naming consistency and static analysis.

## 10. Firmware Dependencies (C++)
For firmware libraries, the recommended approach is to use the **ESP Component Registry**
Example:
```bash
idf.py add-dependency "espressif/json_parser^1.0.0"
```
Espressif documents that `idf.py add-dependency` adds the dependency to the project manifest for the component, typically `main/idf_component.yml`, and that dependency resolution is handled automatically during CMake configuration/build.
