
#include "QPropertyTree/QPropertyTree.h"

#include "yasli/Enum.h"
#include "yasli/decorators/Range.h"
#include "yasli/decorators/FileOpen.h"
#include "yasli/decorators/Button.h"
#include "yasli/decorators/HorizontalLine.h"
#include "yasli/STL.h"
#include "PropertyTree/Color.h"
#include <QMessageBox>

using yasli::Archive;
using yasli::SharedPtr;
using property_tree::Color;

class ConditionBase : public yasli::RefCounter
{
public:
	ConditionBase()
	: isNegated_(false)
	{
	}
	virtual ~ConditionBase() {}

	virtual void serialize(Archive& ar)
	{
		ar(isNegated_, "not", isNegated_ ? "^^Not" : "^^");
	}
private:
	bool isNegated_;
};
YASLI_CLASS(ConditionBase, ConditionBase, "Always True")
YASLI_CLASS_NULL(ConditionBase, 0) // remove NULL value from the list of available types

class ConditionSwitch : public ConditionBase
{
public:
	enum Mode{
		AND,
		OR
	};

	ConditionSwitch()
	: mode_(AND)
	{
	}

	virtual void serialize(Archive& ar)
	{
		ConditionBase::serialize(ar);
		ar(mode_, "mode", "Mode");
		ar(children_, "children", "^Subconditions");
	}

	std::vector<SharedPtr<ConditionBase> > children_;
private:
	Mode mode_;
};

YASLI_CLASS(ConditionBase, ConditionSwitch, "And / Or")

YASLI_ENUM_BEGIN_NESTED(ConditionSwitch, Mode, "Mode")
YASLI_ENUM_VALUE_NESTED(ConditionSwitch, AND, "And")
YASLI_ENUM_VALUE_NESTED(ConditionSwitch, OR, "Or")
YASLI_ENUM_END()

class ConditionCheckFileAttributes : public ConditionBase
{
public:
	ConditionCheckFileAttributes()
	{
	}

	virtual void serialize(Archive& ar)
	{
		ConditionBase::serialize(ar);
		ar(yasli::FileOpen(filename_, "*.*", "."), "filename", "<Filename");
		ar(isDirectory_, "isDirectory", "Directory");
		ar(isReadOnly_, "isReadOnly", "Read Only");
	}
private:
	std::string filename_;
	bool isDirectory_;
	bool isReadOnly_;
};
YASLI_CLASS(ConditionBase, ConditionCheckFileAttributes, "Check File Attributes")

// ---------------------------------------------------------------------------

class ActionBase : public yasli::RefCounter
{
public:
	virtual ~ActionBase() {}
	virtual void act() = 0;
	virtual void serialize(Archive &ar) {}
};
YASLI_CLASS_NULL(ActionBase, "Do Nothing")

class ActionMessageBox : public ActionBase
{
public:
	enum Icon{
		NO_ICON = QMessageBox::NoIcon,
		INFORMATION = QMessageBox::Information,
		QUESTION = QMessageBox::Question,
		EXCLAMATION = QMessageBox::Warning,
		ERROR = QMessageBox::Critical
	};

	ActionMessageBox()
	: icon_(NO_ICON)
	, message_("Example Message")
	{
	}

	void serialize(Archive& ar)
	{
		ar(icon_, "icon", "Icon");
		ar(message_, "message", "<Message");

		if (ar.isEdit())
		{
			yasli::Button button("Test");
			ar(button, "test", "<");
			if (button)
			{
				act();
			}
		}
		
	}

	void act()
	{
		QMessageBox icon((QMessageBox::Icon)icon_, message_.c_str(), "Message");
		icon.exec();
	}
private:
	Icon icon_;
	std::string message_;
};
YASLI_CLASS(ActionBase, ActionMessageBox, "Show Message")

YASLI_ENUM_BEGIN_NESTED(ActionMessageBox, Icon, "Icon")
YASLI_ENUM_VALUE_NESTED(ActionMessageBox, NO_ICON, "No Icon")
YASLI_ENUM_VALUE_NESTED(ActionMessageBox, INFORMATION, "Information")
YASLI_ENUM_VALUE_NESTED(ActionMessageBox, QUESTION, "Question")
YASLI_ENUM_VALUE_NESTED(ActionMessageBox, EXCLAMATION, "Exclamation")
YASLI_ENUM_VALUE_NESTED(ActionMessageBox, ERROR, "Error")
YASLI_ENUM_END()

class ActionSequence : public ActionBase
{
public:
	void serialize(Archive& ar)
	{
		ar(actions_, "actions", "^Actions");
	}
	void act()
	{
		for (size_t i = 0; i < actions_.size(); ++i)
			if (actions_[i])
				actions_[i]->act();
	}

	std::vector<SharedPtr<ActionBase> > actions_;
};
YASLI_CLASS(ActionBase, ActionSequence, "Sequence");

class ActionPaintSky : public ActionBase
{
public:
	ActionPaintSky()
	: zenithColor_(30, 0, 255, 192)
	, horizonColor_(255, 240, 240, 192)
	, showSun_(false)
	, sunColor_(255, 200, 0)
	{}

	void serialize(Archive &ar)
	{
		ar(zenithColor_, "zenithColor", "Zenith Color");
		ar(horizonColor_, "horizonColor", "Horizon Color");
		ar(showSun_, "showSun", "Show Sun");
		if (showSun_)
			ar(sunColor_, "sunColor", "Sun Color");
	}

	void act()
	{
		// look outside!
	}
private:
	Color zenithColor_;
	Color horizonColor_;
	Color sunColor_;
	bool showSun_;
};
YASLI_CLASS(ActionBase, ActionPaintSky, "Paint Sky");


class ActionSmile : public ActionBase
{
public:
	ActionSmile()
	: howWide_(0.5f)
	, withTeeth_(false)
	{}

	void serialize(Archive &ar)
	{
		ar(yasli::Range(howWide_, 0.0f, 1.0f), "howWide", "How Wide?");
		ar(withTeeth_, "withTeeth", "With Teeth");
	}

	void act()
	{
		// look outside!
	}
private:
	float howWide_;
	bool withTeeth_;
};
YASLI_CLASS(ActionBase, ActionSmile, "Smile");

struct PolymorphicTreeData
{
	void serialize(Archive& ar)
	{
		ar(condition_, "condition", "<Condition");
		ar(yasli::HorizontalLine(), "", "<");
		ar(action_, "action", "<Action");
	}

	PolymorphicTreeData()
	{
		ConditionSwitch* cond = new ConditionSwitch();
		cond->children_.push_back(new ConditionCheckFileAttributes());
		cond->children_.push_back(new ConditionBase());
		condition_ = cond;

		ActionSequence* seq = new ActionSequence();
		seq->actions_.push_back(new ActionMessageBox());
		seq->actions_.push_back(new ActionPaintSky());
		seq->actions_.push_back(new ActionSmile());
		action_ = seq;
	}

	SharedPtr<ConditionBase> condition_;
	SharedPtr<ActionBase> action_;
} polymorphicTreeData;

QWidget* createExamplePolymorphicTree()
{
	QPropertyTree* propertyTree = new QPropertyTree();

	propertyTree->setUndoEnabled(true, false);
	propertyTree->attach(yasli::Serializer(polymorphicTreeData));
	propertyTree->expandAll();

	return propertyTree;
}

