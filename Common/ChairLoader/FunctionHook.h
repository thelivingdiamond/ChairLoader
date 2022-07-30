#pragma once
#include "pch.h"

// Base non-templated class for function hooks.
class FunctionHookBase : NoCopy {
public:
	FunctionHookBase();
	~FunctionHookBase();

	//! Removes the installed hook.
	//! Must be called in a Detours transaction.
	void RemoveHook();

	//! @returns whether a hook is installed.
	inline bool IsHooked() { return m_pfn != nullptr; }

protected:
	void* m_pfn = nullptr;
	void* m_pfnHook = nullptr;

	//! Installs a function hook.
	//! Must be called in a Detours transaction.
	//! @param	origFunc	Function to be hooked.
	//! @param	hookFunc	Function that will replace the original function.
	void InstallHook(void* origFunc, void* hookFunc);
};

//! This declaration is required because C++
template <typename UnusedType>
class FunctionHook;

//! Class for function hooks.
template <typename ReturnType, typename ... ArgumentTypes>
class FunctionHook<ReturnType(ArgumentTypes ...)> : public FunctionHookBase {
public:
	using Type = ReturnType(ArgumentTypes ...);

	//! Installs a function hook.
	//! Must be called in a Detours transaction.
	//! @param	origFunc	Function to be hooked.
	//! @param	hookFunc	Function that will replace the original function.
	inline void InstallHook(Type* origFunc, Type* hookFunc) { InstallHook(origFunc, hookFunc); }

	//! Calls the original function with specified arguments.
	inline ReturnType InvokeOrig(ArgumentTypes ... args) {
		return static_cast<Type*>(m_pfn)(args ...);
	}

	//! @returns pointer to use when calling original function.
	inline void* GetOrig() { return static_cast<Type*>(m_pfn); }
};

//! Class for function hooks that don't use templates.
class FunctionHookTypeless : public FunctionHookBase {
public:
	//! Installs a function hook.
	//! Must be called in a Detours transaction.
	//! @param	origFunc	Function to be hooked.
	//! @param	hookFunc	Function that will replace the original function.
	inline void InstallHook(void* origFunc, void* hookFunc) {
		FunctionHookBase::InstallHook(origFunc, hookFunc);
	}

	//! @returns pointer to use when calling original function.
	inline void* GetOrig() { return m_pfn; }
};
