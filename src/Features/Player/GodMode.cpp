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
#include "../../Modules/Hooks/Hooks.hpp"
#include "../../Modules/Hooks/Offsets.hpp"
#include "../../Modules/Menu/Menu.hpp"
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
      *(float*) ((uintptr_t) obj + offset) = value;
    }

    // Helper: write bool at object + offset
    static inline void WriteBool(void* obj, int offset, bool value)
    {
      if (!IsValidPtr(obj))
        return;
      *(bool*) ((uintptr_t) obj + offset) = value;
    }

    // Resolve Hero singleton from IL2CPP runtime
    static void* GetHeroSingleton()
    {
      if (!s_HeroClass) {
        s_HeroClass = IL2CPP::Class::Find("Hero");
      }
      if (!s_HeroClass)
        return nullptr;

      void* staticFields = *(void**) ((uintptr_t) s_HeroClass + Offsets::Class_staticFields);
      if (!IsValidPtr(staticFields))
        return nullptr;

      return *(void**) ((uintptr_t) staticFields + 0x0);
    }

    static void WriteDictionaryFloat(void* dict, const wchar_t* targetKey, float newValue)
    {
      if (!dict)
        return;
      void* entries = *(void**) ((uintptr_t) dict + 0x18);
      if (!entries)
        return;

      int length = *(int*) ((uintptr_t) entries + 0x18);
      for (int i = 0; i < length; i++) {
        uintptr_t entryAddr = (uintptr_t) entries + 0x20 + (i * 0x18);
        void*     keyPtr    = *(void**) (entryAddr + 0x8);
        if (keyPtr) {
          int      strLen   = *(int*) ((uintptr_t) keyPtr + 0x10);
          wchar_t* strChars = (wchar_t*) ((uintptr_t) keyPtr + 0x14);

          bool match        = true;
          int  j            = 0;
          for (; j < strLen; j++) {
            if (targetKey[j] == L'\0' || strChars[j] != targetKey[j]) {
              match = false;
              break;
            }
          }
          if (match && targetKey[j] == L'\0') {
            *(float*) (entryAddr + 0x10) = newValue;
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
      if (bActiveNoCooldown) {
        void* mainCD = ReadPtr(body, Offsets::Body_mainSkillCooldown);
        void* exCD   = ReadPtr(body, Offsets::Body_exSkillCooldown);
        void* wepCD  = ReadPtr(body, Offsets::Body_weaponSkillCooldown);

        if (mainCD) {
          WriteBool(mainCD, Offsets::MultiFloat_fixedValue, true);
          WriteFloat(mainCD, Offsets::MultiFloat_fixedValue + 4, 0.0f);
        }
        if (exCD) {
          WriteBool(exCD, Offsets::MultiFloat_fixedValue, true);
          WriteFloat(exCD, Offsets::MultiFloat_fixedValue + 4, 0.0f);
        }
        if (wepCD) {
          WriteBool(wepCD, Offsets::MultiFloat_fixedValue, true);
          WriteFloat(wepCD, Offsets::MultiFloat_fixedValue + 4, 0.0f);
        }

        void* cdFlowRate = ReadPtr(body, Offsets::Body_skillCooldownFlowRate);
        if (cdFlowRate) {
          WriteBool(cdFlowRate, Offsets::MultiFloat_fixedValue, true);
          WriteFloat(cdFlowRate, Offsets::MultiFloat_fixedValue + 4, 999.0f);
        }

        s_WasNoCooldown = true;
      }
      else if (s_WasNoCooldown) {
        // Restore default dynamic cooldown flow by disabling fixedValue
        void* mainCD = ReadPtr(body, Offsets::Body_mainSkillCooldown);
        void* exCD   = ReadPtr(body, Offsets::Body_exSkillCooldown);
        void* wepCD  = ReadPtr(body, Offsets::Body_weaponSkillCooldown);

        if (mainCD)
          WriteBool(mainCD, Offsets::MultiFloat_fixedValue, false);
        if (exCD)
          WriteBool(exCD, Offsets::MultiFloat_fixedValue, false);
        if (wepCD)
          WriteBool(wepCD, Offsets::MultiFloat_fixedValue, false);

        void* cdFlowRate = ReadPtr(body, Offsets::Body_skillCooldownFlowRate);
        if (cdFlowRate)
          WriteBool(cdFlowRate, Offsets::MultiFloat_fixedValue, false);

        s_WasNoCooldown = false;
      }
    }

    void Initialize()
    {
      // No method hooks needed — we use OnTick() field patching
      // The Hero class will be resolved on first tick
    }

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
