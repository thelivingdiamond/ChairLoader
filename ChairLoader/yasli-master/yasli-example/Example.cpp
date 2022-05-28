#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/StringList.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"

#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/Enum.h"

using namespace yasli;

enum MemberMode{
	MEMBER_MODE_A,
	MEMBER_MODE_B,
	MEMBER_MODE_C
};

YASLI_ENUM_BEGIN(MemberMode, "MemberMode")
YASLI_ENUM_VALUE(MEMBER_MODE_A, "Mode A")
YASLI_ENUM_VALUE(MEMBER_MODE_B, "Mode B")
YASLI_ENUM_VALUE(MEMBER_MODE_C, "Mode C")
YASLI_ENUM_END()

struct Member{
	std::string name;
	float weight;
	MemberMode mode;
	char memberWord[8];

	Member()
    : weight(rand() * 100.0f / RAND_MAX)
	, mode(MEMBER_MODE_A) 
	{
		strcpy(memberWord, "memberW");
		name = "member_name";
	}

	Member(const char* _name, float _weight, MemberMode _mode)
	: name(_name)
	, weight(_weight)
	, mode(_mode) 
	{
		strcpy(memberWord, "memberW");
	}

	void serialize(Archive& ar)
	{
		ar(name, "name");
		ar(weight, "weight");
		ar(mode, "mode");
	}
};

class PolyBase : public RefCounter
{
public:
	PolyBase()
	{
        baseMember_= "Very nice\nand long\nMember";
	}

	virtual void serialize(Archive& ar)
	{		
		ar(baseMember_, "baseMember");
	}
protected:
	std::string baseMember_;
};

class PolyDerivedA : public PolyBase
{
public:
	void serialize(Archive& ar)
	{		
		PolyBase::serialize(ar);
		ar(derivedMember_, "derivedMember");
	}
protected:
	std::string derivedMember_;
};


class MyDataClass{
public:
	MyDataClass()
	: index_(0)
	{
		// we are going to fill in default values
		name_ = L"Foo, that is a name long enough to be allocated on heap";
		stringList_.push_back("Choice 1");
		stringList_.push_back("Choice 2");
		stringList_.push_back("Choice 3");
		stringList_.push_back("Choice 4");
		stringList_.push_back("Choice 5");
		stringList_.push_back("Choice 6");
		stringList_.push_back("Choice 7");
		stringList_.push_back("Choice 8");

        polyPtr_.reset( new PolyDerivedA() );

		members_.push_back(Member("Item A", 0.1f, MEMBER_MODE_A));
		members_.push_back(Member("Item B", 0.3f, MEMBER_MODE_B));
		members_.push_back(Member("Item C", 0.6f, MEMBER_MODE_C));
		floatValues_.resize(200, 0.0f);
		for(size_t i = 0; i < floatValues_.size(); ++i)
			floatValues_[i] = rand() * 1000.0f / RAND_MAX;
		polyVector_.push_back( new PolyBase() );

		strcpy(endBuf_, "END");
	}
	void serialize(Archive& ar){
		ar(name_);
		ar(polyPtr_, "polyPtr");
		ar(polyVector_, "polyVector");
		ar(members_, "members");

		size_t s = floatValues_.size();
		ar(s, "numFloatValues");
		if(ar.isInput())
			floatValues_.reserve(s);
		//ar(floatValues_, "floatValues");

		StringListValue value(stringList_, stringList_[index_]);
		ar(value, "stringList");
		index_ = value.index();
		if(index_ == -1)
			index_ = 0;
	}
protected:
	std::wstring name_;
	typedef std::vector<Member> Members;
	Members members_;
	int index_;
	StringListStatic stringList_;
	std::vector< SharedPtr<PolyBase> > polyVector_;
	std::vector< float > floatValues_;
	SharedPtr<PolyBase> polyPtr_;
	char endBuf_[4];
};

struct MyDataContainer
{
	MyDataContainer()
	{
		objects.resize(10000);
	}

	void serialize(Archive& ar)
	{
		ar(objects, "objects");
	}
	std::vector<MyDataClass> objects;
};

YASLI_CLASS(PolyBase, PolyBase, "Base")
YASLI_CLASS(PolyBase, PolyDerivedA, "Derived A")

struct AutoTimer
{
	unsigned int startTime_;
	const char* name_;

	AutoTimer(const char* name)
	: name_(name)
	{
		startTime_ = clock() * 1000 / CLOCKS_PER_SEC;
	}

	int result() const
	{
		return clock() * 1000 / CLOCKS_PER_SEC - startTime_;
	}

	~AutoTimer()
	{
		printf("%s: %i\n", name_, result());
	}
};

int getFileSize(const char* filename)
{
#ifdef WIN32
	struct _stat64 desc;
	if (_stat64(filename, &desc) != 0)
    return -1;
#else
  struct stat desc;
  if (stat(filename, &desc) != 0)
    return -1;
#endif

  return (int)desc.st_size;
}

void testText()
{
		MyDataClass obj;
		TextIArchive ia;
		if(ia.load("test.ta"))
			ia(obj, "obj");

		TextOArchive oa;
		oa(obj, "obj");
		oa.save("test.ta");
}

int main(int argc, char** argv)
{
	testText();

	return 0;
}
