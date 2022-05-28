#include <windows.h>
#undef ERROR

#include "ww/PropertyTree.h"

#include "ww/Serialization.h"
#include "ww/Decorators.h"
#include "ww/SliderDecorator.h"
#include "ww/FileSelector.h"
#include "ww/Color.h"
#include "ww/Unicode.h"
#include "yasli/Enum.h"

using yasli::Archive;
using yasli::SharedPtr;

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
		ar(ww::NotDecorator(isNegated_), "not", "^^");
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
		__super::serialize(ar);
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
		__super::serialize(ar);
		static ww::FileSelector::Options options("*.*", false);
		ar(ww::FileSelector(filename_, options), "filename", "<Filename");
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
	virtual void act() = 0;
	virtual void serialize(Archive &ar) {}
};
YASLI_CLASS_NULL(ActionBase, "Do Nothing")

class ActionMessageBox : public ActionBase
{
public:
	enum Icon{
		NO_ICON = 0,
		INFORMATION = MB_ICONINFORMATION,
		QUESTION = MB_ICONQUESTION,
		EXCLAMATION = MB_ICONEXCLAMATION,
		ERROR = MB_ICONERROR
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
			ww::ButtonDecorator button("Test");
			ar(button, "test", "<");
			if (button)
			{
				act();
			}
		}
		
	}

	void act()
	{
		MessageBoxW(GetActiveWindow(), ww::toWideChar(message_.c_str()).c_str(), L"Message", MB_OK | int(icon_));
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
	ww::Color zenithColor_;
	ww::Color horizonColor_;
	ww::Color sunColor_;
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
		ar(ww::SliderDecoratorf(howWide_, 0.0f, 1.0f), "howWide", "How Wide?");
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
		ar(ww::HLineDecorator(), "", "<");
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
	} conditionEditorData;

ww::Widget* createPolymorphicTree()
{
	ww::PropertyTree* propertyTree = new ww::PropertyTree();

	propertyTree->setUndoEnabled(true, false);
	propertyTree->attach(yasli::Serializer(conditionEditorData));
	propertyTree->expandAll();

	return propertyTree;
}

