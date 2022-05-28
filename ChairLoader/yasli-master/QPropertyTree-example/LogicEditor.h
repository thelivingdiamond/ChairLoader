#pragma once

#include "yasli/Pointers.h"
using yasli::SharedPtr;
namespace yasli { class Archive; }
using yasli::Archive;

class ConditionBase;
class ActionBase;

struct LogicEditorData
{
	SharedPtr<ConditionBase> condition_;
	SharedPtr<ActionBase> action_;

	LogicEditorData();
	void serialize(Archive& ar);
};
extern LogicEditorData globalLogicEditor;
