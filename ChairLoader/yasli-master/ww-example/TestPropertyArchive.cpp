#include "UnitTest++.h"
#include "../yasli/ComplexClass.h"

#include "ww/PropertyTreeModel.h"
#include "ww/PropertyOArchive.h"
#include "ww/PropertyIArchive.h"

//using std::string;
using namespace yasli;
using namespace ww;

SUITE(PropertyArchive)
{
	TEST(ComplexSaveAndLoad)
	{
		PropertyTreeModel model;

		ComplexClass objChanged;
		objChanged.change();
		{
			PropertyOArchive oa(&model);
			Archive& ar = oa;
			CHECK(ar(objChanged, "obj", "Object"));
		}

		{
			ComplexClass obj;

			PropertyIArchive ia(&model);
			Archive& ar = ia;
			CHECK(ar(obj, "obj", "Object"));

 			obj.checkEquality(objChanged);
		}
	}


	struct StructWithHiddenField
	{
		string field1_;
		string field2_;
		string field3_;
		bool show;

		StructWithHiddenField()
		: show(false)
		{
		}

		void serialize(Archive& ar)
		{
			ar(field1_, "field1", "Field 1");
			if(show)
				ar(field2_, "field2", "Field 2");
			ar(field3_, "field3", "Field 3");
		}
	};

	static bool testRowOrder(PropertyRow* root, const char* name1, const char* name2)
	{
		int index1 = root->childIndex(root->find(name1, 0, 0));
		int index2 = root->childIndex(root->find(name2, 0, 0));
		return index1 < index2;
	}

	TEST(RegressionFieldReordering)
	{
		PropertyTreeModel model;

		{
			StructWithHiddenField data;
			data.show = false;

			PropertyOArchive oa(&model);
			Archive& ar = oa;
			ar(data, "data", "Data");
		}

		{
			StructWithHiddenField data;
			data.show = true;

			PropertyOArchive oa(&model);
			Archive& ar = oa;
			ar(data, "data", "Data");
		}

		CHECK(testRowOrder(model.root(), "field1", "field2"));
		CHECK(testRowOrder(model.root(), "field2", "field3"));
	}
}

