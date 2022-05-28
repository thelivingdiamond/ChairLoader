// This files contains strings both in utf-8 and windows-1251 encoding.
// We need this for CheckUtf8Conversion test.
#include "UnitTest++.h"
#include <vector>
#include <utility>

#include "ComplexClass.h"

#ifndef _MSC_VER
# include <wchar.h>
#endif

using std::string;
using namespace yasli;

struct SGlobalPrefix {};

SUITE(General)
{
	namespace Namespace {
		template<class T> struct Container {};
		class CPrefix {};
	}

	struct TestBaseA
	{
		virtual void testA() {}
		void YASLI_SERIALIZE_METHOD(Archive& ar) {}
	};

	struct TestBaseB : TestBaseA
	{
		virtual void testB() {}
	};

	struct TestClass : TestBaseB
	{
		void testA() {}
		void testB() {}
	};

	YASLI_CLASS_NAME(TestBaseA, TestClass, "a-class", "A Class")
	YASLI_CLASS_NAME(TestBaseB, TestClass, "b-class", "B Class")

	TEST(RegistrationInMultipleFactories)
	{
		yasli::ClassFactory<TestBaseA>& aFactory = yasli::ClassFactory<TestBaseA>::the(); 
		TestBaseA* a = aFactory.create(aFactory.findTypeByName("a-class"));
		CHECK(a != 0);

		yasli::ClassFactory<TestBaseB>& bFactory = yasli::ClassFactory<TestBaseB>::the(); 
		TestBaseB* b = bFactory.create(bFactory.findTypeByName("b-class"));
		CHECK(b != 0);
	}

#if YASLI_NO_RTTI
	TEST(TypeIDNameParsing)
	{
		CHECK_EQUAL("int", string(TypeID::get<int>().name()));
		CHECK_EQUAL("float", string(TypeID::get<float>().name()));
		CHECK_EQUAL("Container<pair<int,float>>", string(TypeID::get<Namespace::Container<std::pair<int, float> > >().name()));
		CHECK_EQUAL("Prefix", string(TypeID::get<Namespace::CPrefix>().name()));
		CHECK_EQUAL("GlobalPrefix", string(TypeID::get<SGlobalPrefix>().name()));
		CHECK_EQUAL("basic_string<char>", string(TypeID::get<string>().name()));
	}
#endif
}

