#include <string>
#include <vector>
using std::vector;

#include "PropertyTree/IMenu.h"
#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/Color.h"
#include "ww/Icon.h"
#include "ww/Decorators.h"
#include "ww/FileSelector.h"
#include "ww/KeyPress.h"
#include "PropertyTree/Color.h"

#include "yasli/BitVector.h" 
#include "yasli/decorators/BitFlags.h"

using yasli::Archive;
using yasli::SharedPtr;

#include "Icons/favourites.xpm"
#include "Icons/favouritesDisabled.xpm"
static ww::Icon iconFavourite(favourites_xpm);
static ww::Icon iconFavouriteDisabled(favouritesDisabled_xpm);

enum Flags
{
	FLAG_FIRST = 1 << 0,
	FLAG_SECOND = 1 << 1,
	FLAG_THIRD = 1 << 2,
	FLAG_ALL = FLAG_FIRST | FLAG_SECOND | FLAG_THIRD
};

YASLI_ENUM_BEGIN(Flags, "Flags")
YASLI_ENUM(FLAG_FIRST, "first", "First")
YASLI_ENUM(FLAG_SECOND, "second", "Second")
YASLI_ENUM(FLAG_THIRD, "third", "Third")
YASLI_ENUM(FLAG_ALL, "all", "All")
YASLI_ENUM_END()

struct CustomRows
{
	yasli::BitVector<Flags> bitVector;
	int bitFlags;
	bool notDecorator;
	ww::KeyPress hotkey;
	ww::Color wwColor;

	bool radio;
	bool wwIconToggle;

	yasli::string fileSelector;
	bool buttonState;
	property_tree::Color propertyTreeColor;

	CustomRows()
	: bitFlags(FLAG_FIRST | FLAG_THIRD)
	, bitVector(FLAG_FIRST | FLAG_THIRD)
	, notDecorator(true)
	, radio(true)
	, fileSelector("test_file.txt")
	, buttonState(false)
	, wwIconToggle(false)
	, wwColor(0, 255, 0, 128)
	, propertyTreeColor(255, 0, 0, 128)
	{

	}

	void serialize(Archive& ar)
	{
		// old way of serializing bit flags
		ar(bitVector, "bitVector", "BitVector<>");

		ar(ww::NotDecorator(notDecorator), "notDecorator", "ww::NotDecorator");

		ar(ww::RadioDecorator(radio), "radio1", "ww::RadioDecorator");

		static ww::FileSelector::Options options("*.txt", false, ".");
		ar(ww::FileSelector(fileSelector, options), "fileSelector", "ww::FileSelector");
		ar(hotkey, "hotkey", "ww::KeyPress");
		ar(wwColor, "wwColor", "ww::Color");
		ar(ww::Icon(iconFavourite), "wwIcon", "ww::Icon");
		ar(ww::IconToggle(wwIconToggle, iconFavourite, iconFavouriteDisabled), "wwIconToggle", "ww::IconToggle");

		ar(yasli::HorizontalLine(), "yasliHline", "<yasli::HorizontalLine");

		// new way to serialize bit flags, less intrusive
		ar(yasli::BitFlags<Flags>(bitFlags), "bitFlags", "yasli::BitFlags");
			
		ar(propertyTreeColor, "propertyTreeColor", "property_tree::Color");

		ar(yasli::Button("yasli::Button"), "yasliButton", "<");
	}
} customRows;

ww::Widget* createCustomRows()
{
	ww::PropertyTree* propertyTree = new ww::PropertyTree();

	propertyTree->setExpandLevels(1);
	propertyTree->attach(yasli::Serializer(customRows));
	propertyTree->expandAll();

	return propertyTree;
}
