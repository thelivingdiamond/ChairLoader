#include "TestData.h"
#define _USE_MATH_DEFINES
#include "yasli/Archive.h"
#include "yasli/Enum.h"
#include "yasli/ClassFactory.h"

YASLI_ENUM_BEGIN(SwitcherType, "SwitcherType")
YASLI_ENUM_VALUE(SWITCHER_AND, "И")
YASLI_ENUM_VALUE(SWITCHER_OR, "ИЛИ")
YASLI_ENUM_END()

YASLI_ENUM_BEGIN(USELESS_ENUM, "Enumeration")
YASLI_ENUM_VALUE(FIRST_VALUE, "First Value")
YASLI_ENUM_VALUE(SECOND_VALUE, "Second Value")
YASLI_ENUM_VALUE(THIRD_VALUE, "Third Value")
YASLI_ENUM_VALUE(LAST_VALUE, "Last Value")
YASLI_ENUM_END()

YASLI_ENUM_BEGIN(EnumType, "Тип зоны")
YASLI_ENUM_VALUE(ENUM_STRINGS, "Strings")
YASLI_ENUM_VALUE(ENUM_INTEGERS, "Integer Types зона")
YASLI_ENUM_VALUE(ENUM_FLOATS, "Floating Point Types")
YASLI_ENUM_END()

YASLI_ENUM_BEGIN(USELESS_FLAGS, "Flags")
YASLI_ENUM_VALUE(FIRST_FLAG, "First Flags")
YASLI_ENUM_VALUE(SECOND_FLAG, "Second Flag")
YASLI_ENUM_VALUE(THIRD_FLAG, "Third Flag")
YASLI_ENUM_END()

YASLI_CLASS(TestBase, TestSwitcher, "И/ИЛИ");
YASLI_CLASS(TestBase, TestBase, "Базовый класс");
YASLI_CLASS(TestBase, TestDerivedA, "Производный класс A");
YASLI_CLASS(TestBase, TestDerivedB, "Производный класс B");


