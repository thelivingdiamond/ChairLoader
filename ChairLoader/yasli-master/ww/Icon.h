#pragma once

#include "ww/Color.h"

namespace yasli {
	class Archive;
}

namespace ww {

struct RGBAImage
{
	int width_;
	int height_;
	std::vector<Color> pixels_;

	RGBAImage() : width_(0), height_(0) {}
};


// Icon, stored in XPM format
class Icon
{
public:
	Icon()
	: source_(0)
	, lineCount_(0)
	, width_(0)
	, height_(0)
	{
	}

	template<size_t Size>
	explicit Icon(char* (&xpm)[Size])
	: width_(0)
	, height_(0)
	{
		set(xpm, Size);
	}

	template<size_t Size>
	explicit Icon(const char* (&xpm)[Size])
		: width_(0)
		, height_(0)
	{
		set(xpm, Size);
	}


	bool getImage(RGBAImage* out) const;
	void serialize(yasli::Archive& ar) {}
	bool operator<(const Icon& rhs) const { return source_ < rhs.source_; }

	int width() const{ return width_; }
	int height() const{ return height_; }

	const char* const* source() const { return source_;  }
	int lineCount() const { return (int)lineCount_;  }

	void set(const char* const* source, size_t lineCount)
	{
		source_ = source;
		lineCount_ = lineCount;
		if (!source_ || sscanf_s(source[0], "%d %d", &width_, &height_) != 2) {
			width_ = 0;
			height_ = 0;
		}
	}
private:

	const char* const* source_;
	size_t lineCount_;
	int width_;
	int height_;
};

struct IconToggle
{
	bool* variable_;
	bool value_;
	Icon iconTrue_;
	Icon iconFalse_;

	template<size_t Size1, size_t Size2>
	IconToggle(bool& variable, char* (&xpmTrue)[Size1], char* (&xpmFalse)[Size2])
	: iconTrue_(xpmTrue)
	, iconFalse_(xpmFalse)
	, variable_(&variable)
	, value_(variable)
	{
	}

	template<size_t Size1, size_t Size2>
	IconToggle(bool& variable, const char* (&xpmTrue)[Size1], const char* (&xpmFalse)[Size2])
		: iconTrue_(xpmTrue)
		, iconFalse_(xpmFalse)
		, variable_(&variable)
		, value_(variable)
	{
	}

	IconToggle(bool& variable, const Icon& iconTrue, const Icon& iconFalse)
	: iconTrue_(iconTrue)
	, iconFalse_(iconFalse)
	, variable_(&variable)
	, value_(variable)
	{
	}

	IconToggle& operator=(const IconToggle& orig)
	{
		value_ = orig.value_;
		if (!variable_) {
			iconTrue_ = orig.iconTrue_;
			iconFalse_ = orig.iconFalse_;
		}
		return *this;
	}

	IconToggle()
	: variable_(0)
	{
	}
	~IconToggle()
	{
		if (variable_)
			*variable_ = value_;
	}

	void serialize(yasli::Archive& ar);
};

}
