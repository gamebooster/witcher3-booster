#pragma once

#include "hooking/Hooking.h"
#include "witcher3-enums.h"

class CRTTISystem;
struct CFunction;
struct CName;
struct CNameHash;
struct CEnum;
class CRTTISerializer;
class TString;
class FileManager;
class File;
class CGame;

enum {  // Sir_Kane
  PropFlag_Editable = 0x1,
  PropFlag_Const = 0x2,
  PropFlag_Inlined = 0x4,  // 0x80
  PropFlag_Serializable = 0x8,
  // 0x10 = ?
  PropFlag_IsScripted = 0x20,
  PropFlag_IsParam = 0x80,
  PropFlag_IsLocal = 0x100,
  PropFlag_Out = 0x200,
  PropFlag_Optional = 0x400,
  PropFlag_Compatible = 0x2000,  // IsExported
  PropFlag_Import = 0x4000,      // Native
  PropFlag_Saved = 0x8000,
  PropFlag_Private = 0x10000,
  PropFlag_Protected = 0x20000,
  PropFlag_Public = 0x40000,
  PropFlag_AutoBindable = 0x80000,
  // PropFlag_IsSerializable = 0x10,// ?
};

enum {  // Sir_Kane
  FuncFlag_Import = 0x1,
  FuncFlag_Global = 0x2,  // Should be right, only seen it on global functions
  FuncFlag_Final = 0x10,
  FuncFlag_Event = 0x20,
  FuncFlag_Latent = 0x40,
  FuncFlag_Entry = 0x80,
  FuncFlag_Exec = 0x100,
  FuncFlag_Timer = 0x400,
  FuncFlag_Scene = 0x800,
  FuncFlag_Quest = 0x1000,
  FuncFlag_Cleanup = 0x2000,
  FuncFlag_Private = 0x4000,
  FuncFlag_Protected = 0x8000,
  FuncFlag_Public = 0x10000,
  FuncFlag_Reward = 0x20000,
  //
  FuncFlag_Exported = 0x8,
};

enum {  // Sir_Kane
  ClassFlag_Abstract = 0x1,
  ClassFlag_Import = 0x2,
  //
  ClassFlag_Scripted = 0x4,
  ClassFlag_Exported = 0x8,
  ClassFlag_HasStateOwner = 0x10,
  ClassFlag_Incomplete = 0x200,
  ClassFlag_Statemachine = 0x400,
};

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

// static hook::thiscall_stub<CName*(CName*, CNameHash)>
// CName_CreateNameFromHash([]() {
//  return hook::pattern("48 89 5C 24 ? 57 48 83 EC 20 8B DA 48 8B F9 85 D2 74
//  37").count(1).get(0).get<void*>(0);
//});
//
//
// static hook::thiscall_stub<const wchar_t*(CName*)> CName_AsChar([]() {
//  return hook::pattern("40 53 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B D9 48 85 C0
//  75 05 E8 ? ? ? ? 8B 13 48 8B 0D ? ? ? ? E8").count(1).get(0).get<void*>(0);
//});

// struct CName {
//  uint32_t pool_index;
//  CName() { pool_index = 0; };
//  CName(CNameHash hash) {
//    CName_CreateNameFromHash(this, hash);
//  }
//
//  const wchar_t* AsChar() {
//    return CName_AsChar(this);
//  }
//};

// static hook::thiscall_stub<int(CEnum*, CName&, int&)> CEnum_FindValue([]() {
//  return hook::pattern("83 79 24 00 74 32").count(1).get(0).get<void*>(0);
//});
//
// static hook::thiscall_stub<bool(CEnum*, int, CName&)> CEnum_FindName([]() {
//  return hook::pattern("83 79 54 00 44 8B D2 ").count(1).get(0).get<void*>(0);
//});

// struct CEnum {
//  uint64_t vtable;
//  CName name;
//  DWORD N27E745C8; //0x000C
//  void* names; //0x0010
//  DWORD names_count; //0x0018
//  char _0x001C[4];
//  DWORD value_count; //0x0020
//  DWORD N27EF6229; //0x0024
//  char _0x0028[24];
//  void* values; //0x0040
//  char _0x0048[56];
//  DWORD flags; //0x0080
//  bool source_script; //0x0084
//  char _0x0085[3];
//
//  bool FindName(int index, CName& name) { return CEnum_FindName(this, index,
//  name); }
//  int FindValue(CName& name, int& out) { return CEnum_FindValue(this, name,
//  out); }
//};
//
// struct CClass {
//  virtual void Deconstructor();
//  virtual CName& GetName();
//
//  uint64_t unknown;
//  CClass* parent;
//  char _0x0018[24];
//  uint64_t* property_array;
//  uint32_t property_count;//0x0038
//};
//
// struct CProperty {
//  uint64_t vtable;
//  CClass* class_type;
//  CName name; //0x0010
//  char _0x0014[4];
//  uint64_t N23BE7CFD; //0x0018
//  uint32_t N23BE7CFE; //0x0020
//  uint32_t flags; //0x0024
//  uint64_t N23BE7CFF; //0x0028
//};
//
// struct CFunction {
//  uint64_t vtable;
//  CClass* parent_class;
//  CName name;
//  uint32_t flags;
//  CProperty* return_type;
//  uint64_t* property_array;
//  uint32_t argument_count;
//  uint32_t align_0;
//  uint64_t unknown_4;
//  uint64_t unknown_5;
//  uint64_t unknown_6;
//  uint64_t unknown_7;
//  uint64_t unknown_8;
//  uint64_t unknown_9;
//  uint32_t unknown_10;
//  uint32_t align_1;
//  char _CScriptCompiledCode[80];
//  uint32_t registration_offset; // 0xB8
//  char _0x00BC[12];
//  uint64_t unknown_11;
//};
//
// static_assert(sizeof(CFunction) == 208, "CFunction has wrong size!");
//
// struct TDynArray {
//  uint64_t* base_pointer;
//  uint32_t count;
//  uint32_t max;
//  uint64_t begin;
//};

// static hook::thiscall_stub<void(TString*, const wchar_t*)>
// TString_Constructor([]() {
//  return hook::pattern("E8 ? ? ? ? 48 8D 54 24 ? 48 8D 4C 24 ? 41 83 CF
//  01").count(1).get(0).extract<void*>(1);
//});
//
// static hook::thiscall_stub<void(TString*)> TString_Deconstructor([]() {
//  return hook::pattern("E8 ? ? ? ? 48 89 9C 24 ? ? ? ? 48 89 BC
//  24").count(1).get(0).extract<void*>(1);
//});

// class TString {
// public:
//  wchar_t* buffer_address;
//  uint32_t length;
//  uint32_t max;
//
//  TString(const wchar_t* string) {
//    TString_Constructor(this, string);
//  }
//
//  ~TString() {
//    TString_Deconstructor(this);
//  }
//};

// static hook::thiscall_stub<void(CGame*, bool toggle)>
// CGame_EnableFreeCamera([]() {
//  return hook::pattern("40 53 48 83 EC 40 48 8B D9 E8 ? ? ? ? 80 BB ? ? ? ?
//  ?").count(1).get(0).get<void>(0);
//});

static hook::thiscall_stub<bool(CGame*, EInputKey, EInputAction, float)>
    CGame_ProcessFreeCameraInput([]() {
      return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 80 3D")
          .count(1)
          .get(0)
          .get<void*>(0);
    });

class CGame {
 public:
  // void EnableFreeCamera(bool toggle) { CGame_EnableFreeCamera(this, toggle);
  // }
  bool ProcessFreeCameraInput(EInputKey key, EInputAction action, float tick) {
    return CGame_ProcessFreeCameraInput(this, key, action, tick);
  }
};

// static hook::thiscall_stub<CFunction*(CRTTISystem*, CName&)>
// CRTTISystem_FindGlobalFunction([]() {
//  return hook::pattern("83 79 44 00 74 2E").count(1).get(0).get<void*>(0);
//});
//
// static hook::thiscall_stub<void(CRTTISystem*, TDynArray&)>
// CRTTISystem_EnumFunctions([]() {
//  return hook::pattern("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 55 41 56 41 57 48
//  83 EC 30 33 FF").count(1).get(0).get<void*>(0);
//});
//
// static hook::thiscall_stub<void(CRTTISystem*, TDynArray&)>
// CRTTISystem_EnumEnums([]() {
//  return hook::pattern("40 53 41 56 41 57 48 83 EC 30 33 DB 4C 8B F2 4C 8B F9
//  39 59 14").count(1).get(0).get<void*>(0);
//});

// class CRTTISystem {
// public:
//
//  CFunction* FindGlobalFunction(CName& name) { return
//  CRTTISystem_FindGlobalFunction(this, name); }
//  void EnumFunctions(TDynArray& array) { CRTTISystem_EnumFunctions(this,
//  array); }
//  void EnumEnums(TDynArray& array) { CRTTISystem_EnumEnums(this, array); }
//};
//
// static hook::thiscall_stub<void(CRTTISerializer*)>
// CRTTISerializer_Constructor([]() {
//  return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 33 F6 48 8B
//  D9 48 83 C1 28 48 89 71 D8 48 89 71 E0 48 89 71 E8 48 89 71 F0 48 89 71
//  F8").count(1).get(0).get<void*>(0);
//});
//
// static hook::thiscall_stub<void(CRTTISerializer*)>
// CRTTISerializer_Deconstructor([]() {
//  char* location = hook::pattern("E8 ? ? ? ? 48 8D 4C 24 ? E8 ? ? ? ? 48 89 9C
//  24").count(1).get(0).get<char*>(1);
//  return reinterpret_cast<void*>(location + *(int32_t*)location + 4);
//});
//
// static hook::thiscall_stub<bool(CRTTISerializer*, TString*, bool)>
// CRTTISerializer_LoadScriptData([]() {
//  return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 4C 89 74 24
//  ? 55 48 8B EC 48 81 EC ? ? ? ? 48 8B F1").count(1).get(0).get<void*>(0);
//});

// static hook::thiscall_stub<bool(CRTTISerializer*, File*)>
// CRTTISerializer_LoadScriptDataFromFile([]() {
//  return hook::pattern("40 53 55 56 57 41 54 41 57 48 83 EC
//  68").count(1).get(0).get<void>(0);
//});

// class CRTTISerializer {
//  char _0x00[1000];
// public:
//  CRTTISerializer() {
//    CRTTISerializer_Constructor(this);
//  }
//
//  ~CRTTISerializer() {
//    CRTTISerializer_Deconstructor(this);
//  }
//
//  bool LoadScriptData(TString* name, bool validate) { return
//  CRTTISerializer_LoadScriptData(this, name, validate); }
//  bool LoadScriptDataFromFile(File* file) { return
//  CRTTISerializer_LoadScriptDataFromFile(this, file); }
//};

// class CScriptsSerializer;
//
// static hook::thiscall_stub<void(CScriptsSerializer*)>
// CScriptsSerializer_Constructor([]() {
//  return hook::pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 33 F6 48 8B
//  D9 48 83 C1 18 48 89 71 E8 48 89 71 F0 48 89 71 F8 E8 ? ? ? ? 48 89 73 30 40
//  88 73 38 39 73 10 74 32 48 8B 05 ? ? ? ? 48 8B 7B
//  18").count(1).get(0).get<void>(0);
//});
//
// static hook::thiscall_stub<bool(CScriptsSerializer*, TString*)>
// CScriptsSerializer_LoadScript([]() {
//  return hook::pattern("48 89 5C 24 ? 57 48 83 EC 70 48 8B F9 48 8B 0D ? ? ? ?
//  33 DB").count(1).get(0).get<void>(0);
//});
//
// static hook::thiscall_stub<bool(CScriptsSerializer*, void*)>
// CScriptsSerializer_LoadScriptFile([]() {
//  return hook::pattern("40 53 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 58 48
//  8B 02").count(1).get(0).get<void>(0);
//});
//
// class CScriptsSerializer {
//  char _0x00[1000];
// public:
//  CScriptsSerializer() {
//    CScriptsSerializer_Constructor(this);
//  }
//
//  bool LoadScriptDataFromFile(TString* name) { return
//  CScriptsSerializer_LoadScript(this, name); }
//  bool LoadScriptFile(void* file) { return
//  CScriptsSerializer_LoadScriptFile(this, file); }
//};
//
// static hook::thiscall_stub<File*(FileManager*, TString const& name, uint64_t,
// uint64_t)> FileManager_CreateFileReader([]() {
//  char* location = hook::pattern("E8 ? ? ? ? 48 8B F8 48 85 C0 74 32 4C 8B
//  C6").count(1).get(0).get<char>(1);
//  return reinterpret_cast<void*>(location + *(int32_t*)location + 4);
//});

// class FileManager {
// public:
//  File* CreateFileReader(TString const& name, uint64_t flag1, uint64_t flag2)
//  { return FileManager_CreateFileReader(this, name, flag1, flag2); }
//};

// struct CScriptFileContext {
//  TString file_name;
//  uint32_t line_number;
//};
