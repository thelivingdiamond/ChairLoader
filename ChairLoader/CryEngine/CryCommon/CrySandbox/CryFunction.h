// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <functional>

namespace CryFunctionPrivate
{
template<typename Signature> struct FunctionWrapper
{
	static std::function<Signature> Cast(const std::function<Signature>& f) { return f; }
};

//0 argument
template<typename Ret>
struct FunctionWrapper<Ret()>
{
	static std::function<Ret()> Cast(const std::function<Ret()>& f) { return f; }

	template<typename OtherRet>
	static std::function<Ret()> Cast(const std::function<OtherRet()>& f) { return [ = ]() { return static_cast<Ret>(f()); }; }
};

template<>
struct FunctionWrapper<void()>
{
	static std::function<void()> Cast(const std::function<void()>& f) { return f; }

	template<typename OtherRet>
	static std::function<void()> Cast(const std::function<OtherRet()>& f) { return [ = ]() { f(); }; }
};

//1 argument
template<typename Ret, typename Arg1>
struct FunctionWrapper<Ret(Arg1)>
{
	static std::function<Ret(Arg1)> Cast(const std::function<Ret(Arg1)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<Ret(Arg1)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1))); }; }

	template<typename OtherRet>
	static std::function<Ret(Arg1)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1) { return static_cast<Ret>(f()); }; }
};

template<typename Arg1>
struct FunctionWrapper<void(Arg1)>
{
	static std::function<void(Arg1)> Cast(const std::function<void(Arg1)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<void(Arg1)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1) { f(static_cast<OtherArg1>(arg1)); }; }

	template<typename OtherRet>
	static std::function<void(Arg1)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1) { f(); }; }
};

//2 arguments
template<typename Ret, typename Arg1, typename Arg2>
struct FunctionWrapper<Ret(Arg1, Arg2)>
{
	static std::function<Ret(Arg1, Arg2)> Cast(const std::function<Ret(Arg1, Arg2)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1, typename OtherArg2>
	static std::function<Ret(Arg1, Arg2)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2)>& f) { return [ = ](Arg1 arg1, Arg2 arg2) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2))); }; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<Ret(Arg1, Arg2)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1, Arg2 arg2) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1))); }; }

	template<typename OtherRet>
	static std::function<Ret(Arg1, Arg2)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1, Arg2 arg2) { return static_cast<Ret>(f()); }; }
};

template<typename Arg1, typename Arg2>
struct FunctionWrapper<void(Arg1, Arg2)>
{
	static std::function<void(Arg1, Arg2)> Cast(const std::function<void(Arg1, Arg2)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1, typename OtherArg2>
	static std::function<void(Arg1, Arg2)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2)>& f) { return [ = ](Arg1 arg1, Arg2 arg2) { f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2)); }; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<void(Arg1, Arg2)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1, Arg2 arg2) { f(static_cast<OtherArg1>(arg1)); }; }

	template<typename OtherRet>
	static std::function<void(Arg1, Arg2)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1, Arg2 arg2) { f(); }; }
};

//3 arguments
template<typename Ret, typename Arg1, typename Arg2, typename Arg3>
struct FunctionWrapper<Ret(Arg1, Arg2, Arg3)>
{
	static std::function<Ret(Arg1, Arg2, Arg3)> Cast(const std::function<Ret(Arg1, Arg2, Arg3)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1, typename OtherArg2, typename OtherArg3>
	static std::function<Ret(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2, OtherArg3)>& f)
	{
		return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2), static_cast<OtherArg3>(arg3))); };
	}

	template<typename OtherRet, typename OtherArg1, typename OtherArg2>
	static std::function<Ret(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2))); }; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<Ret(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1))); }; }

	template<typename OtherRet>
	static std::function<Ret(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { return static_cast<Ret>(f()); }; }
};

template<typename Arg1, typename Arg2, typename Arg3>
struct FunctionWrapper<void(Arg1, Arg2, Arg3)>
{
	static std::function<void(Arg1, Arg2, Arg3)> Cast(const std::function<void(Arg1, Arg2, Arg3)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1, typename OtherArg2, typename OtherArg3>
	static std::function<void(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2, OtherArg3)>& f)
	{
		return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2), static_cast<OtherArg3>(arg3)); };
	}

	template<typename OtherRet, typename OtherArg1, typename OtherArg2>
	static std::function<void(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2)); }; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<void(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { f(static_cast<OtherArg1>(arg1)); }; }

	template<typename OtherRet>
	static std::function<void(Arg1, Arg2, Arg3)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { f(); }; }
};

//4 arguments
template<typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
struct FunctionWrapper<Ret(Arg1, Arg2, Arg3, Arg4)>
{
	static std::function<Ret(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<Ret(Arg1, Arg2, Arg3, Arg4)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1, typename OtherArg2, typename OtherArg3, typename OtherArg4>
	static std::function<Ret(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2, OtherArg3, OtherArg4)>& f)
	{
		return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2), static_cast<OtherArg3>(arg3), static_cast<OtherArg4>(arg4))); };
	}

	template<typename OtherRet, typename OtherArg1, typename OtherArg2, typename OtherArg3>
	static std::function<Ret(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2, OtherArg3)>& f)
	{
		return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2), static_cast<OtherArg3>(arg3))); };
	}

	template<typename OtherRet, typename OtherArg1, typename OtherArg2>
	static std::function<Ret(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2))); }; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<Ret(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { return static_cast<Ret>(f(static_cast<OtherArg1>(arg1))); }; }

	template<typename OtherRet>
	static std::function<Ret(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { return static_cast<Ret>(f()); }; }
};

template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
struct FunctionWrapper<void(Arg1, Arg2, Arg3, Arg4)>
{
	static std::function<void(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<void(Arg1, Arg2, Arg3, Arg4)>& f) { return f; }

	template<typename OtherRet, typename OtherArg1, typename OtherArg2, typename OtherArg3, typename OtherArg4>
	static std::function<void(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2, OtherArg3)>& f)
	{
		return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2), static_cast<OtherArg3>(arg3), static_cast<OtherArg4>(arg4)); };
	}

	template<typename OtherRet, typename OtherArg1, typename OtherArg2, typename OtherArg3>
	static std::function<void(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2, OtherArg3)>& f)
	{
		return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2), static_cast<OtherArg3>(arg3)); };
	}

	template<typename OtherRet, typename OtherArg1, typename OtherArg2>
	static std::function<void(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1, OtherArg2)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { f(static_cast<OtherArg1>(arg1), static_cast<OtherArg2>(arg2)); }; }

	template<typename OtherRet, typename OtherArg1>
	static std::function<void(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet(OtherArg1)>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { f(static_cast<OtherArg1>(arg1)); }; }

	template<typename OtherRet>
	static std::function<void(Arg1, Arg2, Arg3, Arg4)> Cast(const std::function<OtherRet()>& f) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { f(); }; }
};
}

//Intended breach of CryENGINE coding standard to match c++ standard for casting, i.e. static_cast/dynamic_cast etc
template<typename Sig1, typename Sig2>
inline std::function<Sig1> function_cast(const std::function<Sig2>& f)
{
	return CryFunctionPrivate::FunctionWrapper<Sig1>::Cast(f);
}

//////////////////////////////////////////////////////////////////////////

namespace CryFunctionPrivate
{
template<typename Object, typename Function> struct MemberFunctionWrapper {};

template<typename Object, typename Object2, typename Ret>
struct MemberFunctionWrapper<Object, Ret (Object2::*)()>
{
	typedef std::function<Ret()> ReturnType;

	static ReturnType Wrap(Object* obj, Ret (Object2::* fun)()) { return [ = ]() { return (obj->*fun)(); }; }
};

template<typename Object, typename Object2, typename Ret, typename Arg1>
struct MemberFunctionWrapper<Object, Ret (Object2::*)(Arg1)>
{
	typedef std::function<Ret(Arg1)> ReturnType;

	static ReturnType Wrap(Object* obj, Ret (Object2::* fun)(Arg1)) { return [ = ](Arg1 arg1) { return (obj->*fun)(arg1); }; }
};

template<typename Object, typename Object2, typename Ret, typename Arg1, typename Arg2>
struct MemberFunctionWrapper<Object, Ret (Object2::*)(Arg1, Arg2)>
{
	typedef std::function<Ret(Arg1, Arg2)> ReturnType;

	static ReturnType Wrap(Object* obj, Ret (Object2::* fun)(Arg1, Arg2)) { return [ = ](Arg1 arg1, Arg2 arg2) { return (obj->*fun)(arg1, arg2); }; }
};

template<typename Object, typename Object2, typename Ret, typename Arg1, typename Arg2, typename Arg3>
struct MemberFunctionWrapper<Object, Ret (Object2::*)(Arg1, Arg2, Arg3)>
{
	typedef std::function<Ret(Arg1, Arg2, Arg3)> ReturnType;

	static ReturnType Wrap(Object* obj, Ret (Object2::* fun)(Arg1, Arg2, Arg3)) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3) { return (obj->*fun)(arg1, arg2, arg3); }; }
};

template<typename Object, typename Object2, typename Ret, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
struct MemberFunctionWrapper<Object, Ret (Object2::*)(Arg1, Arg2, Arg3, Arg4)>
{
	typedef std::function<Ret(Arg1, Arg2, Arg3, Arg4)> ReturnType;

	static ReturnType Wrap(Object* obj, Ret (Object2::* fun)(Arg1, Arg2, Arg3, Arg4)) { return [ = ](Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) { return (obj->*fun)(arg1, arg2, arg3, arg4); }; }
};
}

template<typename Object, typename Function>
inline typename CryFunctionPrivate::MemberFunctionWrapper<Object, Function>::ReturnType WrapMemberFunction(Object* obj, Function function)
{
	return CryFunctionPrivate::MemberFunctionWrapper<Object, Function>::Wrap(obj, function);
}
