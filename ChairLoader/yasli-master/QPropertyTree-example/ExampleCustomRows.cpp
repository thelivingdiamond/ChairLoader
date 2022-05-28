#include <string>
#include <vector>
using std::vector;

#include "QPropertyTree/QPropertyTree.h"
#include "PropertyTree/Color.h"

#include "yasli/decorators/IconXPM.h"
#include "yasli/decorators/HorizontalLine.h"
#include "yasli/decorators/BitFlags.h"
#include "yasli/decorators/Button.h"
#include "yasli/decorators/Range.h"
#include "yasli/decorators/FileOpen.h"
#include "yasli/decorators/FileSave.h"

using yasli::Archive;
using yasli::SharedPtr;


enum Flags
{
	FLAG_FIRST = 1 << 0,
	FLAG_SECOND = 1 << 1,
	FLAG_THIRD = 1 << 2,
	FLAG_ALL = FLAG_FIRST | FLAG_SECOND | FLAG_THIRD
};

YASLI_ENUM_BEGIN(Flags, "Flags")
YASLI_ENUM(FLAG_FIRST, "first", "First")
YASLI_ENUM(FLAG_SECOND, "second", "Second")
YASLI_ENUM(FLAG_THIRD, "third", "Third")
YASLI_ENUM(FLAG_ALL, "all", "All")
YASLI_ENUM_END()

struct CustomRows
{
	int bitFlags;

	yasli::string fileSelector;
	bool buttonState;
	property_tree::Color propertyTreeColor;
	float rangeF;
	int rangeI;
	std::string fileOpen;
	std::string fileSave;

	CustomRows()
	: bitFlags(FLAG_FIRST | FLAG_THIRD)
	, fileSelector("test_file.txt")
	, buttonState(false)
	, propertyTreeColor(255, 0, 0, 128)
	, rangeF(0.5f)
	, rangeI(50)
	, fileOpen("file_to_open.txt")
	, fileSave("file_to_save.txt")
	{

	}

	void serialize(Archive& ar)
	{
		#include "Icons/favourites.xpm"
		ar(yasli::IconXPM(favourites_xpm), "icon", "yasli::IconXPM");

		// new way to serialize bit flags, less intrusive
		ar(yasli::BitFlags<Flags>(bitFlags), "bitFlags", "yasli::BitFlags");
			
		ar(propertyTreeColor, "propertyTreeColor", "property_tree::Color");

		ar(yasli::Range(rangeF, 0.0f, 1.0f), "rangeF", "yasli::Range (float)");
		ar(yasli::Range(rangeI, 0, 100), "rangeI", "yasli::Range (int)");
		ar(yasli::FileOpen(fileOpen, "Text Files (*.txt);;All Files (*.*)", "."), "fileOpen", "yasli::File Open");
		ar(yasli::FileSave(fileSave, "Text Files (*.txt);;All Files (*.*)", "."), "fileSave", "yasli::File Save");
		ar(yasli::HorizontalLine(), "yasliHline", "<yasli::HorizontalLine");
		ar(yasli::Button("yasli::Button"), "yasliButton", "<");
	}

} customRows;

QWidget* createExampleCustomRows()
{
	QPropertyTree* propertyTree = new QPropertyTree();

	propertyTree->setExpandLevels(1);
	propertyTree->attach(yasli::Serializer(customRows));
	propertyTree->expandAll();

	return propertyTree;
}
