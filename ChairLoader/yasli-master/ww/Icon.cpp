#include "stdafx.h"
#include "Icon.h"
#include "yasli/Archive.h"

namespace ww {

void IconToggle::serialize(yasli::Archive& ar)
{
	ar(value_, "value", "Value");
}

// ---------------------------------------------------------------------------

bool Icon::getImage(RGBAImage* image) const
{
	if (lineCount_ < 3) {
		return false;
	}

	// parse values
	std::vector<Color> pixels;
	int width = 0;
	int height = 0;
	int charsPerPixel = 0;
	int colorCount = 0;
	int hotSpotX = -1;
	int hotSpotY = -1;

	int scanResult = sscanf_s(source_[0], "%d %d %d %d %d %d", &width, &height, &colorCount, &charsPerPixel, &hotSpotX, &hotSpotY);
	if (scanResult != 4 && scanResult != 6)
		return false;

	if (charsPerPixel > 4)
		return false;

	if(lineCount_ != 1 + colorCount + height) {
		YASLI_ASSERT(0 && "Wrong line count");
		return false;
	}

	// parse colors
	std::vector<std::pair<int, Color> > colors;
	colors.resize(colorCount);

	for (int colorIndex = 0; colorIndex < colorCount; ++colorIndex) {
		const char* p = source_[colorIndex + 1];
		int code = 0;
		for (int charIndex = 0; charIndex < charsPerPixel; ++charIndex) {
			if (*p == '\0')
				return false;
			code = (code << 8) | *p;
			++p;
		}
		colors[colorIndex].first = code;

		while (*p == '\t' || *p == ' ')
			++p;

		if (*p == '\0')
			return false;

		if (*p != 'c' && *p != 'g')
			return false;
		++p;

		while (*p == '\t' || *p == ' ')
			++p;

		if (*p == '\0')
			return false;

		if (*p == '#') {
			++p;
			if (strlen(p) == 6) {
				int colorCode;
				if(sscanf_s(p, "%x", &colorCode) != 1)
					return false;
				Color color((colorCode & 0xff0000) >> 16,
							(colorCode & 0xff00) >> 8,
							(colorCode & 0xff),
							255);
				colors[colorIndex].second = color;
			}
		}
		else {
			if(_stricmp(p, "None") == 0)
				colors[colorIndex].second = Color(0, 0, 0, 0);
			else if (_stricmp(p, "Black") == 0)
				colors[colorIndex].second.setGDI(GetSysColor(COLOR_BTNTEXT));
			else if (_stricmp(p, "DarkGray") == 0)
				colors[colorIndex].second.setGDI(GetSysColor(COLOR_3DSHADOW));
			else {
				// unknown color
				colors[colorIndex].second = Color(255, 0, 0, 255);
			}
		}
	}

	// parse pixels
	pixels.resize(width * height);
	int pi = 0;
	for (int y = 0; y < height; ++y) {
		const char* p = source_[1 + colorCount + y];
		if (strlen(p) != width * charsPerPixel)
			return false;

		for (int x = 0; x < width; ++x) {
			int code = 0;
			for (int i = 0; i < charsPerPixel; ++i) {
				code = (code << 8) | *p;
				++p;
			}

			for (size_t i = 0; i < size_t(colorCount); ++i)
				if (colors[i].first == code)
					pixels[pi] = colors[i].second;
			++pi;
		}
	}
	
	image->pixels_.swap(pixels);
	image->width_ = width;
	image->height_ = height;
	return true;
}

}
