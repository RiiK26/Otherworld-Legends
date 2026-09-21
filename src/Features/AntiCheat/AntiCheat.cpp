/*
 * AntiCheat.cpp — Otherworld Legends
 *
 * The game has a PlayerArchive.isCheated flag at offset 0x11.
 * We suppress it by continuously setting it to false.
 */

#include "AntiCheat.hpp"
#include "../../Modules/Hooks/Hooks.hpp"
#include "../../Modules/Hooks/Offsets.hpp"
#include "../../Modules/Menu/Menu.hpp"
#include <cstdint>

namespace Features
{
  namespace AntiCheat
  {
    static void* s_PlayerArchiveClass = nullptr;

    static void* GetPlayerArchiveSingleton()
    {
      // PlayerArchive is accessed via Archive system
      // It likely has a singleton or is accessed through a manager
      // For now we resolve it via IL2CPP class system
      if (!s_PlayerArchiveClass) {
        s_PlayerArchiveClass = IL2CPP::Class::Find("Archive.PlayerArchive");
        if (!s_PlayerArchiveClass) {
          s_PlayerArchiveClass = IL2CPP::Class::Find("PlayerArchive");
        }
      }
      if (!s_PlayerArchiveClass)
        return nullptr;

      // Try to get the static instance
      void* staticFields = *(void**) ((uintptr_t) s_PlayerArchiveClass + Offsets::Class_staticFields);
      if (!staticFields)
        return nullptr;

      // PlayerArchive may not have a static singleton directly
      // We'll need to find it through the game's archive system
      // For now, return nullptr — this will be populated when we
      // discover the access pattern at runtime
      return nullptr;
    }

    void OnTick()
    {
      // Suppress isCheated flag
      void* playerArchive = GetPlayerArchiveSingleton();
      if (playerArchive) {
        *(bool*) ((uintptr_t) playerArchive + Offsets::PlayerArchive_isCheated) = false;
      }
    }

    void Initialize()
    {
      // No hooks needed — OnTick handles suppression
    }

    void Uninitialize() { }
  }  // namespace AntiCheat
}  // namespace Features
