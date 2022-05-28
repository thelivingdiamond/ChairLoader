#pragma once

#include <vector>
#include "XMath\Colors.h"
#include "yasli\Archive.h"

typedef std::vector<Color4f> ColorContainer;

class ComboListColor
{
public:
    ComboListColor(const ColorContainer& comboList, const Color4f& value)
	: comboList_(comboList){
        index_ = indexOf(value);
	}
	ComboListColor() {
		index_ = 0;
	}
    ComboListColor& operator=(const Color4f& value) { index_ = indexOf(value); return *this; }

    operator const Color4f&() const { return value(); }
    const ColorContainer& comboList() const { return comboList_; }
    void setComboList(const ColorContainer& comboList) { comboList_ = comboList; }

	const Color4f& value() const{
		if(size_t(index_) < comboList().size())
			return comboList()[index_];
		else
			return Color4f::WHITE;
	}

	int index() const{ return index_; }
	int indexOf(Color4f value) const{
		for(int i = 0; i < int(comboList_.size()); ++i){
			if(comboList_[i] == value)
				return i;
		}
		return 0;
	}
	void setIndex(int index){
		index_ = index;
	}
	void serialize (yasli::Archive& ar);
private:
	int index_;
    ColorContainer comboList_;
};