#include <windows.h>

#include "vtable/vmthooks.h"
#include "hooking/Hooking.h"

#include <fstream>

HANDLE thread = nullptr;
utils::VtableHook* game_hook = nullptr;
void** global_debug_console = nullptr;

class CRTTISystem;
struct CFunction;
struct CName;
struct CNameHash;

CRTTISystem* rtti_system = nullptr;
void* native_globals_function_map = nullptr;

struct CNameHash {
  uint32_t name_hash;

  CNameHash(const char* name) {
    unsigned int hash = 2166136261;

    while (*name != 0) {
      hash = 16777619 * (*name ^ hash);

      name++;
    }

    name_hash = 16777619 * hash;
  }

  CNameHash(const wchar_t* name) {
    unsigned int hash = 2166136261;

    while (*name != 0) {
      hash = 16777619 * (*name ^ hash);
      auto next = (unsigned int)*name >> 8;
      if (next) {
        hash = 16777619 * (next ^ hash);
      }

      name++;
    }

    name_hash = 16777619 * hash;

  }
};

static hook::thiscall_stub<CName*(CName*, CNameHash)> CName_CreateNameFromHash([]() {
  return hook::pattern("48 89 5C 24 ? 57 48 83 EC 20 8B DA 48 8B F9 85 D2 74 37").count(1).get(0).get<void>(0);
});


static hook::thiscall_stub<const wchar_t*(CName*)> CName_AsChar([]() {
  return hook::pattern("40 53 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B D9 48 85 C0 75 05 E8 ? ? ? ? 8B 13 48 8B 0D ? ? ? ? E8").count(1).get(0).get<void>(0);
});

struct CName {
  uint32_t pool_index;
  CName(CNameHash hash) {
    CName_CreateNameFromHash(this, hash);
  }

  const wchar_t* AsChar() {
    return CName_AsChar(this);
  }
};

struct CClass {
  virtual void Deconstructor();
  virtual CName& GetName();

  char _0x0008[36];
  CName name; //0x002C 
};

struct CProperty {
  uint64_t vtable;
  CClass* class_type;
  CName name; //0x0010 
  char _0x0014[4];
  uint64_t N23BE7CFD; //0x0018 
  uint32_t N23BE7CFE; //0x0020 
  uint32_t optional; //0x0024 
  uint64_t N23BE7CFF; //0x0028 
};

struct CFunction {
  uint64_t vtable;
  uint64_t unknown_0;
  CName name;
  uint16_t flags;
  uint16_t align_2;
  uint64_t unknown_1;
  uint64_t* property_array;
  uint32_t argument_count;
  uint32_t align_0;
  uint64_t unknown_4;
  uint64_t unknown_5;
  uint64_t unknown_6;
  uint64_t unknown_7;
  uint64_t unknown_8;
  uint64_t unknown_9;
  uint32_t unknown_10;
  uint32_t align_1;
  char _CScriptCompiledCode[80];
  uint32_t registration_offset; // 0xB8
  char _0x00BC[12];
  uint64_t unknown_11;

  uint64_t* GetFunctionAddress() {
    return (uint64_t*)((uint64_t)native_globals_function_map + (8 * this->registration_offset));
  }
};

static_assert(sizeof(CFunction) == 208, "CFunction has wrong size!");

static hook::thiscall_stub<CFunction*(CRTTISystem*, CName&)> FindGlobalFunctionHook([]() {
  return hook::pattern("83 79 44 00 74 2E").count(1).get(0).get<void>(0);
});

class CRTTISystem {
public:
  char _0x0000[64];
  uint32_t native_globals_sort; //0x0040 
  uint32_t native_globals_count; //0x0044 
  char _0x0048[24];
  uint64_t* global_function_register; //0x0060

  CFunction* FindGlobalFunction(CName& name) { return FindGlobalFunctionHook(this, name); }

  void DumpGlobalFunctions() {
    if (!native_globals_count) return;

    std::wofstream log_file;
    log_file.open("global_functions_log.txt");

    for (size_t i = 0; i < native_globals_count; i++) {
      auto function_entry = (global_function_register + i);
      if (!function_entry || !*function_entry) continue;
      auto function = *(CFunction**)(*function_entry + 0x8);
      if (!function) continue;

      log_file << function->name.AsChar() << " " << std::dec << function->flags << std::endl;
      for (size_t i = 0; i < function->argument_count; i++) {
        auto property = (CProperty*)function->property_array[i];
        log_file << " " << property->class_type->GetName().AsChar() << " " << property->name.AsChar() << " " << property->optional << std::endl;
      }
    }

    log_file.close();
  }
};

typedef bool(*OnViewportInputType) (void* thisptr, void* viewport, int input_key, int input_action, float tick);
OnViewportInputType OnViewportInputDebugConsole = nullptr;

bool OnViewportInputDebugAlwaysHook(void* thisptr, void* viewport, int input_key, int input_action, float tick) {
  return OnViewportInputDebugConsole(*global_debug_console, viewport, input_key, input_action, tick);
}

void funcLogHook(struct CObject *a1, struct CScriptStackFrame *a2, void *a3) {
  OutputDebugStringW(L"test log");
}

void ReplaceFunction(const char* name, uint64_t address) {
  auto function = rtti_system->FindGlobalFunction(CName(name));

  if (function) {
    std::wstringstream message;
    message << "ReplaceFunction: " << name << " " << std::hex << function << " " << " " << std::hex << function->GetFunctionAddress();
    OutputDebugStringW(message.str().c_str());

    *function->GetFunctionAddress() = address;
  }
}

DWORD WINAPI InitializeHook(void* arguments) {
  hook::set_base();
  HookFunction::RunAll();

  char* location = hook::pattern("48 8B 05 ? ? ? ? 48 8D 4C 24 ? C6 44 24").count(1).get(0).get<char>(3);
  void** global_game = reinterpret_cast<void**>(location + *(int32_t*)location + 4);

  location = hook::pattern("48 89 05 ? ? ? ? EB 07 48 89 35 ? ? ? ? 48 8B 47 60").count(1).get(0).get<char>(3);
  global_debug_console = reinterpret_cast<void**>(location + *(int32_t*)location + 4);

  while (*global_game == nullptr || *global_debug_console == nullptr) {
    OutputDebugStringW(L"Wait for game");
    Sleep(500);
  }

  location = hook::pattern("48 83 EC 28 48 8B 05 ? ? ? ? 0F B6 90 ? ? ? ?").count(1).get(0).get<char>(0);
  OnViewportInputDebugConsole = (OnViewportInputType)location;

  location = hook::pattern("48 8B 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 30").count(1).get(0).get<char>(3);
  rtti_system = *reinterpret_cast<CRTTISystem**>(location + *(int32_t*)location + 4);

  location = hook::pattern("4C 8D 0D ? ? ? ? 49 89 14 C1").count(1).get(0).get<char>(3);
  native_globals_function_map = reinterpret_cast<void*>(location + *(int32_t*)location + 4);

  OutputDebugStringW(L"Hook game");

  game_hook = new utils::VtableHook(*global_game);
  game_hook->HookMethod(OnViewportInputDebugAlwaysHook, 128);

  //rtti_system->DumpGlobalFunctions();

  //ReplaceFunction("Log", (uint64_t)&funcLogHook);
  //ReplaceFunction("LogChannel", (uint64_t)&funcLogHook);
  //ReplaceFunction("Trace", (uint64_t)&funcLogHook);
  //ReplaceFunction("DebugBreak", (uint64_t)&funcLogHook);

  return 1;
}

void FinalizeHook() {
  if (game_hook) delete game_hook;
}

int WINAPI DllMain(HINSTANCE instance, DWORD reason, PVOID reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    thread = CreateThread(nullptr, 0, InitializeHook, 0, 0, nullptr);
  }
  else if (reason == DLL_PROCESS_DETACH) {
    FinalizeHook();
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
  }
  return 1;
}