#include <windows.h>

#include "vtable/vmthooks.h"
#include "hooking/Hooking.h"

HANDLE thread = nullptr;
utils::VtableHook* game_hook = nullptr;
void** global_debug_console = nullptr;

typedef bool(*OnViewportInputType) (void* thisptr, void* viewport, int input_key, int input_action, float tick);
OnViewportInputType OnViewportInputDebugConsole = nullptr;

bool OnViewportInputDebugAlwaysHook(void* thisptr, void* viewport, int input_key, int input_action, float tick) {
  return OnViewportInputDebugConsole(*global_debug_console, viewport, input_key, input_action, tick);
}

DWORD WINAPI InitializeHook(void* arguments) {
	hook::set_base();

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

	OutputDebugStringW(L"Hook game");

	game_hook = new utils::VtableHook(*global_game);
	game_hook->HookMethod(OnViewportInputDebugAlwaysHook, 128);

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