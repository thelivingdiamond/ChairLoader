#include <string>
#include <vector>
using std::vector;
#include "ExampleTwoTrees.h"

#include "QPropertyTree/QPropertyTree.h"
#include "PropertyTree/Color.h"
#include "yasli/STL.h"
#include "yasli/decorators/IconXPM.h"
#include <QSplitter>
#include <QPushButton>
#include <QBoxLayout>

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
				ar(yasli::IconXPM(package_xpm), "typeIcon", "^^");
			}
			else  if (children_.empty()) {
				#include "Icons/page.xpm"
				ar(yasli::IconXPM(page_xpm), "typeIcon", "^^");
			}
			else {
				#include "Icons/folder.xpm"
				ar(yasli::IconXPM(folder_xpm), "typeIcon", "^^");
			}

			ar(name_, "name", "^!!");
			ar(children_, "children", "^");
			#include "Icons/favourites.xpm"
			#include "Icons/favouritesDisabled.xpm"
			ar(yasli::IconXPMToggle(favourite_, favourites_xpm, favouritesDisabled_xpm), "favourite", "^");
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
			sprintf(name, "Node %d", index);

			Node node;
			node.name_ = name;
			node.color_.setHSV((index % 10) / 9.0f * 360.0f, 1.0f, 1.0f);

			root_.children_.push_back(node);
			++index;
		}

	}

} twoTreesData;

void TwoTreesWidget::onDataChanged()
{
	outlineTree->revert();
}

TwoTreesWidget::TwoTreesWidget()
: QSplitter(Qt::Horizontal)
{
	outlineTree = new QPropertyTree();

	QWidget* leftWidget = new QWidget();
	QBoxLayout* vbox = new QBoxLayout(QBoxLayout::TopToBottom);
	{
		outlineTree->setFilter(FILTER_OUTLINE);
		outlineTree->setCompact(true);
		outlineTree->setUndoEnabled(true, false);
		outlineTree->setExpandLevels(2);
		vbox->addWidget(outlineTree, 1);

		QPushButton* button = new QPushButton("Generate New Tree");
		QObject::connect(button, SIGNAL(pressed()), this, SLOT(onGenerate()));
		vbox->addWidget(button, 0);
	}
	leftWidget->setLayout(vbox);
	addWidget(leftWidget);

	QPropertyTree* propertyTree = new QPropertyTree();
	propertyTree->setFilter(FILTER_PROPERTIES);
	propertyTree->setUndoEnabled(true, false);
	propertyTree->setExpandLevels(2);
	addWidget(propertyTree);

	outlineTree->attachPropertyTree(propertyTree);
	outlineTree->attach(yasli::Serializer(twoTreesData));
}

void TwoTreesWidget::onGenerate()
{
	twoTreesData.generate();
}

QWidget* createExampleTwoTrees()
{
	return new TwoTreesWidget();
}
