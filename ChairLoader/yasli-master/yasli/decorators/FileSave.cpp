#include "yasli/decorators/FileSave.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

namespace yasli {

void FileSave::YASLI_SERIALIZE_METHOD(Archive& ar)
{
	ar(path, "path");
	ar(filter, "filter");
	ar(relativeToFolder, "folder");
}

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, FileSave& value, const char* name, const char* label)
{
	if (ar.isEdit())
		return ar(Serializer(value), name, label);
	else
		return ar(value.path, name, label);
}

}

