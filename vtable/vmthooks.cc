// Copyright 2013 Karl Skomski - GPL v3
#include "vmthooks.h"

namespace utils {
uint CountFuncs(void** vmt) {
  // Query memory and as long as the pointer lands in
  // PAGE_EXECUTE_READ or PAGE_EXECUTE_READWRITE memory
  // assume this is a valid function pointer.
  MEMORY_BASIC_INFORMATION mem;
  int i = -1;
  do {
    i++;
    VirtualQuery(vmt[i], &mem, sizeof(MEMORY_BASIC_INFORMATION));
  } while (mem.Protect == PAGE_EXECUTE_READ ||
           mem.Protect == PAGE_EXECUTE_READWRITE);
  return i;
}
uint CountFuncs(void* begin, void* end, void** vmt) {
  int i = -1;
  do {
    i++;
  } while (begin < vmt[i] && vmt[i] < end);
  return i;
}
int FindFunc(void** vmt, void* func, uint vfuncs) {
  if (!vfuncs)
    vfuncs = CountFuncs(vmt);
  for (uint i = 0; i < vfuncs; i++) {
    if (vmt[i] == func) {
      return i;
    }
  }
  return -1;
}

VtableHook::VtableHook(void* inst, size_t offset, uint vfuncs) {
  vtable_ = make_ptr<void***>(inst, offset);
  old_vtable_ = *vtable_;
  // Count vfuncs ourself if needed
  if (!vfuncs)
    vfuncs = CountFuncs(old_vtable_);
  vtable_function_count_ = vfuncs;

  _array = reinterpret_cast<void**>(malloc((vfuncs + 3) * sizeof(void*)));
  // Initialize RTTI pointer (should there be one)
  _array[2] = old_vtable_[-1];
  // Copy over the other vfuncs
  for (uint i = 0; i < vfuncs; ++i)
    _array[i + 3] = old_vtable_[i];
  // Hook it
  *vtable_ = _array + 3;
}
VtableHook::~VtableHook() {
  if (vtable_)
    Unhook();
  free(_array);
}

}  // namespace utils
