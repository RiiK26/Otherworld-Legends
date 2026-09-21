#include <windows.h>
#include <stdio.h>
#include "../Modules/Il2CppResolver/IL2CPP_Resolver.hpp"

void DumpThread(HMODULE hModule)
{
  FILE* f = fopen("Z:\\home\\riik\\Projects\\OtherworldLegends\\currency_dump.txt", "w");
  if (!f)
    return;

  fprintf(f, "Initializing IL2CPP...\n");
  IL2CPP::Initialize();

  const char* names[] = {"GameProcess",       "Ive.GameProcess",         "ChillyRoom.GameProcess",
                         "Core.GameProcess",  "PlayerArchive",           "Archive.PlayerArchive",
                         "Ive.PlayerArchive", "ChillyRoom.PlayerArchive"};

  for (int i = 0; i < 8; i++) {
    Unity::il2cppClass* cls = IL2CPP::Class::Find(names[i]);
    fprintf(f, "Class %s: %p\n", names[i], cls);
    if (cls) {
      void* staticFields = *(void**) ((uintptr_t) cls + 0xB8);
      fprintf(f, "  staticFields: %p\n", staticFields);
    }
  }

  fprintf(f, "Done.\n");
  fclose(f);
  FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE) DumpThread, hModule, 0, 0);
  }
  return TRUE;
}
