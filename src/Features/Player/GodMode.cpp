/*
 * GodMode.cpp — Otherworld Legends
 *
 * HybridCLR Strategy: Since game methods are IL-interpreted (not AOT-compiled),
 * we use a field-patching approach. On each frame (called from Present hook):
 *   1. Resolve Hero.singleton via IL2CPP runtime API
 *   2. Walk to Body → Character
 *   3. Patch _currentHealth, _isBlockDamage, etc.
 *
 * Features controlled by Menu::Config toggles:
 *   - Infinite HP:     Continuously set _currentHealth to max
 *   - Block Damage:    Set _isBlockDamage = true
 *   - High Damage:     Write large value to uDamage USystem base
 *   - Fast Move Speed: Multiply Character.uMoveSpeed base
 *   - Always Crit:     Set uCrit base to 100%
 *   - Fast Attack:     Set uAttackSpeed base high
 *   - No Cooldown:     Freeze cooldown timers on Body
 */

#include "GodMode.hpp"
#include "../../Modules/Hooks/Offsets.hpp"
#include "../../Modules/Menu/Menu.hpp"
#include "../../Modules/Il2CppResolver/IL2CPP_Resolver.hpp"  // IWYU pragma: keep
#include <cstdint>

namespace Features
{
  namespace GodMode
  {
    // Cached pointers — resolved once, validated each tick
    static void* s_HeroClass        = nullptr;
    static void* s_GameProcessClass = nullptr;

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

    // Helper: read a pointer at object + offset
    static inline void* ReadPtr(void* obj, int offset)
    {
      if (!IsValidPtr(obj))
        return nullptr;
      return *(void**) ((uintptr_t) obj + offset);
    }

    // Helper: read float at object + offset
    static inline float ReadFloat(void* obj, int offset)
    {
      if (!IsValidPtr(obj))
        return 0.0f;
      return *(float*) ((uintptr_t) obj + offset);
    }

    // Helper: write float at object + offset
    static inline void WriteFloat(void* obj, int offset, float value)
    {
      if (!IsValidPtr(obj))
        return;
      float* ptr = (float*) ((uintptr_t) obj + offset);
      if (*ptr != value) {
        *ptr = value;
      }
    }

    // Helper: write bool at object + offset
    static inline void WriteBool(void* obj, int offset, bool value)
    {
      if (!IsValidPtr(obj))
        return;
      bool* ptr = (bool*) ((uintptr_t) obj + offset);
      if (*ptr != value) {
        *ptr = value;
      }
    }

    // Resolve Hero singleton from IL2CPP runtime
    static void* GetHeroSingleton()
    {
      if (!s_HeroClass)
        return nullptr;

      void* staticFields = *(void**) ((uintptr_t) s_HeroClass + Offsets::Class_staticFields);
      if (!IsValidPtr(staticFields))
        return nullptr;

      return *(void**) ((uintptr_t) staticFields + Offsets::CLR::FirstStaticField);  // 0x0
    }

    static void WriteDictionaryFloat(void* dict, const wchar_t* targetKey, float newValue)
    {
      if (!dict)
        return;
      void* entries = *(void**) ((uintptr_t) dict + Offsets::CLR::Dictionary_entries);
      if (!entries)
        return;

      // Fast-path: if the entries array hasn't been reallocated, use cached pointer
      static void*  s_lastEntries  = nullptr;
      static float* s_cachedValPtr = nullptr;
      if (entries == s_lastEntries && IsValidPtr(s_cachedValPtr)) {
        if (*s_cachedValPtr != newValue) {
          *s_cachedValPtr = newValue;
        }
        return;
      }

      int length = *(int*) ((uintptr_t) entries + Offsets::CLR::Entries_length);
      for (int i = 0; i < length; i++) {
        uintptr_t entryAddr = (uintptr_t) entries + Offsets::CLR::Entries_data + (i * Offsets::CLR::Entry_stride);
        void*     keyPtr    = *(void**) (entryAddr + Offsets::CLR::Entry_key);
        if (keyPtr) {
          int      strLen   = *(int*) ((uintptr_t) keyPtr + Offsets::CLR::String_length);
          wchar_t* strChars = (wchar_t*) ((uintptr_t) keyPtr + Offsets::CLR::String_chars);

          bool match        = true;
          int  j            = 0;
          for (; j < strLen; j++) {
            if (targetKey[j] == L'\0' || strChars[j] != targetKey[j]) {
              match = false;
              break;
            }
          }
          if (match && targetKey[j] == L'\0') {
            float* valPtr = (float*) (entryAddr + Offsets::CLR::Entry_value);

            // Cache the result for future frames
            s_lastEntries  = entries;
            s_cachedValPtr = valPtr;

            if (*valPtr != newValue) {
              *valPtr = newValue;
            }
            return;
          }
        }
      }
    }

    void OnTick()
    {
      void* hero = GetHeroSingleton();
      if (!IsValidPtr(hero))
        return;

      // Hero → Body
      void* body = ReadPtr(hero, Offsets::Hero_body);
      if (!IsValidPtr(body))
        return;

      // Body → Character
      void* character = ReadPtr(body, Offsets::Body_character);
      if (!IsValidPtr(character))
        return;

      static bool s_WasInfiniteHP = false;
      static bool s_WasSpeed      = false;
      static bool s_WasNoCooldown = false;

      bool bActiveInfiniteHP      = Menu::Config.bGodMode && Menu::Config.bGodMode_InfiniteHP;
      bool bActiveSpeed           = Menu::Config.bGodMode && Menu::Config.bGodMode_Speed;
      bool bActiveNoCooldown      = Menu::Config.bGodMode && Menu::Config.bNoCooldown;

      // === Infinite HP / Invincible ===
      if (bActiveInfiniteHP) {
        WriteFloat(character, Offsets::Character_currentHealth, 99999.0f);
        WriteFloat(character, Offsets::Character_currentShield, 99999.0f);
        WriteBool(character, Offsets::Character_isBlockDamage, true);
        WriteBool(body, Offsets::Body_isTyrants, true);
        WriteBool(body, Offsets::Body_isStiffRes, true);
        s_WasInfiniteHP = true;
      }
      else if (s_WasInfiniteHP) {
        // Restore defensive flags when disabled
        WriteBool(character, Offsets::Character_isBlockDamage, false);
        WriteBool(body, Offsets::Body_isTyrants, false);
        WriteBool(body, Offsets::Body_isStiffRes, false);
        s_WasInfiniteHP = false;
      }

      // === High Damage ===
      if (Menu::Config.bGodMode && Menu::Config.bGodMode_Damage) {
        // Also modify dictionary to reflect visually in UI
        void* properties = ReadPtr(character, Offsets::Character_properties);
        if (properties) {
          void* baseProp = ReadPtr(properties, Offsets::Properties_baseProperty);
          if (baseProp) {
            WriteDictionaryFloat(baseProp, L"Damage", Menu::Config.fGodModeDamage);
          }
        }

        // Write actual damage to active weapon
        void* mainWeapon = ReadPtr(body, Offsets::Body_mainWeapon);
        if (mainWeapon) {
          WriteFloat(mainWeapon, Offsets::Weapon_damage, Menu::Config.fGodModeDamage);
        }
      }

      // === Fast Move Speed ===
      if (bActiveSpeed) {
        WriteFloat(character, Offsets::Character_minMoveSpeed, Menu::Config.fGodModeSpeedMultiplier);
        s_WasSpeed = true;
      }
      else if (s_WasSpeed) {
        // Reset to default base min speed (typically 4.0f)
        WriteFloat(character, Offsets::Character_minMoveSpeed, 4.0f);
        s_WasSpeed = false;
      }

      // === No Cooldown ===
      auto SetCooldownFixed = [](void* cdObj, bool fixed, float val = 0.0f) {
        if (cdObj) {
          WriteBool(cdObj, Offsets::MultiFloat_fixedValue, fixed);
          if (fixed)
            WriteFloat(cdObj, Offsets::MultiFloat_fixedValue + 4, val);
        }
      };

      if (bActiveNoCooldown) {
        SetCooldownFixed(ReadPtr(body, Offsets::Body_mainSkillCooldown), true, 0.0f);
        SetCooldownFixed(ReadPtr(body, Offsets::Body_exSkillCooldown), true, 0.0f);
        SetCooldownFixed(ReadPtr(body, Offsets::Body_weaponSkillCooldown), true, 0.0f);
        SetCooldownFixed(ReadPtr(body, Offsets::Body_skillCooldownFlowRate), true, 999.0f);
        s_WasNoCooldown = true;
      }
      else if (s_WasNoCooldown) {
        SetCooldownFixed(ReadPtr(body, Offsets::Body_mainSkillCooldown), false);
        SetCooldownFixed(ReadPtr(body, Offsets::Body_exSkillCooldown), false);
        SetCooldownFixed(ReadPtr(body, Offsets::Body_weaponSkillCooldown), false);
        SetCooldownFixed(ReadPtr(body, Offsets::Body_skillCooldownFlowRate), false);
        s_WasNoCooldown = false;
      }
    }

    void Initialize() { s_HeroClass = IL2CPP::Class::Find("Hero"); }

    void Uninitialize()
    {
      // Restore _isBlockDamage on cleanup
      void* hero = GetHeroSingleton();
      if (hero) {
        void* body = ReadPtr(hero, Offsets::Hero_body);
        if (body) {
          void* character = ReadPtr(body, Offsets::Body_character);
          if (character) {
            WriteBool(character, Offsets::Character_isBlockDamage, false);
            WriteBool(body, Offsets::Body_isTyrants, false);
            WriteBool(body, Offsets::Body_isStiffRes, false);
          }
        }
      }
    }
  }  // namespace GodMode
}  // namespace Features
