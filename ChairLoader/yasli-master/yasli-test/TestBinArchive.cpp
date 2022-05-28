#include "UnitTest++.h"

#include "ComplexClass.h"
#include "yasli/BinArchive.h"

#ifndef _MSC_VER
# include <wchar.h>
#endif

using std::string;
using namespace yasli;

SUITE(BinArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		ComplexClass objChanged;
		objChanged.change();
		BinOArchive oa;
		CHECK(oa(objChanged, "obj"));
		CHECK(oa.length() != 0);

		{
			ComplexClass obj;

			BinIArchive ia;
			CHECK(ia.open(oa.buffer(), oa.length()));
			CHECK(ia(obj, "obj"));

			BinOArchive oa2;
			CHECK(oa2(obj, "obj"));

			obj.checkEquality(objChanged);
			CHECK(oa2.length() != 0);
			CHECK(oa.length() == oa2.length());
			CHECK(memcmp(oa.buffer(), oa2.buffer(), oa.length()) == 0);
		}
	}
}
