#pragma once

#include "yasli/Assert.h"
#include "yasli/StringList.h"
#include "yasli/STL.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/PointersImpl.h"
#include "yasli/STLImpl.h"
#include "yasli/TextOArchive.h"
#include <string>

using namespace yasli;

#define YCHECK(x) YASLI_ASSERT(x); CHECK(x)

#ifdef _MSC_VER
# pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned
#endif

struct Member
{
	std::string name;
	std::wstring wname;
	float weight;

	Member()
	: weight(0.0f) {}

	void checkEquality(const Member& copy) const
	{
		YCHECK(name == copy.name);
		YCHECK(wname == copy.wname);
		YCHECK(weight == copy.weight);
	}

	void change(int index)
	{
		name = "Changed name ";
		name += (index % 10) + '0';
		wname = L"New unique name";
		wname += (index % 10) + L'0';
		weight = float(index);
	}

	void YASLI_SERIALIZE_METHOD(Archive& ar)
	{
		ar(name, "name");
		ar(wname, "wname");
		ar(weight, "weight");
	}
};

class PolyBase : public RefCounter
{
public:
	PolyBase()
	{
		baseMember_ = "Regular base member";
	}

	virtual void change()
	{
		baseMember_ = "Changed base member";
	}

	virtual void YASLI_SERIALIZE_METHOD(Archive& ar)
	{		
		ar(baseMember_, "baseMember");
	}

	virtual void checkEquality(const PolyBase* copy) const
	{
		YCHECK(baseMember_ == copy->baseMember_);
	}
protected:
	std::string baseMember_;
};

class PolyDerivedA : public PolyBase
{
public:
	void YASLI_SERIALIZE_METHOD(Archive& ar)
	{		
		PolyBase::YASLI_SERIALIZE_METHOD(ar);
		ar(derivedMember_, "derivedMember");
	}

	void checkEquality(const PolyBase* copyBase) const
	{
		const PolyDerivedA* copy = dynamic_cast<const PolyDerivedA*>(copyBase);
        YCHECK(copy != 0);
		YCHECK(derivedMember_ == copy->derivedMember_);

		PolyBase::checkEquality(copyBase);
	}
protected:
	std::string derivedMember_;
};

class PolyDerivedB : public PolyBase
{
public:
	PolyDerivedB()
	: derivedMember_("B Derived")
	{
	}

	void YASLI_SERIALIZE_METHOD(Archive& ar)
	{		
		PolyBase::YASLI_SERIALIZE_METHOD(ar);
		ar(derivedMember_, "derivedMember");
		ar(derivedWMember_, "derivedWMember");
	}

	void checkEquality(const PolyBase* copyBase) const
	{
		const PolyDerivedB* copy = dynamic_cast<const PolyDerivedB*>(copyBase);
		YCHECK(copy != 0);
		YCHECK(derivedMember_ == copy->derivedMember_);
		YCHECK(derivedWMember_ == copy->derivedWMember_);

		PolyBase::checkEquality(copyBase);
	}
protected:
	std::string derivedMember_;
	std::wstring derivedWMember_;
};

struct NumericTypes
{
  NumericTypes()
  : bool_(false)
  , char_(0)
  , sChar_(0)
  , uChar_(0)
  , int_(0)
  , uInt_(0)
  , long_(0)
  , uLong_(0)
  , longLong_(0)
  , uLongLong_(0)
  , float_(0.0f)
  , double_(0.0)
  {
  }

  void change()
  {
    bool_ = true;
    char_ = -1;
    sChar_ = -2;
    uChar_ = 0xff - 3;
    int_ = -4;
    uInt_ = -5;
    long_ = -6l;
    uLong_ = -7ul;
    longLong_ = -8ll;
    uLongLong_ = -9ull;
    float_ = -10.0f;
    double_ = -11.0;
  }

  void YASLI_SERIALIZE_METHOD(Archive& ar)
  {
    ar(bool_, "bool");
    ar(char_, "char");
    ar(sChar_, "signed_char");
    ar(uChar_, "unsigned_char");
    ar(int_, "int");
    ar(uInt_, "unsigned_int");
    ar(long_, "long");
    ar(uLong_, "unsigned_long");
    ar(longLong_, "long_long");
    ar(uLongLong_, "unsigned_long_long");
    ar(float_, "float");
    ar(double_, "double");
  }

  void checkEquality(const NumericTypes& rhs) const
  {
    YCHECK(bool_ == rhs.bool_);
    YCHECK(char_ == rhs.char_);
    YCHECK(sChar_ == rhs.sChar_);
    YCHECK(uChar_ == rhs.uChar_);
    YCHECK(int_ == rhs.int_);
    YCHECK(uInt_ == rhs.uInt_);
    YCHECK(long_ == rhs.long_);
    YCHECK(uLong_ == rhs.uLong_);
    YCHECK(longLong_ == rhs.longLong_);
    YCHECK(uLongLong_ == rhs.uLongLong_);
    YCHECK(float_ == rhs.float_);
    YCHECK(double_ == rhs.double_);
  }

  bool bool_;

  char char_;
  signed char sChar_;
  unsigned char uChar_;

	int int_;
	unsigned int uInt_;

  long long_;
  unsigned long uLong_;

  long long longLong_;
  unsigned long long uLongLong_;

  float float_;
  double double_;
};

class ComplexClass{
public:
	ComplexClass()
	: index_(0)
	{
		name_ = "Foo";
		wname_ = L"Bar";
		stringList_.push_back("Choice 1");
		stringList_.push_back("Choice 2");
		stringList_.push_back("Choice 3");
		stringList_.push_back("Choice 4");
		stringList_.push_back("Choice 5");
		stringList_.push_back("Choice 6");
		stringList_.push_back("Choice 7");
		stringList_.push_back("Choice 8");

		polyPtr_.reset( new PolyDerivedA() );

		polyVector_.push_back(new PolyDerivedB);
		polyVector_.push_back(new PolyBase);

		Member& a = stringToStructMap_["a"];
		a.name = "A";
		Member& b = stringToStructMap_["b"];
		b.name = "B";

		members_.resize(13);

		intToString_.push_back(std::make_pair(1, "one"));
		intToString_.push_back(std::make_pair(2, "two"));
		intToString_.push_back(std::make_pair(3, "three"));
		stringToInt_.push_back(std::make_pair("one", 1));
		stringToInt_.push_back(std::make_pair("two", 2));
		stringToInt_.push_back(std::make_pair("three", 3));
	}

	void change()
	{
		name_ = "Slightly changed name";
		wname_ = L"Another changed string";
		index_ = 2;
		polyPtr_.reset( new PolyDerivedB() );
		polyPtr_->change();

		for (size_t i = 0; i < members_.size(); ++i)
			members_[i].change(int(i));

		members_.erase(members_.begin());

		for (size_t i = 0; i < polyVector_.size(); ++i)
			polyVector_[i]->change();

		polyVector_.resize(4);
		polyVector_.push_back(new PolyBase());
		polyVector_[4]->change();

		const size_t arrayLen = sizeof(array_) / sizeof(array_[0]);
		for (size_t i = 0; i < arrayLen; ++i)
			array_[i].change(int(arrayLen - i));

		numericTypes_.change();

		vectorOfStrings_.push_back("str1");
		vectorOfStrings_.push_back("2str");
		vectorOfStrings_.push_back("thirdstr");
		
		stringToStructMap_.erase("a");
		Member& c = stringToStructMap_["c"];
		c.name = "C";

		intToString_.push_back(std::make_pair(4, "four"));
		stringToInt_.push_back(std::make_pair("four", 4));
	}

	void YASLI_SERIALIZE_METHOD(Archive& ar)
	{
		ar(name_, "name");
		ar(wname_, "wname");
		ar(polyPtr_, "polyPtr");
		ar(polyVector_, "polyVector");
		ar(members_, "members");

		StringListValue value(stringList_, stringList_[index_]);
		ar(value, "stringList");
		index_ = value.index();
		if(index_ == -1)
			index_ = 0;

		ar(array_, "array");
		ar(numericTypes_, "numericTypes");
		ar(vectorOfStrings_, "vectorOfStrings");
		ar(intToString_, "intToString");
		ar(stringToInt_, "stringToInt");
	}

	void checkEquality(const ComplexClass& copy) const
	{
		YCHECK(name_ == copy.name_);
		YCHECK(wname_ == copy.wname_);
		YCHECK(index_ == copy.index_);

		YCHECK(polyPtr_ != 0);
		YCHECK(copy.polyPtr_ != 0);
		polyPtr_->checkEquality(copy.polyPtr_);

		YCHECK(members_.size() == copy.members_.size());
		for (size_t i = 0; i < members_.size(); ++i)
		{
			members_[i].checkEquality(copy.members_[i]);
		}

		YCHECK(polyVector_.size() == copy.polyVector_.size());
		for (size_t i = 0; i < polyVector_.size(); ++i)
		{
			if(polyVector_[i] == 0)
			{
				YCHECK(copy.polyVector_[i] == 0);
				continue;
			}
			YCHECK(copy.polyVector_[i] != 0);
			polyVector_[i]->checkEquality(copy.polyVector_[i]);
		}

		const size_t arrayLen = sizeof(array_) / sizeof(array_[0]);
		for (size_t i = 0; i < arrayLen; ++i)
		{
			array_[i].checkEquality(copy.array_[i]);
		}

		numericTypes_.checkEquality(copy.numericTypes_);

		YCHECK(intToString_.size() == copy.intToString_.size());
		for (size_t i = 0; i < intToString_.size(); ++i)
		{
			YCHECK(intToString_[i] == copy.intToString_[i]);
		}

		YCHECK(stringToInt_.size() == copy.stringToInt_.size());
		for (size_t i = 0; i < stringToInt_.size(); ++i)
		{
			YCHECK(stringToInt_[i] == copy.stringToInt_[i]);
		}
	}
protected:
	std::string name_;
	std::wstring wname_;
	typedef std::vector<Member> Members;
	std::vector<std::string> vectorOfStrings_;
	std::vector<std::pair<int, string> > intToString_;
	std::vector<std::pair<string, int> > stringToInt_;
	Members members_;
	int index_;
  NumericTypes numericTypes_;

	StringListStatic stringList_;
	std::vector< SharedPtr<PolyBase> > polyVector_;
	SharedPtr<PolyBase> polyPtr_;

	std::map<std::string, Member> stringToStructMap_;

	Member array_[5];
};

