/*
 * Signatures.hpp — Otherworld Legends (v3.6.0.3)
 *
 * HybridCLR Note: The game uses HybridCLR hot-update, so most game logic
 * methods (Character, Hero, Body, etc.) are IL-interpreted and do NOT have
 * native AOT-compiled implementations in GameAssembly.dll.
 *
 * Only Unity engine methods and IL2CPP runtime methods have native code
 * that can be hooked via AOB signatures.
 *
 * Strategy: We hook Unity engine functions (like Time.set_timeScale for
 * speed hack) and use the DX11 Present hook as our per-frame tick source
 * for field-patching game objects.
 *

#pragma once

namespace Signatures
{
  // Unity Time.set_timeScale — used for speed hack
  // This is an internal Unity engine method (always AOT in GameAssembly.dll)
  constexpr const char* Time_set_timeScale =
    "48 83 EC 38 48 8B 05 ? ? ? ? 0F 29 74 24 20 0F 28 F0 48 85 C0 75 13 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? "
    "0F 28 C6 0F 28 74 24 20 48 83 C4 38 48 FF E0";

}  // namespace Signatures
*/
