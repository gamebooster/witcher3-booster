#include <windows.h>
#include <fstream>

#include "vtable/vmthooks.h"

#include "witcher3-classes.h"

#include <iostream>

HANDLE thread = nullptr;
utils::VtableHook* game_hook = nullptr;
void** global_debug_console = nullptr;
CRTTISystem* rtti_system = nullptr;
void* native_globals_function_map = nullptr;
CGame** global_game = nullptr;

// void DumpEnums() {
//  std::wofstream log_file;
//  log_file.open("global_enums_log.txt");
//
//  TDynArray enums;
//  enums.base_pointer = &enums.begin;
//  enums.count = 0;
//  enums.max = 0;
//  rtti_system->EnumEnums(enums);
//
//  for (size_t i = 0; i < enums.count; i++) {
//    auto enum_ = *(CEnum**)(enums.base_pointer + i);
//    if (!enum_) continue; //|| !enum_->source_script) continue;
//
//    log_file << "enum " << enum_->name.AsChar() << " // " << std::dec <<
//    enum_->names_count << " " << std::dec << enum_->flags << " " <<
//    enum_->source_script << std::endl;
//    log_file << "{" << std::endl;
//    for (size_t i = 0; i < enum_->names_count; i++) {
//      CName name;
//      enum_->FindName(i, name);
//      if (name.pool_index) {
//        int value;
//        enum_->FindValue(name, value);
//        log_file << " " << name.AsChar() << " = " << value << "," <<
//        std::endl;
//      }
//    }
//    log_file << std::endl << "}" << std::endl;
//  }
//
//  log_file.close();
//}
//
// std::wostream& operator<<(std::wostream& stream, const wchar_t* string) {
//  if (string == nullptr) return stream;
//
//  std::wstring test(string);
//
//  size_t pos = test.find(L"handle:");
//  if (pos != -1) {
//    test.replace(pos, std::wstring(L"handle:").length(), L"");
//  }
//
//  pos = test.find(L"ptr:");
//  if (pos != -1) {
//    test.replace(pos, std::wstring(L"ptr:").length(), L"*");
//  }
//
//  pos = test.find(L"array:2,0,array:2,0,");
//  if (pos != -1) {
//    test.replace(pos, std::wstring(L"array:2,0,array:2,0,").length(),
//    L"C2dArray");
//  }
//
//  pos = test.find(L"array:2,0,");
//  if (pos != -1) {
//    test.replace(pos, std::wstring(L"array:2,0,").length(), L"array<");
//    test += L">";
//  }
//
//  stream << test;
//  return stream;
//}
//
// void DumpGlobalFunctions() {
//  std::wofstream log_file;
//  log_file.open("global_functions_log.txt");
//
//  TDynArray functions;
//  functions.base_pointer = &functions.begin;
//  functions.count = 0;
//  functions.max = 0;
//  rtti_system->EnumFunctions(functions);
//
//  CName* last_class = nullptr;
//
//  for (size_t i = 0; i < functions.count; i++) {
//    auto function = *(CFunction**)(functions.base_pointer + i);
//    if (!function) continue; // || !(function->flags & (1 << 8))) continue;
//
//    if (last_class && (!function->parent_class ||
//    (function->parent_class->GetName().pool_index != last_class->pool_index)))
//    {
//      log_file << std::endl << "}" << std::endl;
//      last_class = nullptr;
//    }
//
//    if (function->parent_class && !last_class) {
//      last_class = &function->parent_class->GetName();
//      log_file << "import class " << last_class->AsChar();
//      if (function->parent_class->parent) {
//        log_file << " extends " <<
//        function->parent_class->parent->GetName().AsChar();
//      }
//      log_file << " {" << std::endl;
//
//      for (size_t i = 0; i < function->parent_class->property_count; i++) {
//        auto property = (CProperty*)function->parent_class->property_array[i];
//        //if (!(property->flags & PropFlag_Compatible)) continue;
//        if (!(property->flags & PropFlag_Import)) continue;
//
//        log_file << "  import var " << property->name.AsChar() << " : " <<
//        property->class_type->GetName().AsChar() << ";" << " // " <<
//        property->flags << std::endl;
//      }
//      log_file << std::endl;
//
//    }
//
//    if (function->parent_class) log_file << "  ";
//
//    if (function->flags & FuncFlag_Exported) {
//      log_file << "import ";
//    }
//
//    if (function->flags & FuncFlag_Public) {
//      log_file << "public ";
//    }
//
//    if (function->flags & FuncFlag_Private) {
//      log_file << "private ";
//    }
//
//    if (function->flags & FuncFlag_Protected) {
//      log_file << "protected ";
//    }
//
//    if (function->flags & FuncFlag_Event) {
//      log_file << "event ";
//    }
//
//    if (function->flags & FuncFlag_Latent) {
//      log_file << "latent ";
//    }
//
//    if (function->flags & FuncFlag_Cleanup) {
//      log_file << "cleanup ";
//    }
//
//    if (function->flags & FuncFlag_Final) {
//      log_file << "final ";
//    }
//
//    log_file << "function " << function->name.AsChar() << "(";
//    for (size_t i = 0; i < function->argument_count; i++) {
//      auto property = (CProperty*)function->property_array[i];
//
//      if (property->flags & PropFlag_Optional) {
//        log_file << " optional ";
//      }
//
//      if (property->flags & PropFlag_Out) {
//        log_file << " out ";
//      }
//
//      log_file  << " " << property->name.AsChar() << " : " <<
//      property->class_type->GetName().AsChar();
//      if (i + 1 != function->argument_count)
//      {
//        log_file << ", ";
//      } else {
//      }
//    }
//    log_file << ")";
//    if (function->return_type) {
//      log_file << " : " <<
//      function->return_type->class_type->GetName().AsChar();
//    }
//    log_file << "; " << " // flags: " << std::dec << function->flags << " "<<
//    std::hex << function << std::endl;
//  }
//
//
//
//  log_file.close();
//}
//
typedef bool (*OnViewportInputType)(void* thisptr,
                                    void* viewport,
                                    EInputKey input_key,
                                    EInputAction input_action,
                                    float tick);
OnViewportInputType OnViewportInputDebugConsole = nullptr;

bool OnViewportInputDebugAlwaysHook(void* thisptr,
                                    void* viewport,
                                    EInputKey input_key,
                                    EInputAction input_action,
                                    float tick) {
  if ((*global_game)->ProcessFreeCameraInput(input_key, input_action, tick))
    return true;

  if (input_key == IK_F2 && input_action == IACT_Release) {
    input_key = IK_Tilde;
    input_action = IACT_Press;
  }

  if (input_key == IK_Backslash && input_action == IACT_Release) {
	  input_key = IK_Tilde;
	  input_action = IACT_Press;
  }

  return OnViewportInputDebugConsole(*global_debug_console, viewport, input_key,
                                     input_action, tick);
}
//
// void funcLogHook(struct CObject *a1, struct CScriptStackFrame *a2, void *a3)
// {
//  OutputDebugStringW(L"test log");
//}
//
// uint64_t* GetFunctionAddress(CFunction* function) {
//  return (uint64_t*)((uint64_t)native_globals_function_map + (8 *
//  function->registration_offset));
//}
//
// void ReplaceFunction(const char* name, uint64_t address) {
//  auto function = rtti_system->FindGlobalFunction(CName(name));
//
//  if (function) {
//    auto function_address = GetFunctionAddress(function);
//
//    std::wstringstream message;
//    message << "ReplaceFunction: " << name << " " << std::hex << function << "
//    " << " " << std::hex << function_address;
//    OutputDebugStringW(message.str().c_str());
//
//    *function_address = address;
//  }
//}
//
// static void ScriptWarn(void* thisptr, CScriptFileContext* file_context,
// TString* message) {
//  std::wofstream log_file;
//  log_file.open("script_compilation.log", std::ios_base::app);
//  log_file << "warn " << file_context->file_name.buffer_address << ":" <<
//  file_context->line_number << " " << message->buffer_address << std::endl;
//  log_file.close();
//
//  std::wcout << "warn " << file_context->file_name.buffer_address << ":" <<
//  file_context->line_number << " " << message->buffer_address << std::endl;
//}
//
// static void ScriptError(void* thisptr, CScriptFileContext* file_context,
// TString* message) {
//  std::wofstream log_file;
//  log_file.open("script_compilation.log", std::ios_base::app);
//  log_file << "ERROR " << file_context->file_name.buffer_address << ":" <<
//  file_context->line_number << " " << message->buffer_address << std::endl;
//  log_file.close();
//
//  std::wcerr << "ERROR " << file_context->file_name.buffer_address << ":" <<
//  file_context->line_number << " " << message->buffer_address << std::endl;
//}

std::wstring GetExecutablePath() {
  wchar_t buffer[MAX_PATH];
  GetModuleFileName(NULL, buffer, MAX_PATH);
  std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
  return std::wstring(buffer).substr(0, pos);
}

std::vector<std::wstring> GetAllFileNamesFromFolder(std::wstring folder) {
  std::vector<std::wstring> names;
  wchar_t search_path[200];
  wsprintf(search_path, L"%s*.*", folder.c_str());
  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile(search_path, &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        names.push_back(fd.cFileName);
      }
    } while (::FindNextFile(hFind, &fd));
    ::FindClose(hFind);
  }
  return names;
}

static bool(__thiscall* OrginalBaseEngine_InitializeScripts)(void*);

static bool BaseEngine_InitializeScripts(void* rcx, void* rdx) {
  std::wcout << "BaseEngine_InitializeScripts" << std::endl;

  // CRTTISerializer serializer;
  // TString path(L"cookedfinal.redscripts");

  // if (serializer.LoadScriptData(&path, false)) {
  //  std::wcout << L"Loaded scriptplugin: cookedfinal.redscripts" << std::endl;
  //};

  return OrginalBaseEngine_InitializeScripts(rcx);
}

DWORD WINAPI InitializeHook(void* arguments) {
  hook::set_base();
  HookFunction::RunAll();

  global_game = hook::pattern("48 8B 05 ? ? ? 01 C6 44 24 30 01 89 4C 24 28")
                    .count(1)
                    .get(0)
                    .extract<CGame**>(3);
  global_debug_console =
      hook::pattern("48 89 05 ? ? ? ? EB 07 48 89 35 ? ? ? ? 48 8B")
          .count(1)
          .get(0)
          .extract<void**>(3);

  while (*global_game == nullptr || *global_debug_console == nullptr) {
    Sleep(500);
  }

  OnViewportInputDebugConsole =
      hook::pattern("48 83 EC 28 48 8B 05 ? ? ? ? 0F B6 90")
          .count(1)
          .get(0)
          .get<OnViewportInputType>(0);
  rtti_system = *hook::pattern("48 8B 0D ? ? ? ? 48 8B 5C 24 ? 48 83 C4 30")
                     .count(1)
                     .get(0)
                     .extract<CRTTISystem**>(3);
  // native_globals_function_map = hook::pattern("4C 8D 0D ? ? ? ? 49 89 14
  // C1").count(1).get(0).extract<void*>(3);
  // FileManager* file_manager  = hook::pattern("48 8B 0D ? ? ? ? 48 8D 55 A0 41
  // B8").count(1).get(0).extract<FileManager*>(3);

  game_hook = new utils::VtableHook(*global_game);
  game_hook->HookMethod(OnViewportInputDebugAlwaysHook, 136);

  // auto exe_path = GetExecutablePath();
  // exe_path += L"\\scriptplugins\\";

  // auto plugin_names = GetAllFileNamesFromFolder(exe_path);

  // for (auto name : plugin_names) {
  //  if (name.find(L".redscriptsplugin") == std::string::npos) continue;

  //  auto full_path = (L"..\\..\\bin\\x64\\scriptplugins\\" + name);
  //  CRTTISerializer serializer;
  //  TString path(full_path.c_str());

  //  if (serializer.LoadScriptData(&path, false)) {
  //    OutputDebugStringW((L"Loaded scriptplugin: " + name).c_str());
  //  };
  //}

  // DumpGlobalFunctions();

  // DumpEnums();

  // ReplaceFunction("Log", (uint64_t)&funcLogHook);
  // ReplaceFunction("LogChannel", (uint64_t)&funcLogHook);
  // ReplaceFunction("Trace", (uint64_t)&funcLogHook);
  // ReplaceFunction("DebugBreak", (uint64_t)&funcLogHook);

  return 1;
}

void FinalizeHook() {
  if (game_hook)
    delete game_hook;
}

int WINAPI DllMain(HINSTANCE instance, DWORD reason, PVOID reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    thread = CreateThread(nullptr, 0, InitializeHook, 0, 0, nullptr);
  } else if (reason == DLL_PROCESS_DETACH) {
    FinalizeHook();
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
  }
  return 1;
}
