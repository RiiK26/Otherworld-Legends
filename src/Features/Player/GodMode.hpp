#pragma once

namespace Features
{
  namespace GodMode
  {
    void Initialize();
    void Uninitialize();

    // Per-frame tick — called from the Present hook in Menu.cpp
    void OnTick();
  }  // namespace GodMode
}  // namespace Features
