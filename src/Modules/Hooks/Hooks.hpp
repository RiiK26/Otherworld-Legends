/*
HOOK_METHOD:    using IL2CPPResolver @src/Modules/Il2CppResolver Module
HOOK_SIGNATURE: using Signature scanner from @src/Modules/Hooks/Signatures.hpp
HOOK_OFFSET:    using Offset Scanner from @src/Modules/Offsets.hpp (Not used for now)
*/

#pragma once
#include "../Il2CppResolver/IL2CPP_Resolver.hpp"  // IWYU pragma: keep
#include "MinHook.h"                              // IWYU pragma: keep

namespace Hooks
{
  extern bool bHooksFailed;
}

#define HOOK_METHOD(ClassName, MethodName, ArgsCount, HookFunc, OrigFuncPtr) \
  do { \
    void* target = IL2CPP::ResolveUnityMethod(ClassName, MethodName, ArgsCount); \
    if (target) { \
      int createStatus = MH_CreateHook(target, (LPVOID) HookFunc, (LPVOID*) &OrigFuncPtr); \
      if (createStatus != MH_OK) { \
        Hooks::bHooksFailed = true; \
      } \
      else { \
        int enableStatus = MH_EnableHook(target); \
        if (enableStatus != MH_OK) { \
          Hooks::bHooksFailed = true; \
        } \
      } \
    } \
    else { \
      Hooks::bHooksFailed = true; \
    } \
  } while (0)

#include "../../Cores/Scanner.hpp"  // IWYU pragma: keep
#define HOOK_SIGNATURE(OffsetName, Signature, HookFunc, OrigFuncPtr) \
  do { \
    void* target = (void*) Scanner::FindPattern((HMODULE) IL2CPP::Globals.m_GameAssembly, Signature); \
    if (target) { \
      int createStatus = MH_CreateHook(target, (LPVOID) HookFunc, (LPVOID*) &OrigFuncPtr); \
      if (createStatus != MH_OK) { \
        Hooks::bHooksFailed = true; \
      } \
      else { \
        int enableStatus = MH_EnableHook(target); \
        if (enableStatus != MH_OK) { \
          Hooks::bHooksFailed = true; \
        } \
      } \
    } \
    else { \
      Hooks::bHooksFailed = true; \
    } \
  } while (0)

#define HOOK_OFFSET(OffsetName, RVA, HookFunc, OrigFuncPtr) \
  do { \
    void* target = (void*) ((uintptr_t) IL2CPP::Globals.m_GameAssembly + RVA); \
    if (target) { \
      int createStatus = MH_CreateHook(target, (LPVOID) HookFunc, (LPVOID*) &OrigFuncPtr); \
      if (createStatus != MH_OK) { \
        Hooks::bHooksFailed = true; \
      } \
      else { \
        int enableStatus = MH_EnableHook(target); \
        if (enableStatus != MH_OK) { \
          Hooks::bHooksFailed = true; \
        } \
      } \
    } \
    else { \
      Hooks::bHooksFailed = true; \
    } \
  } while (0)

namespace Hooks
{
  void Initialize();
  void Uninitialize();
}  // namespace Hooks
