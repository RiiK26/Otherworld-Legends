#include "Scanner.hpp"
#include <vector>
#include <cstdint>
#include <cstring>

namespace Scanner
{
  std::vector<int> ParsePattern(const char* pattern)
  {
    std::vector<int> bytes;
    const char*      start = pattern;
    const char*      end   = pattern + strlen(pattern);

    for (const char* current = start; current < end; ++current) {
      if (*current == '?') {
        bytes.push_back(-1);
        if (current + 1 < end && *(current + 1) == '?')
          ++current;  // Skip second ? in "??"
      }
      else if (*current != ' ') {
        bytes.push_back(strtoul(current, const_cast<char**>(&current), 16));
      }
    }
    return bytes;
  }

  uintptr_t FindPattern(HMODULE hModule, const char* pattern)
  {
    if (!hModule)
      return 0;

    auto dosHeader = (PIMAGE_DOS_HEADER) hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
      return 0;

    auto ntHeaders = (PIMAGE_NT_HEADERS) ((uint8_t*) hModule + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
      return 0;

    uintptr_t moduleBase          = (uintptr_t) hModule;
    size_t    moduleSize          = ntHeaders->OptionalHeader.SizeOfImage;

    std::vector<int> patternBytes = ParsePattern(pattern);
    if (patternBytes.empty())
      return 0;

    size_t patternSize = patternBytes.size();
    int*   pat         = patternBytes.data();

    if (moduleSize < patternSize)
      return 0;

    uint8_t* start = (uint8_t*) moduleBase;
    uint8_t* end   = start + moduleSize - patternSize;

    if (pat[0] != -1) {
      uint8_t firstByte = (uint8_t) pat[0];
      while (start <= end) {
        start = (uint8_t*) memchr(start, firstByte, end - start + 1);
        if (!start)
          return 0;

        bool found = true;
        for (size_t j = 1; j < patternSize; ++j) {
          if (pat[j] != -1 && pat[j] != start[j]) {
            found = false;
            break;
          }
        }
        if (found) {
          return (uintptr_t) start;
        }
        start++;
      }
    }
    else {
      for (uint8_t* curr = start; curr <= end; ++curr) {
        bool found = true;
        for (size_t j = 0; j < patternSize; ++j) {
          if (pat[j] != -1 && pat[j] != curr[j]) {
            found = false;
            break;
          }
        }
        if (found) {
          return (uintptr_t) curr;
        }
      }
    }
    return 0;
  }
}  // namespace Scanner
