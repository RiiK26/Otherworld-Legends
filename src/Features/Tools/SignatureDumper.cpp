#include "SignatureDumper.hpp"
#include "../../Modules/Il2CppResolver/IL2CPP_Resolver.hpp"  // IWYU pragma: keep
#include "../../Modules/Hooks/Offsets.hpp"
#include <fstream>
#include <string>
#include <vector>

namespace Features
{
  namespace SignatureDumper
  {
    struct SigRule
    {
      std::string name;
      std::string className;
      std::string typeName;
      int         index;
    };

    static void ProcessField(const char* name, const char* className, int targetOffset, std::vector<SigRule>& rules)
    {
      auto m_pClass = IL2CPP::Class::Find(className);
      if (!m_pClass)
        return;

      void*       iter           = nullptr;
      std::string targetTypeName = "";

      // First pass: find the exact type of the field at this offset
      while (auto field = IL2CPP::Class::GetFields(m_pClass, &iter)) {
        if (field->m_iOffset == targetOffset) {
          auto typeClass = IL2CPP::Class::Utils::ClassFromType(field->m_pType);
          if (typeClass && typeClass->m_pName) {
            targetTypeName = typeClass->m_pName;
          }
          else {
            // Fallback for types that might not resolve cleanly
            targetTypeName = "Unknown";
          }
          break;
        }
      }

      if (targetTypeName.empty() || targetTypeName == "Unknown")
        return;

      // Second pass: Count how many fields of this type appear BEFORE our target offset
      iter           = nullptr;
      int  typeIndex = 0;
      bool found     = false;

      while (auto field = IL2CPP::Class::GetFields(m_pClass, &iter)) {
        auto        typeClass       = IL2CPP::Class::Utils::ClassFromType(field->m_pType);
        std::string currentTypeName = (typeClass && typeClass->m_pName) ? typeClass->m_pName : "Unknown";

        if (currentTypeName == targetTypeName) {
          if (field->m_iOffset == targetOffset) {
            found = true;
            break;
          }
          typeIndex++;
        }
      }

      if (found) {
        rules.push_back({name, className, targetTypeName, typeIndex});
      }
    }

    void DumpSignatures()
    {
      std::vector<SigRule> rules;

      // Character
      ProcessField("Character_minMoveSpeed", "Character", Offsets::Character_minMoveSpeed, rules);
      ProcessField("Character_characterName", "Character", Offsets::Character_characterName, rules);
      ProcessField("Character_camp", "Character", Offsets::Character_camp, rules);
      ProcessField("Character_body", "Character", Offsets::Character_body, rules);
      ProcessField("Character_buffManager", "Character", Offsets::Character_buffManager, rules);
      ProcessField("Character_properties", "Character", Offsets::Character_properties, rules);
      ProcessField("Character_isBlockDamage", "Character", Offsets::Character_isBlockDamage, rules);
      ProcessField("Character_healthLockValue", "Character", Offsets::Character_healthLockValue, rules);
      ProcessField("Character_uMoveSpeed", "Character", Offsets::Character_uMoveSpeed, rules);
      ProcessField("Character_uStiff", "Character", Offsets::Character_uStiff, rules);
      ProcessField("Character_uAntiStiff", "Character", Offsets::Character_uAntiStiff, rules);
      ProcessField("Character_uDamage", "Character", Offsets::Character_uDamage, rules);
      ProcessField("Character_uCrit", "Character", Offsets::Character_uCrit, rules);
      ProcessField("Character_uCritDamageRate", "Character", Offsets::Character_uCritDamageRate, rules);
      ProcessField("Character_uDamageReduce", "Character", Offsets::Character_uDamageReduce, rules);
      ProcessField("Character_uAttackSpeed", "Character", Offsets::Character_uAttackSpeed, rules);
      ProcessField("Character_uMaxHealth", "Character", Offsets::Character_uMaxHealth, rules);
      ProcessField("Character_uMaxShield", "Character", Offsets::Character_uMaxShield, rules);
      ProcessField("Character_currentHealth", "Character", Offsets::Character_currentHealth, rules);
      ProcessField("Character_currentShield", "Character", Offsets::Character_currentShield, rules);
      ProcessField("Character_LockHealthMultiBool", "Character", Offsets::Character_LockHealthMultiBool, rules);
      ProcessField("Character_toughness", "Character", Offsets::Character_toughness, rules);

      // Hero
      ProcessField("Hero_body", "Hero", Offsets::Hero_body, rules);
      ProcessField("Hero_heroTyrant", "Hero", Offsets::Hero_heroTyrant, rules);
      ProcessField("Hero_herolockHealth", "Hero", Offsets::Hero_herolockHealth, rules);
      ProcessField("Hero_heroNeverDieCountdown", "Hero", Offsets::Hero_heroNeverDieCountdown, rules);
      ProcessField("Hero_isMaxHealthGift", "Hero", Offsets::Hero_isMaxHealthGift, rules);

      // Body
      ProcessField("Body_myHeroClass", "Body", Offsets::Body_myHeroClass, rules);
      ProcessField("Body_mainWeapon", "Body", Offsets::Body_mainWeapon, rules);
      ProcessField("Body_bodyName", "Body", Offsets::Body_bodyName, rules);
      ProcessField("Body_isFaceRight", "Body", Offsets::Body_isFaceRight, rules);
      ProcessField("Body_states", "Body", Offsets::Body_states, rules);
      ProcessField("Body_character", "Body", Offsets::Body_character, rules);
      ProcessField("Body_skillList", "Body", Offsets::Body_skillList, rules);
      ProcessField("Body_invincibleCountdown", "Body", Offsets::Body_invincibleCountdown, rules);
      ProcessField("Body_isTyrants", "Body", Offsets::Body_isTyrants, rules);
      ProcessField("Body_isStiffRes", "Body", Offsets::Body_isStiffRes, rules);
      ProcessField("Body_invincibleMultiBool", "Body", Offsets::Body_invincibleMultiBool, rules);
      ProcessField("Body_tyrantMultiBool", "Body", Offsets::Body_tyrantMultiBool, rules);
      ProcessField("Body_mainSkillCooldown", "Body", Offsets::Body_mainSkillCooldown, rules);
      ProcessField("Body_exSkillCooldown", "Body", Offsets::Body_exSkillCooldown, rules);
      ProcessField("Body_weaponSkillCooldown", "Body", Offsets::Body_weaponSkillCooldown, rules);
      ProcessField("Body_skillCooldownFlowRate", "Body", Offsets::Body_skillCooldownFlowRate, rules);

      // Weapon
      ProcessField("Weapon_damage", "Weapon", Offsets::Weapon_damage, rules);

      // DamageMessage
      ProcessField("DamageMessage_isCrit", "DamageMessage", Offsets::DamageMessage_isCrit, rules);
      ProcessField("DamageMessage_damage", "DamageMessage", Offsets::DamageMessage_damage, rules);
      ProcessField("DamageMessage_finalDamage", "DamageMessage", Offsets::DamageMessage_finalDamage, rules);
      ProcessField("DamageMessage_stiff", "DamageMessage", Offsets::DamageMessage_stiff, rules);

      // HeroRunningData
      ProcessField("HRD_currentHealth", "HeroRunningData", Offsets::HRD_currentHealth, rules);
      ProcessField("HRD_currentShield", "HeroRunningData", Offsets::HRD_currentShield, rules);
      ProcessField("HRD_coinPickUp", "HeroRunningData", Offsets::HRD_coinPickUp, rules);
      ProcessField("HRD_currentHPBottleNumber", "HeroRunningData", Offsets::HRD_currentHPBottleNumber, rules);

      // PlayerArchive
      ProcessField("PlayerArchive_isCheated", "PlayerArchive", Offsets::PlayerArchive_isCheated, rules);
      ProcessField("PlayerArchive_exSoulStone", "PlayerArchive", Offsets::PlayerArchive_exSoulStone, rules);
      ProcessField("PlayerArchive_exBlueStone", "PlayerArchive", Offsets::PlayerArchive_exBlueStone, rules);
      ProcessField("PlayerArchive_exRedStone", "PlayerArchive", Offsets::PlayerArchive_exRedStone, rules);

      // Ive.Countdown
      ProcessField("Countdown_interval", "Ive.Countdown", Offsets::Countdown_interval, rules);
      ProcessField("Countdown_isFreeze", "Ive.Countdown", Offsets::Countdown_isFreeze, rules);
      ProcessField("Countdown_isPause", "Ive.Countdown", Offsets::Countdown_isPause, rules);

      // Ive.Properties
      ProcessField("Properties_baseProperty", "Ive.Properties", Offsets::Properties_baseProperty, rules);

      // MultiFloat
      ProcessField("MultiFloat_minValue", "MultiFloat", Offsets::MultiFloat_minValue, rules);
      ProcessField("MultiFloat_fixedValue", "MultiFloat", Offsets::MultiFloat_fixedValue, rules);
      ProcessField("MultiFloat_handlerList", "MultiFloat", Offsets::MultiFloat_handlerList, rules);

      // MultiBool
      ProcessField("MultiBool_value", "MultiBool", Offsets::MultiBool_value, rules);
      ProcessField("MultiBool_handlerList", "MultiBool", Offsets::MultiBool_handlerList, rules);

      // EncryptValue
      ProcessField("EncryptValue_value", "EncryptValue", Offsets::EncryptValue_value, rules);
      ProcessField("EncryptValue_key", "EncryptValue", Offsets::EncryptValue_key, rules);
      ProcessField("EncryptValue_encrypted", "EncryptValue", Offsets::EncryptValue_encrypted, rules);

      // Monster
      ProcessField("Monster_body", "Monster", Offsets::Monster_body, rules);

      // Output to runtime_signatures.json to avoid overwriting config.json's other fields (like 'offsets' for aobgenerator)
      std::ofstream file("runtime_signatures.json");
      if (file.is_open()) {
        file << "{\n";
        file << "  \"projectName\": \"Otherworld-Legends\",\n";
        file << "  \"version\": \"3.6.0.3\",\n";
        file << "  \"structural_signatures\": [\n";

        for (size_t i = 0; i < rules.size(); i++) {
          file << "    {\n";
          file << "      \"name\": \"" << rules[i].name << "\",\n";
          file << "      \"class\": \"" << rules[i].className << "\",\n";
          file << "      \"type\": \"" << rules[i].typeName << "\",\n";
          file << "      \"index\": " << rules[i].index << "\n";
          file << "    }";
          if (i < rules.size() - 1)
            file << ",";
          file << "\n";
        }

        file << "  ]\n";
        file << "}\n";
        file.close();
      }
    }

    // A simple JSON parser (since we don't have nlohmann/json in this project)
    std::string ExtractJsonValue(const std::string& line, const std::string& key, bool isString)
    {
      size_t pos = line.find("\"" + key + "\"");
      if (pos == std::string::npos)
        return "";
      pos = line.find(":", pos);
      if (pos == std::string::npos)
        return "";

      size_t start, end;
      if (isString) {
        start = line.find("\"", pos + 1);
        if (start == std::string::npos)
          return "";
        end = line.find("\"", start + 1);
        if (end == std::string::npos)
          return "";
        return line.substr(start + 1, end - start - 1);
      }
      else {
        start = line.find_first_not_of(" \t", pos + 1);
        if (start == std::string::npos)
          return "";
        end = line.find_first_of(" \t,\n\r}", start);
        if (end == std::string::npos)
          end = line.length();
        return line.substr(start, end - start);
      }
    }

    void GenerateOffsetsHpp()
    {
      std::ifstream file("runtime_signatures.json");
      if (!file.is_open())
        return;

      std::vector<SigRule> rules;
      std::string          line;
      SigRule              currentRule;

      while (std::getline(file, line)) {
        if (line.find("{") != std::string::npos) {
          currentRule = SigRule();
        }

        std::string name = ExtractJsonValue(line, "name", true);
        if (!name.empty())
          currentRule.name = name;

        std::string cls = ExtractJsonValue(line, "class", true);
        if (!cls.empty())
          currentRule.className = cls;

        std::string type = ExtractJsonValue(line, "type", true);
        if (!type.empty())
          currentRule.typeName = type;

        std::string idx = ExtractJsonValue(line, "index", false);
        if (!idx.empty())
          currentRule.index = std::stoi(idx);

        if (line.find("}") != std::string::npos) {
          if (!currentRule.name.empty()) {
            rules.push_back(currentRule);
            currentRule.name = "";
          }
        }
      }
      file.close();

      std::ofstream out("Offsets_New.hpp");
      if (!out.is_open())
        return;

      out << "/*\n";
      out << " * Offsets.hpp - Auto-generated from config.json structural rules at runtime\n";
      out << " */\n\n";
      out << "#pragma once\n\n";
      out << "namespace Offsets\n{\n";

      for (const auto& rule : rules) {
        auto m_pClass = IL2CPP::Class::Find(rule.className.c_str());
        if (!m_pClass) {
          out << "  // constexpr int " << rule.name << " = -1; // CLASS NOT FOUND\n";
          continue;
        }

        void* iter        = nullptr;
        int   typeIndex   = 0;
        int   foundOffset = -1;

        while (auto field = IL2CPP::Class::GetFields(m_pClass, &iter)) {
          auto        typeClass       = IL2CPP::Class::Utils::ClassFromType(field->m_pType);
          std::string currentTypeName = (typeClass && typeClass->m_pName) ? typeClass->m_pName : "Unknown";

          if (currentTypeName == rule.typeName) {
            if (typeIndex == rule.index) {
              foundOffset = field->m_iOffset;
              break;
            }
            typeIndex++;
          }
        }

        if (foundOffset != -1) {
          out << "  constexpr int " << rule.name << " = 0x" << std::hex << std::uppercase << foundOffset << ";\n";
        }
        else {
          out << "  // constexpr int " << rule.name << " = -1; // FIELD NOT FOUND\n";
        }
      }

      out << "\n}  // namespace Offsets\n";
      out.close();
    }
  }  // namespace SignatureDumper
}  // namespace Features
