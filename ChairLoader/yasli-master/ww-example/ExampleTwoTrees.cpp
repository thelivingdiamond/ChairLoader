#include <string>
#include <vector>
using std::vector;

#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/HSplitter.h"
#include "ww/VBox.h"
#include "ww/Button.h"
#include "ww/Color.h"
#include "ww/Icon.h"

using yasli::Archive;
using yasli::SharedPtr;
using namespace ww;

enum Filter
{
	FILTER_OUTLINE = 0x1,
	FILTER_PROPERTIES = 0x2
};

struct Node
{
	std::string name_;
	bool enabled_;
	bool favourite_;
	Color color_;
	float weight_;
	bool isRoot_;

	vector<Node> children_;

	Node()
	: enabled_(false)
	, favourite_(false)
	, isRoot_(false)
	, weight_(1.0f)
	{
	}

	void serialize(Archive& ar)
	{
		if (ar.filter(FILTER_OUTLINE)) {
			if (isRoot_) {
				#include "Icons/package.xpm"
				ar(Icon(package_xpm), "typeIcon", "^^");
			}
			else  if (children_.empty()) {
				#include "Icons/page.xpm"
				ar(Icon(page_xpm), "typeIcon", "^^");
			}
			else {
				#include "Icons/folder.xpm"
				ar(Icon(folder_xpm), "typeIcon", "^^");
			}

			ar(name_, "name", "^!!");
			ar(children_, "children", "^");
			#include "Icons/favourites.xpm"
			#include "Icons/favouritesDisabled.xpm"
			ar(IconToggle(favourite_, favourites_xpm, favouritesDisabled_xpm), "favourite", "^");
		}

		if (ar.filter(FILTER_PROPERTIES)) {
			if (!ar.filter(FILTER_OUTLINE)) { // prevent duplication when both filters enabled
				ar(name_, "name", "^");
			}
			ar(color_, "color", "Color");
			ar(enabled_, "enabled", "Enabled");
			ar(weight_, "weight", "Weight");
		}
	}
};

struct TwoTreesData
{
	Node root_;
	ww::signal0 signalChanged_;

	void serialize(Archive& ar)
	{
		ar(root_, "root", "<Tree");
	}

	TwoTreesData()
	{
		generate();
		root_.isRoot_ = true;
	}

	void generate()
	{
		root_.name_ = "Root";
		root_.children_.clear();
		
		static int index = 0;
		for (int i = 0; i < 5; ++i) {
			char name[64];
			sprintf_s(name, "Node %d", index);

			Node node;
			node.name_ = name;
			node.color_.setHSV((index % 10) / 9.0f * 360.0f, 1.0f, 1.0f);

			root_.children_.push_back(node);
			++index;
		}

		signalChanged_.emit();
	}

} twoTreesData;

class TwoTreesWidget : public ww::HSplitter
{
public:
	ww::PropertyTree* outlineTree;

	void onGenerate()
	{
		twoTreesData.generate();
	}

	void onDataChanged()
	{
		outlineTree->revert();
	}

	TwoTreesWidget()
	{
		outlineTree = new ww::PropertyTree();

		ww::VBox* vbox = new ww::VBox();
		{
			outlineTree->setFilter(FILTER_OUTLINE);
			outlineTree->setCompact(true);
			outlineTree->setUndoEnabled(true, false);
			outlineTree->setExpandLevels(2);
			twoTreesData.signalChanged_.connect(this, &TwoTreesWidget::onDataChanged);
			vbox->add(outlineTree, PACK_FILL);

			ww::Button* button = new ww::Button("Generate New Tree", 2);
			button->signalPressed().connect(this, &TwoTreesWidget::onGenerate);
			vbox->add(button, PACK_COMPACT);
		}
		add(vbox, 0.3f);

		ww::PropertyTree* propertyTree = new ww::PropertyTree();
		propertyTree->setFilter(FILTER_PROPERTIES);
		propertyTree->setUndoEnabled(true, false);
		propertyTree->setExpandLevels(2);
		add(propertyTree);

		outlineTree->attachPropertyTree(propertyTree);
		outlineTree->attach(yasli::Serializer(twoTreesData));
	}
};

ww::Widget* createTwoTrees()
{
	return new TwoTreesWidget();
}

