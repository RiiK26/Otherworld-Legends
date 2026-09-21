# Project: Otherworld Legends

## Project Tech Stack
- C++20
- MinHook (Hooking)
- ImGui (Menu/UI)
- Il2CppResolver (Dynamic Unity Method/Class Resolution)
- Capstone Engine (Static Assembly Analysis & Offset Extraction)
- CMake, MinGW-w64 (Build System)

## Code Conventions
- **Modular Architecture**: 1 Feature = 1 `.hpp`/`.cpp` pair located inside `src/Features/<Domain>/` (e.g. `src/Features/Combat/GodMode.cpp`). **No god files.**
- **UI/Feature Sync**: The `src/Features/<Domain>/` directory structure MUST map exactly 1:1 with the ImGui Tab names defined in `Menu.cpp` (e.g., `Player`, `Economy`). If a feature is displayed in the "Player" tab, its source code MUST reside in `src/Features/Player/`.
- **Hooking**: Always prefer `HOOK_SIGNATURE` (AOB pattern scanning) or `HOOK_METHOD` over static `HOOK_OFFSET` where possible. Dynamic resolving and signatures are resilient to game updates and obfuscation, whereas hardcoded offsets break immediately.
- **Offset Extraction**: NEVER use `IL2CPP::Class::Utils::GetFieldOffset` in C++ for field offsets, as it breaks when games are obfuscated. Instead, rely on `aobgenerator.py` which dynamically extracts offsets via Capstone machine-code analysis. Use the generated `Offsets.hpp` variables (e.g. `Offsets::Character_minMoveSpeed`), and make sure this offsets are centralized inside the codebase, so there will be no conflict between offset variables across features.
- **Signature Database**: `config.json` at the root of the project serves as the baseline signature and extraction rule database.
- **Toggles & Config**: Store feature toggles and parameters (like damage values) in `Menu::ConfigData`. Load/save these values using `config.txt` at the project binaries directory. Avoid hardcoding magic numbers for features.
- **Initialization**: Every feature file must have an `Initialize()` and `Uninitialize()` function, which is registered in `src/Modules/Hooks/Hooks.cpp`.
- **IDE Parsing (`clangd`)**: The project uses a `.clangd` file to force-include `IL2CPP_Resolver.hpp` for internal module headers. Do NOT manually add `#include "../../IL2CPP_Resolver.hpp"` to internal headers, as it causes circular dependency "incomplete type" errors in the IDE.
