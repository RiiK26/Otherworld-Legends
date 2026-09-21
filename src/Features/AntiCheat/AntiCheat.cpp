/*
 * AntiCheat.cpp — Otherworld Legends
 *
 * The game has a PlayerArchive.isCheated flag at offset 0x11.
 * We suppress it by continuously setting it to false.
 */

#include "AntiCheat.hpp"
#include "../../Modules/Hooks/Offsets.hpp"
#include "../../Modules/Il2CppResolver/IL2CPP_Resolver.hpp"  // IWYU pragma: keep
#include <cstdint>

namespace Features
{
  namespace AntiCheat
  {
    static void* s_PlayerArchiveClass = nullptr;

    static void* GetPlayerArchiveSingleton()
    {
      if (!s_PlayerArchiveClass)
        return nullptr;

      // Try to get the static instance
      void* staticFields = *(void**) ((uintptr_t) s_PlayerArchiveClass + Offsets::Class_staticFields);
      if (!staticFields)
        return nullptr;

      return nullptr;
    }

    void OnTick()
    {
      // Suppress isCheated flag
      void* playerArchive = GetPlayerArchiveSingleton();
      if (playerArchive) {
        bool* ptr = (bool*) ((uintptr_t) playerArchive + Offsets::PlayerArchive_isCheated);
        if (*ptr != false) {
          *ptr = false;
        }
      }
    }

    void Initialize()
    {
      s_PlayerArchiveClass = IL2CPP::Class::Find("Archive.PlayerArchive");
      if (!s_PlayerArchiveClass) {
        s_PlayerArchiveClass = IL2CPP::Class::Find("PlayerArchive");
      }
    }

    void Uninitialize() { }
  }  // namespace AntiCheat
}  // namespace Features
