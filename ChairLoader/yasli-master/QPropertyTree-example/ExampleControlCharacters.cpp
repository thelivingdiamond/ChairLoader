#include <string>
#include <vector>
using std::vector;

#include "QPropertyTree/QPropertyTree.h"
#include "PropertyTree/Color.h"
#include "yasli/STL.h"

using yasli::Archive;
using yasli::SharedPtr;
using property_tree::Color;

struct ControlCharactersItem
{
	std::string name_;
	bool enabled_;
	Color color_;
	float weight_;


	void serialize(Archive& ar)
	{
		// ^ character inlines item
		ar(name_, "name", "^Name");
		ar(enabled_, "enabled", "Enabled");
		ar(color_, "color", "Color");
		// < character expands value field
		ar(weight_, "weight", "<Weight");
	}

	ControlCharactersItem()
	: enabled_(false)
	, weight_(1.0f)
	{
	}
};
typedef vector<ControlCharactersItem> Items;

struct ControlCharactersData
{
	void serialize(Archive& ar)
	{
		ar(items_, "items", "Items");
	}

	ControlCharactersData()
	{
		items_.resize(20);

		for (size_t i = 0; i < items_.size(); ++i)
		{
			char buf[32];
			sprintf(buf, "Item %i", int(i));
			items_[i].name_ = buf;
			items_[i].color_.setHSV((i * 360.0f) / items_.size(), 1.0f, 1.0f);
		}
	}

	Items items_;
} digestData;

QWidget* createExampleControlCharacters()
{
	QPropertyTree* propertyTree = new QPropertyTree();

	propertyTree->setUndoEnabled(true, false);
	propertyTree->attach(yasli::Serializer(digestData));
	propertyTree->expandAll();

	return propertyTree;
}

