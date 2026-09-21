#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace Menu
{
  struct ConfigData
  {
    bool bMenuOpen = true;

    // === Player ===
    bool  bGodMode                = false;
    bool  bGodMode_InfiniteHP     = true;
    bool  bGodMode_Damage         = false;
    bool  bGodMode_Speed          = false;
    float fGodModeDamage          = 99999.0f;
    float fGodModeSpeedMultiplier = 10.0f;
    bool  bNoCooldown             = false;

    // === Economy ===
    bool bInfiniteCurrency = false;

    void LoadConfig();
    void SaveConfig();
  };

  extern ConfigData Config;

  void Initialize();
  void Uninitialize();
}  // namespace Menu
