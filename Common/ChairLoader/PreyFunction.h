#pragma once
#include <type_traits>
#include <ChairLoader/FunctionHook.h>
#include "pch.h"

// These declarations are required because C++
template <typename UnusedType>
class PreyFunction;
template <typename UnusedType>
class PreyFunctionHook;

// Forward declarations
template <typename ReturnType, typename ... ArgumentTypes>
class PreyFunctionHook<ReturnType(ArgumentTypes ...)>;

//------------------------------------------------------------
// PreyFunction
//------------------------------------------------------------

//! Base non-templated class for pointers to functions in the game DLL.
class PreyFunctionBase : NoCopy {
public:
	//! Contructs the function pointer.
	//! @param	offset	Offset into the game module.
	PreyFunctionBase(uintptr_t offset);

	//! @returns raw pointer to the function. If not initialized, returns offset.
	//! @{
	inline uintptr_t GetIntPtr() { return m_Ptr; }
	inline void* GetVoidPtr() { return reinterpret_cast<void*>(m_Ptr); }
	//! @}

protected:
	uintptr_t m_Ptr = 0;

private:
	PreyFunctionBase* m_pNext = nullptr;

	//! Initializes the function pointer.
	//! @param	moduleBase	Base address of the game module.
	void Init(uintptr_t moduleBase);

	friend class PreyFunctionSystem;
};

//! Class for pointers to functions in the game DLL.
//! Instances of this class must exist for as long as current DLL does.
template <typename ReturnType, typename ... ArgumentTypes>
class PreyFunction<ReturnType(ArgumentTypes ...)> : public PreyFunctionBase {
public:
	using Type = ReturnType(ArgumentTypes ...);

	using PreyFunctionBase::PreyFunctionBase;

	//! Calls the function with specified arguments.
	inline ReturnType operator ()(ArgumentTypes ... args) {
		return Get()(args ...);
	}

	//! @returns the function pointer.
	inline Type* Get() { return reinterpret_cast<Type*>(m_Ptr); }

	//! Creates a function hook. The hook will be installed during mod init.
	auto MakeHook();
};

//------------------------------------------------------------
// PreyFunctionHook
//------------------------------------------------------------

//! Base class for PreyDll function hooks.
class PreyFunctionHookBase : NoCopy {
public:
	//! @returns whether the hook function is set.
	inline bool IsHookFuncSet() { return m_pHookFunc != nullptr; }

	//! @returns whether a hook is installed.
	inline bool IsHooked() { return m_Hook.IsHooked(); }

protected:
	PreyFunctionBase *m_pOrigFunc = nullptr;
	void* m_pHookFunc = nullptr;
	FunctionHookTypeless m_Hook;

	//! Constructs the hook.
	//! @param	origFunc	Function to be hooked
	PreyFunctionHookBase(PreyFunctionBase* origFunc);

private:
	PreyFunctionHookBase* m_pNext = nullptr;

	void InstallHook();
	void RemoveHook();

	friend class PreyFunctionSystem;
};

//! Class for PreyDll function hooks.
//! Instances of this class must exist for as long as current DLL does.
template <typename ReturnType, typename ... ArgumentTypes>
class PreyFunctionHook<ReturnType(ArgumentTypes ...)> : PreyFunctionHookBase {
public:
	using Type = ReturnType(ArgumentTypes ...);

	//! Constructs the hook.
	//! @param	origFunc	Function to be hooked
	//! @param	hookFunc	Function that will replace the original function.
	inline PreyFunctionHook(PreyFunction<Type>* origFunc)
		: PreyFunctionHookBase(origFunc) {}

	//! Sets the hook function.
	//! @param	hookFunc	Function that will replace the original function.
	inline void SetHookFunc(Type* hookFunc) { m_pHookFunc = hookFunc; }

	//! Calls the original function with specified arguments.
	inline ReturnType InvokeOrig(ArgumentTypes ... args) {
		return GetOrig()(args ...);
	}

	//! @returns pointer to use when calling original function.
	inline Type* GetOrig() { return static_cast<Type*>(m_Hook.GetOrig()); }
};

//------------------------------------------------------------
// PreyFunctionSystem
//------------------------------------------------------------

//! Static class for Prey function initialization.
class PreyFunctionSystem {
public:
	//! Initializes all functions.
	static void Init(uintptr_t moduleBase);

	//! Installs all hooks.
	//! Must be called in a Detours transaction.
	static void InstallHooks();

	//! Installs all installed hooks.
	//! Must be called in a Detours transaction.
	static void RemoveHooks();
};

template<typename ReturnType, typename ... ArgumentTypes>
inline auto PreyFunction<ReturnType(ArgumentTypes ...)>::MakeHook()
{
	// This method is called for a global from other global.
	// Do not dereference `this` - it may not be initialized yet.
	return PreyFunctionHook<Type>(this);
}
