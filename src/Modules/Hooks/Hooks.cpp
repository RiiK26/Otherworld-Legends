#include "Hooks.hpp"

#include "../../Features/Player/GodMode.hpp"
#include "../../Features/Economy/Currency.hpp"
#include "../../Features/AntiCheat/AntiCheat.hpp"

#include "MinHook.h"

bool Hooks::bHooksFailed = false;

void Hooks::Initialize()
{
  int mhStatus = MH_Initialize();
  if (mhStatus != MH_OK && mhStatus != MH_ERROR_ALREADY_INITIALIZED) {
    Hooks::bHooksFailed = true;
  }

  // Initialize modular features
  Features::GodMode::Initialize();
  Features::Currency::Initialize();
  Features::AntiCheat::Initialize();
}

void Hooks::Uninitialize()
{
  Features::GodMode::Uninitialize();
  Features::Currency::Uninitialize();
  Features::AntiCheat::Uninitialize();

  // Disable/uninitialize MinHook here
  MH_DisableHook(MH_ALL_HOOKS);
  MH_Uninitialize();
}
