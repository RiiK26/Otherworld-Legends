#include "DailyChest.hpp"
#include "../../Modules/Hooks/Hooks.hpp"
#include "../../Modules/Menu/Menu.hpp"

namespace Features::DailyChest
{
  typedef int (*ReadNextBoxFinalLevel_t)(void* _this);
  ReadNextBoxFinalLevel_t oReadNextBoxFinalLevel = nullptr;

  int hkReadNextBoxFinalLevel(void* _this)
  {
    if (Menu::Config.bDailyChest) {
      return Menu::Config.iDailyChestTier;
    }
    if (oReadNextBoxFinalLevel) {
      return oReadNextBoxFinalLevel(_this);
    }
    return 0;
  }

  void Initialize()
  {
    HOOK_METHOD("DailyBoxArchive", "ReadNextBoxFinalLevel", 0, hkReadNextBoxFinalLevel, oReadNextBoxFinalLevel);
  }

  void Uninitialize() { }
}  // namespace Features::DailyChest
