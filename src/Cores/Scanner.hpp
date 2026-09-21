#pragma once
#include <windows.h>

namespace Scanner
{
  // Scans a module in memory for a byte pattern.
  // The pattern should be formatted like "48 8B C4 ? ? 58" or "48 8b c4 ?? ?? 58".
  // Returns the absolute memory address of the match, or 0 if not found.
  uintptr_t FindPattern(HMODULE hModule, const char* pattern);
}  // namespace Scanner
