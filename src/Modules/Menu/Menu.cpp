#include "Menu.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include "MinHook.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include "../../Features/Player/GodMode.hpp"
#include "../../Features/Economy/Currency.hpp"
#include "../../Features/AntiCheat/AntiCheat.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Menu
{
  ConfigData         Config;
  static std::string g_IniPath;

  std::string GetConfigPath()
  {
    char    path[MAX_PATH];
    HMODULE hMod = NULL;
    GetModuleHandleExA(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &GetConfigPath,
      &hMod
    );
    if (hMod && GetModuleFileNameA(hMod, path, MAX_PATH)) {
      std::string fullPath(path);
      size_t      lastSlash = fullPath.find_last_of("\\/");
      if (lastSlash != std::string::npos) {
        return fullPath.substr(0, lastSlash) + ("\\config.txt");
      }
    }
    return ("config.txt");
  }

  static float SafeParseFloat(const std::string& value, float defaultValue)
  {
    try {
      return std::stof(value);
    } catch (...) {
      return defaultValue;
    }
  }

  void ConfigData::LoadConfig()
  {
    std::ifstream f(GetConfigPath());
    if (!f.is_open())
      return;

    std::string line;
    while (std::getline(f, line)) {
      std::istringstream is_line(line);
      std::string        key;
      if (std::getline(is_line, key, '=')) {
        std::string value;
        if (std::getline(is_line, value)) {
          if (key == ("god_mode"))
            bGodMode = (value == "1");
          else if (key == ("god_mode_infinite_hp"))
            bGodMode_InfiniteHP = (value == "1");
          else if (key == ("god_mode_damage"))
            bGodMode_Damage = (value == "1");
          else if (key == ("god_mode_speed"))
            bGodMode_Speed = (value == "1");
          else if (key == ("god_mode_damage_val"))
            fGodModeDamage = SafeParseFloat(value, fGodModeDamage);
          else if (key == ("god_mode_speed_val"))
            fGodModeSpeedMultiplier = SafeParseFloat(value, fGodModeSpeedMultiplier);
          else if (key == ("no_cooldown"))
            bNoCooldown = (value == "1");
          else if (key == ("infinite_currency"))
            bInfiniteCurrency = (value == "1");
        }
      }
    }
  }

  void ConfigData::SaveConfig()
  {
    std::ofstream out(GetConfigPath());
    if (!out.is_open())
      return;

    out << ("menu_open=") << (bMenuOpen ? "1" : "0") << "\n";
    out << ("god_mode=") << (bGodMode ? "1" : "0") << "\n";
    out << ("god_mode_infinite_hp=") << (bGodMode_InfiniteHP ? "1" : "0") << "\n";
    out << ("god_mode_damage=") << (bGodMode_Damage ? "1" : "0") << "\n";
    out << ("god_mode_speed=") << (bGodMode_Speed ? "1" : "0") << "\n";
    out << ("god_mode_damage_val=") << fGodModeDamage << "\n";
    out << ("god_mode_speed_val=") << fGodModeSpeedMultiplier << "\n";
    out << ("no_cooldown=") << (bNoCooldown ? "1" : "0") << "\n";
    out << ("infinite_currency=") << (bInfiniteCurrency ? "1" : "0") << "\n";
  }

  typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
  Present_t oPresent = nullptr;

  typedef HRESULT(__stdcall* ResizeBuffers_t)(
    IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags
  );
  ResizeBuffers_t oResizeBuffers = nullptr;

  typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
  WNDPROC oWndProc                                         = nullptr;

  HWND                    window                           = nullptr;
  ID3D11Device*           pDevice                          = nullptr;
  ID3D11DeviceContext*    pContext                         = nullptr;
  ID3D11RenderTargetView* mainRenderTargetView             = nullptr;
  bool                    init                             = false;

  ID3D12Device*              g_pd3dDevice                  = nullptr;
  ID3D12DescriptorHeap*      g_pd3dRtvDescHeap             = nullptr;
  ID3D12DescriptorHeap*      g_pd3dSrvDescHeap             = nullptr;
  ID3D12CommandQueue*        g_pd3dCommandQueue            = nullptr;
  ID3D12GraphicsCommandList* g_pd3dCommandList             = nullptr;
  ID3D12CommandAllocator*    g_commandAllocators[3]        = {};
  ID3D12Resource*            g_mainRenderTargetResource[3] = {};
  UINT                       g_NUM_FRAMES_IN_FLIGHT        = 3;
  bool                       initDX12                      = false;

  typedef void(__stdcall* ExecuteCommandLists_t)(
    ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists
  );
  ExecuteCommandLists_t oExecuteCommandLists = nullptr;

  void CleanupRenderTargetDX12()
  {
    for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++) {
      if (g_mainRenderTargetResource[i]) {
        g_mainRenderTargetResource[i]->Release();
        g_mainRenderTargetResource[i] = nullptr;
      }
    }
  }

  void CreateRenderTargetDX12(IDXGISwapChain* pSwapChain, ID3D12Device* pDevice)
  {
    for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++) {
      ID3D12Resource* pBackBuffer = nullptr;
      pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**) &pBackBuffer);
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
      rtvHandle.ptr += i * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
      pDevice->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
      g_mainRenderTargetResource[i] = pBackBuffer;
    }
  }

  void CleanupRenderTarget()
  {
    if (mainRenderTargetView) {
      mainRenderTargetView->Release();
      mainRenderTargetView = nullptr;
    }
  }

  void CreateRenderTarget(IDXGISwapChain* pSwapChain)
  {
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &pBackBuffer);
    pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
    pBackBuffer->Release();
  }

  static bool IsMouseMessage(UINT uMsg)
  {
    return uMsg == WM_MOUSEMOVE || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN
        || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL || uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP;
  }

  static bool IsKeyboardMessage(UINT uMsg)
  {
    return uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_CHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;
  }

  LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
      Config.bMenuOpen = !Config.bMenuOpen;
      return 1;
    }

    if (Config.bMenuOpen) {
      ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

      ImGuiIO& io = ImGui::GetIO();

      // Block mouse input to the game ONLY if ImGui wants to capture it (e.g. hovering over the menu)
      if (io.WantCaptureMouse && IsMouseMessage(uMsg)) {
        return 1;
      }

      // Block keyboard input to the game ONLY if ImGui is focused on a text input
      if (io.WantCaptureKeyboard && IsKeyboardMessage(uMsg)) {
        return 1;
      }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
  }

  HRESULT __stdcall hkResizeBuffers(
    IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags
  )
  {
    CleanupRenderTarget();
    if (initDX12)
      CleanupRenderTargetDX12();

    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (pDevice)
      CreateRenderTarget(pSwapChain);
    if (initDX12 && g_pd3dDevice)
      CreateRenderTargetDX12(pSwapChain, g_pd3dDevice);

    return hr;
  }

  void DrawMenuUI()
  {
    if (Config.bMenuOpen) {
      ImGui::SetNextWindowSize(ImVec2(650, 650), ImGuiCond_FirstUseEver);
      ImGui::Begin(
        ("Otherworld Legends v" PROJECT_VERSION), nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
      );

      if (ImGui::BeginTabBar("CheatTabs", ImGuiTabBarFlags_FittingPolicyScroll)) {
        // Player Tab menu
        if (ImGui::BeginTabItem("Player")) {
          ImGui::BeginChild("PlayerChild", ImVec2(0, -65), false, 0);

          ImGui::Checkbox("God Mode (Master)", &Config.bGodMode);
          if (ImGui::TreeNode("God Mode Settings")) {
            ImGui::Checkbox("Infinite HP / Shield", &Config.bGodMode_InfiniteHP);
            ImGui::Checkbox("High Damage", &Config.bGodMode_Damage);
            // ImGui::InputFloat("##GodModeDamage"), &Config.fGodModeDamage);
            ImGui::Checkbox("Movement Speed", &Config.bGodMode_Speed);
            ImGui::SliderFloat("##GodModeSpeed", &Config.fGodModeSpeedMultiplier, 1.0f, 15.0f);
            ImGui::TreePop();
          }

          ImGui::Checkbox("No Skill Cooldowns", &Config.bNoCooldown);

          ImGui::EndChild();
          ImGui::EndTabItem();
        }

        // Economy Tab menu
        if (ImGui::BeginTabItem("Economy")) {
          ImGui::BeginChild("EconomyChild", ImVec2(0, -65), false, 0);

          if (ImGui::Button("Max Currency (Lobby) - 99,999", ImVec2(-1, 30))) {
            Features::Currency::ApplyInfiniteLobbyCurrency();
          }
          if (ImGui::IsItemHovered())
            ImGui::SetTooltip(
              "Sets Soul Stones, Sapphires, and Rubies to 99,999.\nUse this inside the lobby, then buy something to "
              "trigger a save."
            );

          ImGui::TextDisabled("Run Gold is temporarily disabled for stability.");

          ImGui::EndChild();
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }

      // Save Config button
      ImGui::Separator();
      if (ImGui::Button("Save Config", ImVec2(-1, 0))) {
        Config.SaveConfig();
      }
      ImGui::Separator();
      ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[INSERT] show/hide menu");

      ImGui::End();
    }
  }

  void RenderImGui_DX11(IDXGISwapChain* pSwapChain, ID3D11Device* pDeviceArg)
  {
    if (!init) {
      pDevice = pDeviceArg;
      if (pDevice) {
        pDevice->GetImmediateContext(&pContext);
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        window = sd.OutputWindow;
        CreateRenderTarget(pSwapChain);

        oWndProc = (WNDPROC) SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) WndProc);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        char    path[MAX_PATH];
        HMODULE hMod = NULL;
        GetModuleHandleExA(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          (LPCSTR) &GetConfigPath, &hMod
        );
        if (hMod && GetModuleFileNameA(hMod, path, MAX_PATH)) {
          std::string fullPath(path);
          size_t      lastSlash = fullPath.find_last_of("\\/");
          if (lastSlash != std::string::npos) {
            g_IniPath      = fullPath.substr(0, lastSlash) + ("\\imgui.ini");
            io.IniFilename = g_IniPath.c_str();
          }
        }

        ImGui_ImplWin32_Init(window);
        ImGui_ImplDX11_Init(pDevice, pContext);

        ImGui::StyleColorsDark();

        // --- Apply Styling ---
        ImGuiStyle& style       = ImGui::GetStyle();
        style.WindowRounding    = 8.0f;
        style.FrameRounding     = 6.0f;
        style.PopupRounding     = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding      = 6.0f;
        style.TabRounding       = 6.0f;

        style.WindowPadding     = ImVec2(12, 12);
        style.FramePadding      = ImVec2(8, 4);
        style.ItemSpacing       = ImVec2(8, 8);
        style.ItemInnerSpacing  = ImVec2(6, 6);

        // Custom Dark/Vibrant Palette
        ImVec4* colors                    = style.Colors;
        colors[ImGuiCol_WindowBg]         = ImVec4(0.08f, 0.08f, 0.09f, 0.96f);
        colors[ImGuiCol_Header]           = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderHovered]    = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
        colors[ImGuiCol_HeaderActive]     = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
        colors[ImGuiCol_Button]           = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
        colors[ImGuiCol_ButtonHovered]    = ImVec4(0.26f, 0.35f, 0.44f, 1.00f);
        colors[ImGuiCol_ButtonActive]     = ImVec4(0.36f, 0.45f, 0.54f, 1.00f);
        colors[ImGuiCol_FrameBg]          = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgActive]    = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
        colors[ImGuiCol_CheckMark]        = ImVec4(0.30f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab]       = ImVec4(0.30f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.38f, 0.73f, 1.00f, 1.00f);
        colors[ImGuiCol_TitleBg]          = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive]    = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);

        init                              = true;
      }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawMenuUI();

    ImGui::Render();

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }  // namespace Menu


  void RenderImGui_DX12(IDXGISwapChain* pSwapChain, ID3D12Device* pDevice)
  {
    if (!g_pd3dCommandQueue)
      return;

    if (!initDX12) {
      g_pd3dDevice = pDevice;

      DXGI_SWAP_CHAIN_DESC sd;
      pSwapChain->GetDesc(&sd);
      window                 = sd.OutputWindow;
      oWndProc               = (WNDPROC) SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) WndProc);

      g_NUM_FRAMES_IN_FLIGHT = sd.BufferCount;
      if (g_NUM_FRAMES_IN_FLIGHT > 3)
        g_NUM_FRAMES_IN_FLIGHT = 3;

      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.NumDescriptors             = g_NUM_FRAMES_IN_FLIGHT;
      desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      desc.NodeMask                   = 1;
      if (pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
        return;

      D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
      srvdesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      srvdesc.NumDescriptors             = 1;
      srvdesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
      if (pDevice->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
        return;

      for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++) {
        if (
          pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i])) != S_OK
        )
          return;
      }

      if (
        pDevice->CreateCommandList(
          0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, IID_PPV_ARGS(&g_pd3dCommandList)
        )
        != S_OK
      )
        return;
      g_pd3dCommandList->Close();

      CreateRenderTargetDX12(pSwapChain, pDevice);

      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO();
      io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

      char    path[MAX_PATH];
      HMODULE hMod = NULL;
      GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &GetConfigPath,
        &hMod
      );
      if (hMod && GetModuleFileNameA(hMod, path, MAX_PATH)) {
        std::string fullPath(path);
        size_t      lastSlash = fullPath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
          g_IniPath      = fullPath.substr(0, lastSlash) + ("\\imgui.ini");
          io.IniFilename = g_IniPath.c_str();
        }
      }

      ImGui_ImplWin32_Init(window);
      ImGui_ImplDX12_InitInfo info      = {};
      info.Device                       = pDevice;
      info.CommandQueue                 = g_pd3dCommandQueue;
      info.NumFramesInFlight            = g_NUM_FRAMES_IN_FLIGHT;
      info.RTVFormat                    = DXGI_FORMAT_R8G8B8A8_UNORM;
      info.DSVFormat                    = DXGI_FORMAT_UNKNOWN;
      info.SrvDescriptorHeap            = g_pd3dSrvDescHeap;
      info.LegacySingleSrvCpuDescriptor = g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
      info.LegacySingleSrvGpuDescriptor = g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
      ImGui_ImplDX12_Init(&info);

      ImGui::StyleColorsDark();

      ImGuiStyle& style                 = ImGui::GetStyle();
      style.WindowRounding              = 8.0f;
      style.FrameRounding               = 6.0f;
      style.PopupRounding               = 6.0f;
      style.ScrollbarRounding           = 6.0f;
      style.GrabRounding                = 6.0f;
      style.TabRounding                 = 6.0f;

      style.WindowPadding               = ImVec2(12, 12);
      style.FramePadding                = ImVec2(8, 4);
      style.ItemSpacing                 = ImVec2(8, 8);
      style.ItemInnerSpacing            = ImVec2(6, 6);

      ImVec4* colors                    = style.Colors;
      colors[ImGuiCol_WindowBg]         = ImVec4(0.08f, 0.08f, 0.09f, 0.96f);
      colors[ImGuiCol_Header]           = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
      colors[ImGuiCol_HeaderHovered]    = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
      colors[ImGuiCol_HeaderActive]     = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
      colors[ImGuiCol_Button]           = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
      colors[ImGuiCol_ButtonHovered]    = ImVec4(0.26f, 0.35f, 0.44f, 1.00f);
      colors[ImGuiCol_ButtonActive]     = ImVec4(0.36f, 0.45f, 0.54f, 1.00f);
      colors[ImGuiCol_FrameBg]          = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
      colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
      colors[ImGuiCol_FrameBgActive]    = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
      colors[ImGuiCol_CheckMark]        = ImVec4(0.30f, 0.65f, 1.00f, 1.00f);
      colors[ImGuiCol_SliderGrab]       = ImVec4(0.30f, 0.65f, 1.00f, 1.00f);
      colors[ImGuiCol_SliderGrabActive] = ImVec4(0.38f, 0.73f, 1.00f, 1.00f);
      colors[ImGuiCol_TitleBg]          = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
      colors[ImGuiCol_TitleBgActive]    = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);

      initDX12                          = true;
    }

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawMenuUI();

    ImGui::Render();

    IDXGISwapChain3* pSwapChain3   = nullptr;
    UINT             backBufferIdx = 0;
    if (SUCCEEDED(pSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**) &pSwapChain3))) {
      backBufferIdx = pSwapChain3->GetCurrentBackBufferIndex();
      pSwapChain3->Release();
    }

    ID3D12CommandAllocator* commandAllocator = g_commandAllocators[backBufferIdx];
    commandAllocator->Reset();
    g_pd3dCommandList->Reset(commandAllocator, nullptr);

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIdx * pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    g_pd3dCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    ID3D12CommandList* ppCommandLists[] = {g_pd3dCommandList};
    g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);
  }

  void __stdcall
  hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
  {
    if (!g_pd3dCommandQueue && queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
      g_pd3dCommandQueue = queue;
    }
    return oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
  }

  HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
  {
    // Feature Ticks
    Features::GodMode::OnTick();
    Features::Currency::OnTick();
    Features::AntiCheat::OnTick();

    ID3D11Device* pD11 = nullptr;
    ID3D12Device* pD12 = nullptr;
    if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**) &pD11))) {
      RenderImGui_DX11(pSwapChain, pD11);
      pD11->Release();
    }
    else if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**) &pD12))) {
      RenderImGui_DX12(pSwapChain, pD12);
      pD12->Release();
    }
    return oPresent(pSwapChain, SyncInterval, Flags);
  }

  void Initialize()
  {
    Config.LoadConfig();

    // Dummy DX11 swap chain creation to get the vtable address of Present
    WNDCLASSEXA wc = {sizeof(WNDCLASSEXA),    CS_CLASSDC, DefWindowProcA, 0L,   0L,
                      GetModuleHandleA(NULL), NULL,       NULL,           NULL, NULL,
                      "EvitaniaDummy",        NULL};
    RegisterClassExA(&wc);
    HWND dummyWindow =
      CreateWindowA("EvitaniaDummy", "", WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount                    = 1;
    sd.BufferDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage                    = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                   = dummyWindow;
    sd.SampleDesc.Count               = 1;
    sd.Windowed                       = TRUE;
    sd.SwapEffect                     = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL featureLevel;

    IDXGISwapChain*      pDummySwapChain = nullptr;
    ID3D11Device*        pDummyDevice    = nullptr;
    ID3D11DeviceContext* pDummyContext   = nullptr;

    HRESULT hr                           = D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 3, D3D11_SDK_VERSION, &sd, &pDummySwapChain,
      &pDummyDevice, NULL, &pDummyContext
    );

    if (FAILED(hr)) {
      hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_WARP, NULL, 0, featureLevels, 3, D3D11_SDK_VERSION, &sd, &pDummySwapChain, &pDummyDevice,
        NULL, &pDummyContext
      );
    }

    if (SUCCEEDED(hr) && pDummySwapChain) {

      void** pVTable         = *reinterpret_cast<void***>(pDummySwapChain);
      void*  pPresent        = pVTable[8];

      MH_STATUS createStatus = MH_CreateHook(pPresent, (void*) hkPresent, (void**) &oPresent);
      if (createStatus != MH_OK) {
        char buf[64];
        snprintf(buf, sizeof(buf), ("MH_CreateHook for Present failed: %d"), (int) createStatus);
        MessageBoxA(NULL, buf, ("Otherworld Legends Error"), MB_OK);
      }

      MH_STATUS enableStatus = MH_EnableHook(pPresent);
      if (enableStatus != MH_OK) {
        char buf[64];
        snprintf(buf, sizeof(buf), ("MH_EnableHook for Present failed: %d"), (int) enableStatus);
        MessageBoxA(NULL, buf, ("Otherworld Legends Error"), MB_OK);
      }

      void*     pResizeBuffers = pVTable[13];
      MH_STATUS createStatusRB = MH_CreateHook(pResizeBuffers, (void*) hkResizeBuffers, (void**) &oResizeBuffers);
      if (createStatusRB == MH_OK) {
        MH_EnableHook(pResizeBuffers);
      }

      pDummySwapChain->Release();
      pDummyDevice->Release();
      pDummyContext->Release();
    }
    else {
      MessageBoxA(NULL, "Failed to create D3D11 Device (Menu might not show)", "Otherworld Legends Error", MB_OK);
    }

    // Dummy DX12 device creation to get the vtable address of ExecuteCommandLists
    ID3D12Device* pDummyD12Device = nullptr;
    if (SUCCEEDED(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**) &pDummyD12Device))) {
      D3D12_COMMAND_QUEUE_DESC queueDesc = {};
      queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
      queueDesc.Priority                 = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
      queueDesc.NodeMask                 = 0;

      ID3D12CommandQueue* pDummyQueue    = nullptr;
      if (
        SUCCEEDED(pDummyD12Device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**) &pDummyQueue))
      ) {
        void** pQueueVTable         = *reinterpret_cast<void***>(pDummyQueue);
        void*  pExecuteCommandLists = pQueueVTable[10];

        if (
          MH_CreateHook(pExecuteCommandLists, (void*) hkExecuteCommandLists, (void**) &oExecuteCommandLists) == MH_OK
        ) {
          MH_EnableHook(pExecuteCommandLists);
        }

        pDummyQueue->Release();
      }
      pDummyD12Device->Release();
    }

    DestroyWindow(dummyWindow);
    UnregisterClassA("EvitaniaDummy", wc.hInstance);
  }

  void Uninitialize()
  {
    if (init) {
      SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) oWndProc);
      ImGui_ImplDX11_Shutdown();
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
      CleanupRenderTarget();
    }
    if (initDX12) {
      SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR) oWndProc);
      ImGui_ImplDX12_Shutdown();
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
      CleanupRenderTargetDX12();
      for (UINT i = 0; i < g_NUM_FRAMES_IN_FLIGHT; i++)
        if (g_commandAllocators[i]) {
          g_commandAllocators[i]->Release();
          g_commandAllocators[i] = nullptr;
        }
      if (g_pd3dCommandList) {
        g_pd3dCommandList->Release();
        g_pd3dCommandList = nullptr;
      }
      if (g_pd3dRtvDescHeap) {
        g_pd3dRtvDescHeap->Release();
        g_pd3dRtvDescHeap = nullptr;
      }
      if (g_pd3dSrvDescHeap) {
        g_pd3dSrvDescHeap->Release();
        g_pd3dSrvDescHeap = nullptr;
      }
    }
  }
}  // namespace Menu
