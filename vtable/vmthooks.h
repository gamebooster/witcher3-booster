// Copyright 2013 Karl Skomski - GPL v3
#pragma once

#include <windows.h>
#include <cassert>

namespace utils {
typedef unsigned int uint;
typedef size_t int_ptr;
template <typename T>
inline T make_ptr(void* ptr, int_ptr offset) {
  return reinterpret_cast<T>((size_t)ptr + offset);
}
inline void**& getvtable(void* inst, int_ptr offset = 0) {
  return *reinterpret_cast<void***>((size_t)inst + offset);
}

// Find the number of vfuncs in a vtable
uint CountFuncs(void** pVMT);
uint CountFuncs(void* begin, void* end, void** pVMT);

// Find the index for a vfunc, result is negative if not found
int FindFunc(void** pVMT, void* pFunc, uint vfuncs = 0);

// ----------------------------------------------------------------
// Class: VMTManager
// ----------------------------------------------------------------
// Hooks virtual functions by replacing the vtable pointer from an instance.
class VtableHook {
  // Forbid copy constructing and assignment.
  VtableHook(const VtableHook&);
  VtableHook& operator=(const VtableHook&);

 public:
  VtableHook(void* inst, size_t offset = 0, uint vfuncs = 0);
  ~VtableHook();

  // Hooks a function by index.
  inline void HookMethod(void* newfunc, size_t index) {
    assert(index < vtable_function_count_);
    _array[index + 3] = newfunc;
  }
  // Unhooks a function by index.
  inline void UnhookMethod(size_t index) {
    assert(index < vtable_function_count_);
    _array[index + 3] = old_vtable_[index];
  }

  // Manage the hooks.
  inline void Unhook() { *vtable_ = old_vtable_; }
  inline void Rehook() { *vtable_ = _array + 3; }
  inline bool Hooked() const { return *vtable_ != old_vtable_; }
  inline void EraseHooks() {
    for (uint i = 0; i < vtable_function_count_; ++i) {
      _array[i + 3] = vtable_[i];
    }
  }
  inline uint NumFuncs() const { return vtable_function_count_; }

  // If the instance is somehow destroyed
  // before you get a chance to unhook it
  // or destruct this hook object, call this.
  // It'll prevent the destructor from crashing.
  inline void Poof() { vtable_ = nullptr; }

  // Get the original function.
  // Use a function prototype
  // for the template argument to make it very easy to call this function.
  // Example syntax:
  // hook.GetMethod<bool (__thiscall*)( void*, int )>( 12 )( inst, arg );
  template <typename Fn>
  inline Fn GetMethod(size_t index) const {
    assert(index < vtable_function_count_);
    return (Fn)old_vtable_[index];
  }

 protected:
  inline void _set_guard(size_t S) { _array[1] = reinterpret_cast<void*>(S); }
  inline size_t _get_guard() const { return (size_t)_array[1]; }
  inline void _set_backptr(void* ptr) { _array[0] = ptr; }
  inline void* _get_backptr() const { return _array[0]; }

 private:
  void*** vtable_;
  void** old_vtable_;
  void** _array;
  uint vtable_function_count_;
};
}  // namespace utils
