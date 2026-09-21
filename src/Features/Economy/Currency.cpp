/*
 * Currency.cpp — Otherworld Legends
 *
 * Currency manipulation for Soul Stones, Sapphires (Blue Stones), and Rubies (Red Stones).
 *
 * The currencies are stored in PlayerArchive using EncryptValue wrapper.
 * EncryptValue is a custom obfuscation class — likely XOR or similar.
 * Until we reverse EncryptValue's internal layout, this feature is a stub
 * that will be populated once we discover the encryption key at runtime.
 *
 * Alternative approach: Hook the currency getter/setter methods if they
 * have native AOT implementations.
 */

#include "Currency.hpp"
#include "../../Modules/Hooks/Hooks.hpp"
#include "../../Modules/Hooks/Offsets.hpp"
#include "../../Modules/Menu/Menu.hpp"
#include "../../Modules/Il2CppResolver/IL2CPP_Resolver.hpp"
#include <windows.h>
#include <cstdint>
#include <fstream>
#include <vector>
namespace Features
{
  namespace Currency
  {
    static void* s_PlayerArchive    = nullptr;
    static void* s_GameProcessClass = nullptr;
    static void LogDebug(const char* msg)
    {
      std::ofstream logFile("Z:\\tmp\\cheat_log.txt", std::ios_base::app);
      if (logFile.is_open()) {
        logFile << msg << "\n";
      }
    }

    // Helper: Safely check if a pointer is readable
    static bool IsValidPtr(void* p)
    {
      if (!p || (uintptr_t) p < 0x10000)
        return false;

      MEMORY_BASIC_INFORMATION mbi;

      if (VirtualQuery(p, &mbi, sizeof(mbi))) {
        return mbi.State == MEM_COMMIT
            && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE || mbi.Protect == PAGE_READONLY);
      }
      return false;
    }

    // Helper: Get EncryptValue fields
    static void SetEncryptValue(void* encryptObj, int32_t newValue)
    {
      if (!encryptObj)
        return;

      int32_t key = *(int32_t*) ((uintptr_t) encryptObj + Offsets::EncryptValue_key);
      *(int32_t*) ((uintptr_t) encryptObj + Offsets::EncryptValue_value)     = newValue;        // value
      *(int32_t*) ((uintptr_t) encryptObj + Offsets::EncryptValue_encrypted) = newValue ^ key;  // encrypted
    }

    static bool IsValidEncryptValue(void* ev, void* expectedClass)
    {
      if (!IsValidPtr(ev))
        return false;
      if (*(void**) ev != expectedClass)
        return false;

      int32_t val = *(int32_t*) ((uintptr_t) ev + Offsets::EncryptValue_value);
      int32_t key = *(int32_t*) ((uintptr_t) ev + Offsets::EncryptValue_key);
      int32_t enc = *(int32_t*) ((uintptr_t) ev + Offsets::EncryptValue_encrypted);
      // Key can be 0, but XOR must always match
      return val == (enc ^ key);
    }

    static void* s_PlayerArchiveClass = nullptr;
    static void* s_EncryptValueClass  = nullptr;

    struct MemRegion
    {
      uintptr_t start;
      uintptr_t end;
    };

    static bool IsPtrInRegions(const std::vector<MemRegion>& regions, uintptr_t ptr)
    {
      if (ptr < 0x10000)
        return false;
      // Simple linear search is fast enough for ~2000 regions
      for (const auto& r : regions) {
        if (ptr >= r.start && ptr < r.end)
          return true;
      }
      return false;
    }

    void ApplyInfiniteLobbyCurrency()
    {
      LogDebug("ApplyInfiniteLobbyCurrency clicked.");
      if (!s_EncryptValueClass) {
        s_EncryptValueClass = IL2CPP::Class::Find("EncryptValue");
      }

      if (!s_EncryptValueClass) {
        LogDebug("Failed to find EncryptValue class. Aborting scan.");
        return;
      }

      SYSTEM_INFO sysInfo;
      GetSystemInfo(&sysInfo);
      uint8_t*               addr    = (uint8_t*) sysInfo.lpMinimumApplicationAddress;
      uint8_t*               maxAddr = (uint8_t*) 0x7FFFFFFFFFFF;
      std::vector<MemRegion> regions;
      regions.reserve(4096);

      // Step 1: Collect all readable/writable memory regions (Fast)
      while (addr < maxAddr) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(addr, &mbi, sizeof(mbi))) {
          addr += 0x1000;
          continue;
        }

        if (mbi.State == MEM_COMMIT && (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)) {
          regions.push_back({(uintptr_t) mbi.BaseAddress, (uintptr_t) mbi.BaseAddress + mbi.RegionSize});
        }

        addr = (uint8_t*) mbi.BaseAddress + mbi.RegionSize;
      }

      // Step 2: Scan backwards from the highest memory region to find the NEWEST allocation!
      int patchCount = 0;

      for (auto it = regions.rbegin(); it != regions.rend(); ++it) {
        uintptr_t regionStart = it->start;
        uintptr_t regionEnd   = it->end;

        if (regionEnd - regionStart < 0x40)
          continue;

        // Scan backwards within the region
        for (uintptr_t p = regionEnd - 0x30; p >= regionStart; p -= 8) {
          void* p18 = *(void**) (p + Offsets::PlayerArchive_exSoulStone);
          void* p20 = *(void**) (p + Offsets::PlayerArchive_exBlueStone);
          void* p28 = *(void**) (p + Offsets::PlayerArchive_exRedStone);

          // Fast pointer validity check (no VirtualQuery syscalls!)
          if (
            IsPtrInRegions(regions, (uintptr_t) p18) && IsPtrInRegions(regions, (uintptr_t) p20)
            && IsPtrInRegions(regions, (uintptr_t) p28)
          ) {
            void* class1 = *(void**) p18;

            if (class1 == s_EncryptValueClass) {
              if (
                IsValidEncryptValue(p18, class1) && IsValidEncryptValue(p20, class1) && IsValidEncryptValue(p28, class1)
              ) {
                // Found a true PlayerArchive instance!
                *(bool*) ((uintptr_t) p + Offsets::PlayerArchive_isCheated) = false;

                // Dynamically scan the PlayerArchive object to find ALL currencies!
                // We'll scan up to offset 0x100 (should cover all 13+ currency fields).
                int fieldsPatched = 0;
                for (int offset = Offsets::PlayerArchive_exSoulStone; offset < 0x100; offset += 8) {
                  if (p + offset >= regionEnd)
                    break;

                  void* fieldPtr = *(void**) (p + offset);

                  if (IsPtrInRegions(regions, (uintptr_t) fieldPtr)) {
                    if (*(void**) fieldPtr == s_EncryptValueClass) {
                      if (IsValidEncryptValue(fieldPtr, s_EncryptValueClass)) {
                        SetEncryptValue(fieldPtr, 99999);
                        fieldsPatched++;
                      }
                    }
                  }
                }

                patchCount++;
              }
            }
          }
        }
      }

      char buf[256];
      snprintf(buf, sizeof(buf), "Heap scan finished. Patched %d true PlayerArchive instances.", patchCount);
      LogDebug(buf);
    }

    void OnTick()
    {
      // No continuous ticking needed for Lobby Currency if we use a button
    }

    void Initialize() { }
    void Uninitialize() { }
  }  // namespace Currency
}  // namespace Features
