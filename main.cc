#include <windows.h>
#include <fstream>

#include "vtable/vmthooks.h"
#include "hooking/Hooking.h"

#include "enums.h"

HANDLE thread = nullptr;
utils::VtableHook* game_hook = nullptr;
void** global_debug_console = nullptr;

class CRTTISystem;
struct CFunction;
struct CName;
struct CNameHash;
struct CEnum;

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
  CName() { pool_index = 0; };
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
  uint32_t flags;
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

static hook::thiscall_stub<CFunction*(CRTTISystem*, CName&)> CRTTISystem_FindGlobalFunction([]() {
  return hook::pattern("83 79 44 00 74 2E").count(1).get(0).get<void>(0);
});


struct TDynArray {
  uint64_t* base_pointer;
  uint32_t count;
  uint32_t max;
  uint64_t begin;
};

static hook::thiscall_stub<int(CEnum*, CName&, int&)> CEnum_FindValue([]() {
  return hook::pattern("83 79 24 00 74 32").count(1).get(0).get<void>(0);
});

static hook::thiscall_stub<bool(CEnum*, int, CName&)> CEnum_FindName([]() {
  return hook::pattern("83 79 54 00 44 8B D2 ").count(1).get(0).get<void>(0);
});

struct CEnum {
  uint64_t vtable;
  CName name;
  DWORD N27E745C8; //0x000C 
  void* names; //0x0010 
  DWORD names_count; //0x0018 
  char _0x001C[4];
  DWORD value_count; //0x0020 
  DWORD N27EF6229; //0x0024 
  char _0x0028[24];
  void* values; //0x0040 
  char _0x0048[56];
  DWORD flags; //0x0080 
  BYTE unknown; //0x0084 
  char _0x0085[3];

  bool FindName(int index, CName& name) { return CEnum_FindName(this, index, name); }
  int FindValue(CName& name, int& out) { return CEnum_FindValue(this, name, out); }
};

class CGame;

static hook::thiscall_stub<void(CGame*, bool toggle)> CGame_EnableFreeCamera([]() {
  return hook::pattern("40 53 48 83 EC 40 48 8B D9 E8 ? ? ? ? 80 BB ? ? ? ? ?").count(1).get(0).get<void>(0);
});

static hook::thiscall_stub<bool(CGame*, EInputKey, EInputAction, float)> CGame_ProcessFreeCameraInput([]() {
  return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 80 3D").count(1).get(0).get<void>(0);
});

class CGame {
public:
  void EnableFreeCamera(bool toggle) { CGame_EnableFreeCamera(this, toggle); }
  bool ProcessFreeCameraInput(EInputKey key, EInputAction action, float tick) { return CGame_ProcessFreeCameraInput(this, key, action, tick); }
};

CGame** global_game = nullptr;

static hook::thiscall_stub<void(CRTTISystem*, TDynArray&)> CRTTISystem_EnumFunctions([]() {
  return hook::pattern("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 55 41 56 41 57 48 83 EC 30 33 FF").count(1).get(0).get<void>(0);
});

static hook::thiscall_stub<void(CRTTISystem*, TDynArray&)> CRTTISystem_EnumEnums([]() {
  return hook::pattern("40 53 41 56 41 57 48 83 EC 30 33 DB 4C 8B F2 4C 8B F9 39 59 14").count(1).get(0).get<void>(0);
});

class CRTTISystem {
public:

  CFunction* FindGlobalFunction(CName& name) { return CRTTISystem_FindGlobalFunction(this, name); }
  void EnumFunctions(TDynArray& array) { CRTTISystem_EnumFunctions(this, array); }
  void EnumEnums(TDynArray& array) { CRTTISystem_EnumEnums(this, array); }

  void DumpEnums() {
    std::wofstream log_file;
    log_file.open("global_enums_log.txt");

    TDynArray enums;
    enums.base_pointer = &enums.begin;
    enums.count = 0;
    enums.max = 0;
    EnumEnums(enums);

    for (size_t i = 0; i < enums.count; i++) {
      auto enum_ = *(CEnum**)(enums.base_pointer + i);
      if (!enum_) continue;

      log_file << enum_->name.AsChar() << " " << std::dec << enum_->names_count << " " << std::dec << enum_->flags << " " << enum_->unknown << std::endl;
      for (size_t i = 0; i < enum_->names_count; i++) {
        CName name;
        enum_->FindName(i, name);
        if (name.pool_index) {
          int value;
          enum_->FindValue(name, value);
          log_file << " " << name.AsChar() << " " << value << std::endl;
        }
      }
    }

    log_file.close();
  }

  void DumpGlobalFunctions() {
    std::wofstream log_file;
    log_file.open("global_functions_log.txt");

    TDynArray functions;
    functions.base_pointer = &functions.begin;
    functions.count = 0;
    functions.max = 0;
    EnumFunctions(functions);

    for (size_t i = 0; i < functions.count; i++) {
      auto function = *(CFunction**)(functions.base_pointer + i);
      if (!function) continue; // || !(function->flags & (1 << 8))) continue;

      log_file << function->name.AsChar() << " " << std::dec << function->flags << std::endl;
      for (size_t i = 0; i < function->argument_count; i++) {
        auto property = (CProperty*)function->property_array[i];
        log_file << " " << property->class_type->GetName().AsChar() << " " << property->name.AsChar() << " " << property->optional << std::endl;
      }
    }

    log_file.close();
  }
};

class CRTTISerializer;
class TString;

static hook::thiscall_stub<void(CRTTISerializer*)> CRTTISerializer_Constructor([]() {
  return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 33 F6 48 8B D9 48 83 C1 28 48 89 71 D8 48 89 71 E0 48 89 71 E8 48 89 71 F0 48 89 71 F8").count(1).get(0).get<void>(0);
});

static hook::thiscall_stub<bool(CRTTISerializer*, TString*, bool)> CRTTISerializer_LoadScriptDataFromFile([]() {
  return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 4C 89 74 24 ? 55 48 8B EC 48 81 EC ? ? ? ? 48 8B F1").count(1).get(0).get<void>(0);
});

class TString {
public:
  wchar_t* buffer_address;
  uint32_t length;
  uint32_t max;
  wchar_t buffer[512];

  TString(wchar_t* string) {
    buffer_address = buffer;
    wcscpy_s(buffer, 512, string);
    length = wcslen(string);
    max = 512;
  }
};

class CRTTISerializer {
  char _0x00[1000];
public:
  CRTTISerializer() {
    CRTTISerializer_Constructor(this);
  }

  bool LoadScriptDataFromFile(TString* name, bool validate) { return CRTTISerializer_LoadScriptDataFromFile(this, name, validate); }
};

typedef bool(*OnViewportInputType) (void* thisptr, void* viewport, EInputKey input_key, EInputAction input_action, float tick);
OnViewportInputType OnViewportInputDebugConsole = nullptr;

bool OnViewportInputDebugAlwaysHook(void* thisptr, void* viewport, EInputKey input_key, EInputAction input_action, float tick) {
  if ((*global_game)->ProcessFreeCameraInput(input_key, input_action, tick)) return true;

  if (input_key == IK_F2 && input_action == IACT_Release) {
    input_key = IK_Tilde;
    input_action = IACT_Press;
  }

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

static void ScriptWarn(void* thisptr, void* file_context, TString* message) {
  std::wofstream log_file;
  log_file.open("script_compilation.log", std::ios_base::app);
  log_file << "warn: " << message->buffer_address << std::endl;
  log_file.close();
}

static void ScriptError(void* thisptr, void* file_context, TString* message) {
  std::wofstream log_file;
  log_file.open("script_compilation.log", std::ios_base::app);
  log_file << "error: " << message->buffer_address << std::endl;
  log_file.close();
}

DWORD WINAPI InitializeHook(void* arguments) {
  hook::set_base();
  HookFunction::RunAll();

  //char* location = hook::pattern("48 8D 35 ? ? ? ? 48 8D 54 24 ?").count(1).get(0).get<char>(3);
  //void* compilation_messages = reinterpret_cast<void*>(location + *(int32_t*)location + 4);

  //DWORD old_protect;

  //if (VirtualProtect((void*)((uint64_t)compilation_messages + 2 * 8), 16, PAGE_EXECUTE_READWRITE, &old_protect)) {
  //  *(uint64_t*)((uint64_t)compilation_messages + 2 * 8) = (uint64_t)&ScriptWarn;
  //  *(uint64_t*)((uint64_t)compilation_messages + 3 * 8) = (uint64_t)&ScriptError;
  //}

  char* location = hook::pattern("48 8B 05 ? ? ? ? 48 8D 4C 24 ? C6 44 24").count(1).get(0).get<char>(3);
  global_game = reinterpret_cast<CGame**>(location + *(int32_t*)location + 4);

  location = hook::pattern("48 89 05 ? ? ? ? EB 07 48 89 35 ? ? ? ? 48 8B 47 60").count(1).get(0).get<char>(3);
  global_debug_console = reinterpret_cast<void**>(location + *(int32_t*)location + 4);

  while (*global_game == nullptr || *global_debug_console == nullptr) {
    Sleep(500);
  }

  location = hook::pattern("48 83 EC 28 48 8B 05 ? ? ? ? 0F B6 90 ? ? ? ?").count(1).get(0).get<char>(0);
  OnViewportInputDebugConsole = (OnViewportInputType)location;

  location = hook::pattern("48 8B 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 30").count(1).get(0).get<char>(3);
  rtti_system = *reinterpret_cast<CRTTISystem**>(location + *(int32_t*)location + 4);

  location = hook::pattern("4C 8D 0D ? ? ? ? 49 89 14 C1").count(1).get(0).get<char>(3);
  native_globals_function_map = reinterpret_cast<void*>(location + *(int32_t*)location + 4);

  game_hook = new utils::VtableHook(*global_game);
  game_hook->HookMethod(OnViewportInputDebugAlwaysHook, 128);


  //CRTTISerializer serializer;
  //TString path(L"x64.final.redscripts ");

  //if(serializer.LoadScriptDataFromFile(&path, false)) {
  //  OutputDebugStringW(L"Load custom script file");
  //};

  //rtti_system->DumpGlobalFunctions();

  //rtti_system->DumpEnums();

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