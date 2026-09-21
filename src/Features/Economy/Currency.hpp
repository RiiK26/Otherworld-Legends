#pragma once

namespace Features
{
  namespace Currency
  {
    void Initialize();
    void Uninitialize();
    void ApplyInfiniteLobbyCurrency();
    void OnTick();
  }  // namespace Currency
}  // namespace Features
