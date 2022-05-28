#include <string>
#include <vector>
using std::vector;

namespace yasli {
	class Archive;
	template<class T>
	class SharedPtr;
};
using yasli::Archive;
using yasli::SharedPtr;

struct ObjectNode;

bool serialize(Archive& ar, SharedPtr<ObjectNode>& node, const char* name, const char* label);

#include "ww/PropertyTree.h"
#include "ww/Serialization.h"
#include "ww/HSplitter.h"
#include "ww/VBox.h"
#include "ww/Button.h"
#include "ww/Color.h"
#include "ww/Icon.h"

using yasli::RefCounter;
using namespace ww;

enum Filter
{
	FILTER_OUTLINE = 0x1,
	FILTER_PROPERTIES = 0x2
};

struct Geometry : RefCounter
{
	virtual ~Geometry() {}
	virtual void serialize(Archive& ar) {
	}
};

struct GeometryBox : Geometry
{
	virtual void serialize(Archive& ar) {
	}
};

struct GeometrySphere : Geometry
{
	virtual void serialize(Archive& ar) {
	}
};

YASLI_CLASS_NULL(Geometry, "None")
YASLI_CLASS(Geometry, GeometryBox, "Box")
YASLI_CLASS(Geometry, GeometrySphere, "Sphere")

#include "Icons/favourites.xpm"
#include "Icons/favouritesDisabled.xpm"

struct ObjectNode : RefCounter
{
	std::string name_;
	bool enabled_;
	bool favourite_;
	Color color_;
	float weight_;
	bool isRoot_;
	int counter_;

	vector<SharedPtr<ObjectNode> > children_;
	SharedPtr<Geometry> geometry_;

	ObjectNode()
	: enabled_(false)
	, favourite_(false)
	, isRoot_(false)
	, weight_(1.0f)
	, counter_(0)
	{
	}

	void serialize(Archive& ar)
	{
		if (ar.filter(FILTER_OUTLINE)) {
			if (isRoot_) {
				#include "Icons/package.xpm"
				static Icon packageIcon(package_xpm);
				ar(packageIcon, "typeIcon", "^^");
			}
			else  if (children_.empty()) {
				#include "Icons/page.xpm"
				static Icon pageIcon(page_xpm);
				ar(pageIcon, "typeIcon", "^^");
			}
			else {
				#include "Icons/folder.xpm"
				static Icon folderIcon(folder_xpm);
				ar(folderIcon, "typeIcon", "^^");
			}

			ar(name_, "name", "^!!<");
			ar(geometry_, "geometry", "^>40>");
			ar(children_, "children", "^>0>");

			ar(IconToggle(favourite_, favourites_xpm, favouritesDisabled_xpm), "favourite", "^");
			
			if (ar.isOutput())
			{
				++counter_;
				ar(counter_, "counter", "^>40>");
			}
		}

		if (ar.filter(FILTER_PROPERTIES)) {
			if (!ar.filter(FILTER_OUTLINE)) { // prevent duplication when both filters enabled
				ar(name_, "name", "^");
			}
			ar(color_, "color", "Color");
			ar(enabled_, "enabled", "Enabled");
			ar(weight_, "weight", "Weight");
			ar(geometry_, "geometry", "Geometry");
			ar(IconToggle(favourite_, favourites_xpm, favouritesDisabled_xpm), "favourite", "Favourite");
		}

	}
};

bool serialize(Archive& ar, SharedPtr<ObjectNode>& node, const char* name, const char* label)
{
	if (!node)
		node = new ObjectNode();
	return ar(yasli::asObject(node), name, label);
}

struct ObjectsData
{
	SharedPtr<ObjectNode> root_;
	ww::signal0 signalChanged_;

	void serialize(Archive& ar)
	{
		ar(root_, "root", "<Tree");
	}

	ObjectsData()
	{
		root_ = new ObjectNode;
		generate();
		root_->isRoot_ = true;
	}

	void generate()
	{
		root_->name_ = "Root";
		root_->children_.clear();
		
		static int index = 0;
		for (int i = 0; i < 55; ++i) {
			char name[32];
			sprintf_s(name, sizeof(name), "Node %d", index);

			ObjectNode node;
			node.name_ = name;
			node.color_.setHSV((index % 10) / 9.0f * 360.0f, 1.0f, 1.0f);

			root_->children_.push_back(new ObjectNode(node));
			++index;
		}

		signalChanged_.emit();
	}

} objectsData;

class ObjectsWidget : public ww::HSplitter
{
public:
	ww::PropertyTree* outlineTree_;
	void onGenerate()
	{
		objectsData.generate();
	}

	ObjectsWidget()
	{
		outlineTree_ = new ww::PropertyTree();

		ww::VBox* vbox = new ww::VBox();
		{
			outlineTree_->setFilter(FILTER_OUTLINE);
			outlineTree_->setCompact(true);
			outlineTree_->setUndoEnabled(true, false);
			outlineTree_->setShowContainerIndices(false);
			outlineTree_->setExpandLevels(2);
			objectsData.signalChanged_.connect(this, &ObjectsWidget::onDataChanged);
			vbox->add(outlineTree_, PACK_FILL);

			ww::Button* button = new ww::Button("Generate New Tree", 2);
			button->signalPressed().connect(this, &ObjectsWidget::onGenerate);
			vbox->add(button, PACK_COMPACT);
		}
		add(vbox, 0.3f);

		ww::PropertyTree* propertyTree = new ww::PropertyTree();
		propertyTree->setFilter(FILTER_PROPERTIES);
		propertyTree->signalObjectChanged().connect(this, &ObjectsWidget::onPropertyChanged);
		propertyTree->setUndoEnabled(true, false);
		propertyTree->setExpandLevels(2);
		add(propertyTree);

		outlineTree_->attachPropertyTree(propertyTree);
		outlineTree_->attach(yasli::Serializer(objectsData));
	}

	void onDataChanged()
	{
		outlineTree_->revert();
	}

	void onPropertyChanged(const yasli::Object& object)
	{
		//outlineTree_->revertObject(object.address());
	}
};

ww::Widget* createObjectsSample()
{
	return new ObjectsWidget();
}

