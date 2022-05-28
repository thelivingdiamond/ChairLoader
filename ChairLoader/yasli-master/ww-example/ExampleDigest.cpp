#include <string>
#include <vector>
using std::vector;

#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/Color.h"

using yasli::Archive;
using yasli::SharedPtr;
using namespace ww;


struct DigestItem
{
	std::string name_;
	bool enabled_;
	Color color_;
	float weight_;


	void serialize(Archive& ar)
	{
		// & character makes item appear in a digest of parent row
		ar(name_, "name", "^Name");
		ar(enabled_, "enabled", "Enabled");
		ar(color_, "color", "Color");
		ar(weight_, "weight", "Weight");
	}

	DigestItem()
	: enabled_(false)
	, weight_(1.0f)
	{
	}
};
typedef vector<DigestItem> Items;

struct DigestData
{
	void serialize(Archive& ar)
	{
		ar(items_, "items", "Items");
	}

	DigestData()
	{
		items_.resize(20);

		for (size_t i = 0; i < items_.size(); ++i)
		{
			char buf[32];
			sprintf_s(buf, sizeof(buf), "Item %i", int(i));
			items_[i].name_ = buf;
			items_[i].color_.setHSV((i * 360.0f) / items_.size(), 1.0f, 1.0f);
		}
	}

	Items items_;
} digestData;

ww::Widget* createDigestSample()
{
	ww::PropertyTree* propertyTree = new ww::PropertyTree();

	propertyTree->setUndoEnabled(true, false);
	propertyTree->attach(yasli::Serializer(digestData));
	propertyTree->expandAll();

	return propertyTree;
}

