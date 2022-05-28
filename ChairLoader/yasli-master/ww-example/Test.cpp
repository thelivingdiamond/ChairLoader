#include "UnitTest++.h"
#include "../yasli/ComplexClass.h"

// TODO: move into ComplexClass.cpp
YASLI_CLASS(PolyBase, PolyBase, "Base")
YASLI_CLASS(PolyBase, PolyDerivedA, "Derived A")
YASLI_CLASS(PolyBase, PolyDerivedB, "Derived B")

int main(int argc, char* argv[])
{
	yasli::setTestMode(true);
	return UnitTest::RunAllTests();
}
