#include <string>
#include <vector>
using std::vector;

#include "QPropertyTree/QPropertyTree.h"
#include "yasli/STL.h"
#include "yasli/decorators/IconXPM.h"

using yasli::Archive;
using yasli::SharedPtr;

struct TableItem
{
	bool favourite_;
	std::string name_;
	bool enabled_;
	Color color_;
	float weight_;


	void serialize(Archive& ar)
	{
		#include "Icons/favourites.xpm"
		#include "Icons/favouritesDisabled.xpm"
		ar(yasli::IconXPMToggle(favourite_, favourites_xpm, favouritesDisabled_xpm), "favourite", "^^");
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
			sprintf(buf, "Item %i", int(i));
			items_[i].name_ = buf;
			items_[i].color_.setHSV((i * 360.0f) / items_.size(), 1.0f, 1.0f);
			items_[i].color_.a = int(200.0f + 55.0f * sinf(float(i) * 3.1415926f / 10.0f));
		}
	}

	Items items_;
} tableData;

QWidget* createExampleTable()
{
	QPropertyTree* propertyTree = new QPropertyTree();

	propertyTree->setUndoEnabled(true, false);
	propertyTree->attach(yasli::Serializer(tableData));
	propertyTree->expandAll();

	return propertyTree;
}

