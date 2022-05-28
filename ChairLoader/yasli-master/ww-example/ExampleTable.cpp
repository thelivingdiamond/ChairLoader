#include <string>
#include <vector>
using std::vector;

#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/Color.h"
#include "ww/Icon.h"

using yasli::Archive;
using yasli::SharedPtr;
using namespace ww;

#include "Icons/favourites.xpm"
#include "Icons/favouritesDisabled.xpm"
static Icon iconFavourite(favourites_xpm);
static Icon iconFavouriteDisabled(favouritesDisabled_xpm);

struct TableItem
{
	bool favourite_;
	std::string name_;
	bool enabled_;
	Color color_;
	float weight_;


	void serialize(Archive& ar)
	{
		ar(IconToggle(favourite_, iconFavourite, iconFavouriteDisabled), "favourite", "^^");
		ar(name_, "name", "^Name");
		ar(enabled_, "enabled", "^");
		ar(color_, "color", "^");
		ar(weight_, "weight", "^Weight");
	}

	TableItem()
	: favourite_(false)
	, enabled_(false)
	, weight_(1.0f)
	{
	}
};
typedef vector<TableItem> Items;

struct TableData
{
	void serialize(Archive& ar)
	{
		ar(items_, "items", "Items");
	}

	TableData()
	{
		items_.resize(100);

		for (size_t i = 0; i < items_.size(); ++i)
		{
			char buf[32];
			sprintf_s(buf, sizeof(buf), "Item %i", int(i));
			items_[i].name_ = buf;
			items_[i].color_.setHSV((i * 360.0f) / items_.size(), 1.0f, 1.0f);
			items_[i].color_.a = int(200.0f + 55.0f * sinf(float(i) * 3.1415926f / 10.0f));
		}
	}

	Items items_;
} digestData;

ww::Widget* createTableSample()
{
	ww::PropertyTree* propertyTree = new ww::PropertyTree();

	propertyTree->setUndoEnabled(true, false);
	propertyTree->attach(yasli::Serializer(digestData));
	propertyTree->expandAll();

	return propertyTree;
}

