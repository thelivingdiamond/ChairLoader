#pragma once
#include <QObject>
#include "PropertyTree/PropertyRow.h"

class PropertyRowFileOpen;
class PropertyTree;

struct FileOpenMenuHandler : QObject, PropertyRowMenuHandler
{
Q_OBJECT
public:
	PropertyRowFileOpen* self;
	PropertyTree* tree;

	FileOpenMenuHandler(PropertyRowFileOpen* self, PropertyTree* tree)
	: self(self)
	, tree(tree)
	{
	}

public slots:
	void onMenuActivate();
	void onMenuClear();
};

std::string extractExtensionFromFilter(const char* fileSelectorFilter);
