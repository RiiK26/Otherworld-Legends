/*
 * Offsets.hpp — Otherworld Legends (v3.6.0.3)
 *
 * Field offsets extracted via Cheat Engine mono data collector
 * from the HybridCLR-loaded GameLogic.dll assembly at runtime.
 *
 * HybridCLR Note: Game methods are IL-interpreted (not AOT-compiled),
 * so method hooking via AOB is NOT possible for game logic classes.
 * Instead, we use field-patching on per-tick hooks.
 */

#pragma once

namespace Offsets
{
  // ===== CLR Internal Structures (Unity/IL2CPP) =====
  namespace CLR
  {
    // First static field
    constexpr int FirstStaticField = 0x0;
    // System.String
    constexpr int String_length = 0x10;
    constexpr int String_chars  = 0x14;

    // System.Collections.Generic.Dictionary<TKey, TValue>
    constexpr int Dictionary_entries = 0x18;
    constexpr int Entries_length     = 0x18;
    constexpr int Entries_data       = 0x20;
    constexpr int Entry_key          = 0x8;
    constexpr int Entry_value        = 0x10;
    constexpr int Entry_stride       = 0x18;
  }  // namespace CLR

  // ===== Character (base class for Hero & Monster) =====
  // Character inherits MonoBehaviour → Component → Object (Unity)
  // MonoBehaviour base size varies, but Character fields start at 0x20+

  constexpr int Character_minMoveSpeed        = 0x24;   // System.Single
  constexpr int Character_characterName       = 0x38;   // CharacterName (enum/int32)
  constexpr int Character_camp                = 0x3C;   // System.Int32
  constexpr int Character_body                = 0x40;   // Body*
  constexpr int Character_buffManager         = 0x48;   // BuffManager*
  constexpr int Character_properties          = 0xC8;   // Ive.Properties*
  constexpr int Character_isBlockDamage       = 0x160;  // System.Boolean — blocks ALL damage
  constexpr int Character_healthLockValue     = 0x158;  // UnityEngine.Vector2 (x=min, y=max)
  constexpr int Character_uMoveSpeed          = 0x168;  // USystem*
  constexpr int Character_uStiff              = 0x170;  // USystem*
  constexpr int Character_uAntiStiff          = 0x178;  // USystem*
  constexpr int Character_uDamage             = 0x180;  // USystem*
  constexpr int Character_uCrit               = 0x188;  // USystem*
  constexpr int Character_uCritDamageRate     = 0x190;  // USystem*
  constexpr int Character_uDamageReduce       = 0x198;  // USystem*
  constexpr int Character_uAttackSpeed        = 0x1A0;  // USystem*
  constexpr int Character_uMaxHealth          = 0x1A8;  // USystem*
  constexpr int Character_uMaxShield          = 0x1B0;  // USystem*
  constexpr int Character_currentHealth       = 0x1DC;  // System.Single
  constexpr int Character_currentShield       = 0x1E0;  // System.Single
  constexpr int Character_LockHealthMultiBool = 0x2D8;  // MultiBool*
  constexpr int Character_toughness           = 0x348;  // System.Single

  // ===== Hero (player character) =====
  // Hero.singleton is a STATIC field at offset 0x0 in the class metadata
  constexpr int Hero_body                  = 0x38;   // Body*
  constexpr int Hero_heroTyrant            = 0x248;  // RefBool*
  constexpr int Hero_herolockHealth        = 0x250;  // RefBool*
  constexpr int Hero_heroNeverDieCountdown = 0x258;  // Ive.Countdown*
  constexpr int Hero_isMaxHealthGift       = 0x288;  // System.Boolean

  // ===== Body (physical entity representation) =====
  constexpr int Body_myHeroClass           = 0x48;   // Hero*
  constexpr int Body_mainWeapon            = 0xC8;   // Weapon*
  constexpr int Body_bodyName              = 0xD8;   // BodyName (enum/int32)
  constexpr int Body_isFaceRight           = 0x108;  // System.Boolean
  constexpr int Body_states                = 0x110;  // BodyStateMachine*
  constexpr int Body_character             = 0x120;  // Character*
  constexpr int Body_skillList             = 0x128;  // List<Skill>*
  constexpr int Body_invincibleCountdown   = 0x188;  // Ive.Countdown*
  constexpr int Body_isTyrants             = 0x198;  // System.Boolean
  constexpr int Body_isStiffRes            = 0x199;  // System.Boolean
  constexpr int Body_invincibleMultiBool   = 0x1D8;  // MultiBool*
  constexpr int Body_tyrantMultiBool       = 0x1E0;  // MultiBool*
  constexpr int Body_mainSkillCooldown     = 0x268;  // MultiFloat*
  constexpr int Body_exSkillCooldown       = 0x270;  // MultiFloat*
  constexpr int Body_weaponSkillCooldown   = 0x278;  // MultiFloat*
  constexpr int Body_skillCooldownFlowRate = 0x280;  // MultiFloat*

  // ===== Weapon =====
  constexpr int Weapon_damage = 0x78;  // System.Single

  // ===== DamageMessage =====
  constexpr int DamageMessage_isCrit      = 0x1F;  // System.Boolean
  constexpr int DamageMessage_damage      = 0x24;  // System.Single
  constexpr int DamageMessage_finalDamage = 0x28;  // System.Single
  constexpr int DamageMessage_stiff       = 0x44;  // System.Single

  // ===== HeroRunningData =====
  constexpr int HRD_currentHealth         = 0x88;   // System.Single
  constexpr int HRD_currentShield         = 0x8C;   // System.Single
  constexpr int HRD_coinPickUp            = 0x168;  // System.Int32
  constexpr int HRD_currentHPBottleNumber = 0x294;  // System.Int32

  // ===== PlayerArchive (persistent currencies — EncryptValue wrapped) =====
  constexpr int PlayerArchive_isCheated   = 0x11;  // System.Boolean — anti-cheat flag!
  constexpr int PlayerArchive_exSoulStone = 0x18;  // EncryptValue (Soul Stones)
  constexpr int PlayerArchive_exBlueStone = 0x20;  // EncryptValue (Sapphires)
  constexpr int PlayerArchive_exRedStone  = 0x28;  // EncryptValue (Rubies)

  // ===== Ive.Countdown =====
  constexpr int Countdown_interval = 0x18;  // System.Single
  constexpr int Countdown_isFreeze = 0x1C;  // System.Boolean
  constexpr int Countdown_isPause  = 0x24;  // System.Boolean

  // ===== Ive.Properties =====
  constexpr int Properties_baseProperty = 0x10;  // Dictionary<string, float>*

  // ===== MultiFloat =====
  constexpr int MultiFloat_minValue    = 0x10;  // System.Single
  constexpr int MultiFloat_fixedValue  = 0x14;  // Nullable<float> (hasValue at +0x14, value at +0x18)
  constexpr int MultiFloat_handlerList = 0x20;  // List<ReferenceFloat>*

  // ===== MultiBool =====
  constexpr int MultiBool_value       = 0x10;  // System.Boolean
  constexpr int MultiBool_handlerList = 0x18;  // List<RefBool>*

  // ===== EncryptValue =====
  constexpr int EncryptValue_value     = 0x10;  // System.Int32
  constexpr int EncryptValue_key       = 0x14;  // System.Int32
  constexpr int EncryptValue_encrypted = 0x18;  // System.Int32

  // ===== IL2CPP Core =====
  constexpr int Class_staticFields = 0xB8;  // void* (pointer to static fields)

  // ===== GameProcess (singleton — holds HeroRunningData) =====
  // GameProcess.singleton is a static field
  constexpr int GameProcess_heroRunningData = 0x08;  // HeroRunningData* (static offset within class)

  // ===== Monster =====
  constexpr int Monster_body = 0x30;  // Body*

}  // namespace Offsets
