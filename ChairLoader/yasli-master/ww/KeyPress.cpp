/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "stdafx.h"
#include "KeyPress.h"
#include "yasli/BitVector.h"
#include "yasli/Archive.h"

namespace ww{

const char* toHex(unsigned char byte)
{
	static char char_values[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static char byteBuf[3];
	byteBuf[0] = char_values[byte >> 4];
	byteBuf[1] = char_values[byte & 0x0F];
	byteBuf[2] = '\0';
	return byteBuf;
}

YASLI_ENUM_BEGIN(Key, "Key")
YASLI_ENUM_VALUE(KEY_NONE				 , "None")
YASLI_ENUM_VALUE(KEY_LBUTTON             , "LButton")
YASLI_ENUM_VALUE(KEY_RBUTTON             , "RButton")
YASLI_ENUM_VALUE(KEY_CANCEL              , "Cancel")
YASLI_ENUM_VALUE(KEY_MBUTTON             , "MButton")
YASLI_ENUM_VALUE(KEY_XBUTTON1            , "XButton1")
YASLI_ENUM_VALUE(KEY_XBUTTON2            , "XButton2")
YASLI_ENUM_VALUE(KEY_BACK                , "Back")
YASLI_ENUM_VALUE(KEY_TAB                 , "Tab")
YASLI_ENUM_VALUE(KEY_CLEAR               , "Clear")
YASLI_ENUM_VALUE(KEY_RETURN              , "Enter")
YASLI_ENUM_VALUE(KEY_SHIFT               , "Shift")
YASLI_ENUM_VALUE(KEY_CONTROL             , "Control")
YASLI_ENUM_VALUE(KEY_MENU                , "Alt")
YASLI_ENUM_VALUE(KEY_PAUSE               , "Pause")
YASLI_ENUM_VALUE(KEY_CAPITAL             , "Capital")
YASLI_ENUM_VALUE(KEY_KANA                , "Kana")
YASLI_ENUM_VALUE(KEY_HANGEUL             , "Hangeul")
YASLI_ENUM_VALUE(KEY_HANGUL              , "Hangul")
YASLI_ENUM_VALUE(KEY_JUNJA               , "Junja")
YASLI_ENUM_VALUE(KEY_FINAL               , "Final")
YASLI_ENUM_VALUE(KEY_HANJA               , "Hanja")
YASLI_ENUM_VALUE(KEY_KANJI               , "Kanji")
YASLI_ENUM_VALUE(KEY_ESCAPE              , "Escape")
YASLI_ENUM_VALUE(KEY_CONVERT             , "Convert")
YASLI_ENUM_VALUE(KEY_NONCONVERT          , "Nonconvert")
YASLI_ENUM_VALUE(KEY_ACCEPT              , "Accept")
YASLI_ENUM_VALUE(KEY_MODECHANGE          , "Modechange")
YASLI_ENUM_VALUE(KEY_SPACE               , "Space")
YASLI_ENUM_VALUE(KEY_PRIOR               , "Prior")
YASLI_ENUM_VALUE(KEY_NEXT                , "Next")
YASLI_ENUM_VALUE(KEY_END                 , "End")
YASLI_ENUM_VALUE(KEY_HOME                , "Home")
YASLI_ENUM_VALUE(KEY_LEFT                , "Left")
YASLI_ENUM_VALUE(KEY_UP                  , "Up")
YASLI_ENUM_VALUE(KEY_RIGHT               , "Right")
YASLI_ENUM_VALUE(KEY_DOWN                , "Down")
YASLI_ENUM_VALUE(KEY_SELECT              , "Select")
YASLI_ENUM_VALUE(KEY_PRINT               , "Print")
YASLI_ENUM_VALUE(KEY_EXECUTE             , "Execute")
YASLI_ENUM_VALUE(KEY_SNAPSHOT            , "Snapshot")
YASLI_ENUM_VALUE(KEY_INSERT              , "Insert")
YASLI_ENUM_VALUE(KEY_DELETE              , "Delete")
YASLI_ENUM_VALUE(KEY_HELP                , "Help")
YASLI_ENUM_VALUE(KEY_A                   , "A")
YASLI_ENUM_VALUE(KEY_B                   , "B")
YASLI_ENUM_VALUE(KEY_C                   , "C")
YASLI_ENUM_VALUE(KEY_D                   , "D")
YASLI_ENUM_VALUE(KEY_E                   , "E")
YASLI_ENUM_VALUE(KEY_F                   , "F")
YASLI_ENUM_VALUE(KEY_G                   , "G")
YASLI_ENUM_VALUE(KEY_H                   , "H")
YASLI_ENUM_VALUE(KEY_I                   , "I")
YASLI_ENUM_VALUE(KEY_J                   , "J")
YASLI_ENUM_VALUE(KEY_K                   , "K")
YASLI_ENUM_VALUE(KEY_L                   , "L")
YASLI_ENUM_VALUE(KEY_M                   , "M")
YASLI_ENUM_VALUE(KEY_N                   , "N")
YASLI_ENUM_VALUE(KEY_O                   , "O")
YASLI_ENUM_VALUE(KEY_P                   , "P")
YASLI_ENUM_VALUE(KEY_Q                   , "Q")
YASLI_ENUM_VALUE(KEY_R                   , "R")
YASLI_ENUM_VALUE(KEY_S                   , "S")
YASLI_ENUM_VALUE(KEY_T                   , "T")
YASLI_ENUM_VALUE(KEY_U                   , "U")
YASLI_ENUM_VALUE(KEY_V                   , "V")
YASLI_ENUM_VALUE(KEY_W                   , "W")
YASLI_ENUM_VALUE(KEY_X                   , "X")
YASLI_ENUM_VALUE(KEY_Y                   , "Y")
YASLI_ENUM_VALUE(KEY_Z                   , "Z")
YASLI_ENUM_VALUE(KEY_0                   , "0")
YASLI_ENUM_VALUE(KEY_1                   , "1")
YASLI_ENUM_VALUE(KEY_2                   , "2")
YASLI_ENUM_VALUE(KEY_3                   , "3")
YASLI_ENUM_VALUE(KEY_4                   , "4")
YASLI_ENUM_VALUE(KEY_5                   , "5")
YASLI_ENUM_VALUE(KEY_6                   , "6")
YASLI_ENUM_VALUE(KEY_7                   , "7")
YASLI_ENUM_VALUE(KEY_8                   , "8")
YASLI_ENUM_VALUE(KEY_9                   , "9")
YASLI_ENUM_VALUE(KEY_LWIN                , "Lwin")
YASLI_ENUM_VALUE(KEY_RWIN                , "Rwin")
YASLI_ENUM_VALUE(KEY_APPS                , "Apps")
YASLI_ENUM_VALUE(KEY_SLEEP               , "Sleep")
YASLI_ENUM_VALUE(KEY_NUMPAD0             , "Numpad0")
YASLI_ENUM_VALUE(KEY_NUMPAD1             , "Numpad1")
YASLI_ENUM_VALUE(KEY_NUMPAD2             , "Numpad2")
YASLI_ENUM_VALUE(KEY_NUMPAD3             , "Numpad3")
YASLI_ENUM_VALUE(KEY_NUMPAD4             , "Numpad4")
YASLI_ENUM_VALUE(KEY_NUMPAD5             , "Numpad5")
YASLI_ENUM_VALUE(KEY_NUMPAD6             , "Numpad6")
YASLI_ENUM_VALUE(KEY_NUMPAD7             , "Numpad7")
YASLI_ENUM_VALUE(KEY_NUMPAD8             , "Numpad8")
YASLI_ENUM_VALUE(KEY_NUMPAD9             , "Numpad9")
YASLI_ENUM_VALUE(KEY_MULTIPLY            , "Multiply")
YASLI_ENUM_VALUE(KEY_ADD                 , "Add")
YASLI_ENUM_VALUE(KEY_SEPARATOR           , "Separator")
YASLI_ENUM_VALUE(KEY_SUBTRACT            , "Subtract")
YASLI_ENUM_VALUE(KEY_DECIMAL             , "Decimal")
YASLI_ENUM_VALUE(KEY_DIVIDE              , "Divide")
YASLI_ENUM_VALUE(KEY_F1                  , "F1")
YASLI_ENUM_VALUE(KEY_F2                  , "F2")
YASLI_ENUM_VALUE(KEY_F3                  , "F3")
YASLI_ENUM_VALUE(KEY_F4                  , "F4")
YASLI_ENUM_VALUE(KEY_F5                  , "F5")
YASLI_ENUM_VALUE(KEY_F6                  , "F6")
YASLI_ENUM_VALUE(KEY_F7                  , "F7")
YASLI_ENUM_VALUE(KEY_F8                  , "F8")
YASLI_ENUM_VALUE(KEY_F9                  , "F9")
YASLI_ENUM_VALUE(KEY_F10                 , "F10")
YASLI_ENUM_VALUE(KEY_F11                 , "F11")
YASLI_ENUM_VALUE(KEY_F12                 , "F12")
YASLI_ENUM_VALUE(KEY_F13                 , "F13")
YASLI_ENUM_VALUE(KEY_F14                 , "F14")
YASLI_ENUM_VALUE(KEY_F15                 , "F15")
YASLI_ENUM_VALUE(KEY_F16                 , "F16")
YASLI_ENUM_VALUE(KEY_F17                 , "F17")
YASLI_ENUM_VALUE(KEY_F18                 , "F18")
YASLI_ENUM_VALUE(KEY_F19                 , "F19")
YASLI_ENUM_VALUE(KEY_F20                 , "F20")
YASLI_ENUM_VALUE(KEY_F21                 , "F21")
YASLI_ENUM_VALUE(KEY_F22                 , "F22")
YASLI_ENUM_VALUE(KEY_F23                 , "F23")
YASLI_ENUM_VALUE(KEY_F24                 , "F24")
YASLI_ENUM_VALUE(KEY_NUMLOCK             , "Numlock")
YASLI_ENUM_VALUE(KEY_SCROLL              , "Scroll")
YASLI_ENUM_VALUE(KEY_OEM_NEC_EQUAL       , "OEM NEC EQUAL")
YASLI_ENUM_VALUE(KEY_OEM_FJ_JISHO        , "OEM FJ JISHO")
YASLI_ENUM_VALUE(KEY_OEM_FJ_MASSHOU      , "OEM FJ MASSHOU")
YASLI_ENUM_VALUE(KEY_OEM_FJ_TOUROKU      , "OEM FJ TOUROKU")
YASLI_ENUM_VALUE(KEY_OEM_FJ_LOYA         , "OEM FJ LOYA")
YASLI_ENUM_VALUE(KEY_OEM_FJ_ROYA         , "OEM FJ ROYA")
YASLI_ENUM_VALUE(KEY_LSHIFT              , "LShift")
YASLI_ENUM_VALUE(KEY_RSHIFT              , "RShift")
YASLI_ENUM_VALUE(KEY_LCONTROL            , "LControl")
YASLI_ENUM_VALUE(KEY_RCONTROL            , "RControl")
YASLI_ENUM_VALUE(KEY_LMENU               , "LMenu")
YASLI_ENUM_VALUE(KEY_RMENU               , "RMenu")
YASLI_ENUM_VALUE(KEY_BROWSER_BACK        , "Browser Back")
YASLI_ENUM_VALUE(KEY_BROWSER_FORWARD     , "Browser Forward")
YASLI_ENUM_VALUE(KEY_BROWSER_REFRESH     , "Browser Refresh")
YASLI_ENUM_VALUE(KEY_BROWSER_STOP        , "Browser Stop")
YASLI_ENUM_VALUE(KEY_BROWSER_SEARCH      , "Browser Search")
YASLI_ENUM_VALUE(KEY_BROWSER_FAVORITES   , "Browser Favorites")
YASLI_ENUM_VALUE(KEY_BROWSER_HOME        , "Browser Home")
YASLI_ENUM_VALUE(KEY_VOLUME_MUTE         , "Volume Mute")
YASLI_ENUM_VALUE(KEY_VOLUME_DOWN         , "Volume Down")
YASLI_ENUM_VALUE(KEY_VOLUME_UP           , "Volume Up")
YASLI_ENUM_VALUE(KEY_MEDIA_NEXT_TRACK    , "Media Next Track")
YASLI_ENUM_VALUE(KEY_MEDIA_PREV_TRACK    , "Media Prev Track")
YASLI_ENUM_VALUE(KEY_MEDIA_STOP          , "Media Stop")
YASLI_ENUM_VALUE(KEY_MEDIA_PLAY_PAUSE    , "Media Play Pause")
YASLI_ENUM_VALUE(KEY_LAUNCH_MAIL         , "Launch Mail")
YASLI_ENUM_VALUE(KEY_LAUNCH_MEDIA_SELECT , "Launch Media Select")
YASLI_ENUM_VALUE(KEY_LAUNCH_APP1         , "Launch App1")
YASLI_ENUM_VALUE(KEY_LAUNCH_APP2         , "Launch App2")
YASLI_ENUM_VALUE(KEY_OEM_1               , "OEM_1")
YASLI_ENUM_VALUE(KEY_OEM_PLUS            , "OEM Plus")
YASLI_ENUM_VALUE(KEY_OEM_COMMA           , "OEM Comma")
YASLI_ENUM_VALUE(KEY_OEM_MINUS           , "OEM Minus")
YASLI_ENUM_VALUE(KEY_OEM_PERIOD          , "OEM Period")
YASLI_ENUM_VALUE(KEY_OEM_2               , "OEM 2")
YASLI_ENUM_VALUE(KEY_OEM_3               , "OEM 3")
YASLI_ENUM_VALUE(KEY_OEM_4               , "OEM 4")
YASLI_ENUM_VALUE(KEY_OEM_5               , "OEM 5")
YASLI_ENUM_VALUE(KEY_OEM_6               , "OEM 6")
YASLI_ENUM_VALUE(KEY_OEM_7               , "OEM 7")
YASLI_ENUM_VALUE(KEY_OEM_8               , "OEM 8")
YASLI_ENUM_VALUE(KEY_OEM_AX              , "OEM AX")
YASLI_ENUM_VALUE(KEY_OEM_102             , "OEM 102")
YASLI_ENUM_VALUE(KEY_ICO_HELP            , "Ico_help")
YASLI_ENUM_VALUE(KEY_ICO_00              , "Ico_00")
YASLI_ENUM_VALUE(KEY_PROCESSKEY          , "Processkey")
YASLI_ENUM_VALUE(KEY_ICO_CLEAR           , "Ico_clear")
YASLI_ENUM_VALUE(KEY_PACKET              , "Packet")
YASLI_ENUM_VALUE(KEY_OEM_RESET           , "OEM Reset")
YASLI_ENUM_VALUE(KEY_OEM_JUMP            , "OEM Jump")
YASLI_ENUM_VALUE(KEY_OEM_PA1             , "OEM Pa1")
YASLI_ENUM_VALUE(KEY_OEM_PA2             , "OEM Pa2")
YASLI_ENUM_VALUE(KEY_OEM_PA3             , "OEM Pa3")
YASLI_ENUM_VALUE(KEY_OEM_WSCTRL          , "OEM Wsctrl")
YASLI_ENUM_VALUE(KEY_OEM_CUSEL           , "OEM Cusel")
YASLI_ENUM_VALUE(KEY_OEM_ATTN            , "OEM Attn")
YASLI_ENUM_VALUE(KEY_OEM_FINISH          , "OEM Finish")
YASLI_ENUM_VALUE(KEY_OEM_COPY            , "OEM Copy")
YASLI_ENUM_VALUE(KEY_OEM_AUTO            , "OEM Auto")
YASLI_ENUM_VALUE(KEY_OEM_ENLW            , "OEM Enlw")
YASLI_ENUM_VALUE(KEY_OEM_BACKTAB         , "OEM Backtab")
YASLI_ENUM_VALUE(KEY_ATTN                , "Attn")
YASLI_ENUM_VALUE(KEY_CRSEL               , "Crsel")
YASLI_ENUM_VALUE(KEY_EXSEL               , "Exsel")
YASLI_ENUM_VALUE(KEY_EREOF               , "Ereof")
YASLI_ENUM_VALUE(KEY_PLAY                , "Play")
YASLI_ENUM_VALUE(KEY_ZOOM                , "Zoom")
YASLI_ENUM_VALUE(KEY_NONAME              , "NoName")
YASLI_ENUM_VALUE(KEY_PA1                 , "Pa1")
YASLI_ENUM_VALUE(KEY_OEM_CLEAR           , "OEM_clear")
YASLI_ENUM_END()

YASLI_ENUM_BEGIN(KeyModifiers, "KeyModifiers")
YASLI_ENUM_VALUE(KEY_MOD_CONTROL, "Ctrl")
YASLI_ENUM_VALUE(KEY_MOD_SHIFT, "Shift")
YASLI_ENUM_VALUE(KEY_MOD_ALT, "Alt")
YASLI_ENUM_END()

// TODO: remove?
const char* KeyPress::keyNames_[]  = {
	"", "LMB", "RMB", "Cancel", "MMB", "", "", "", "Backspace", "Tab", "", "", "Clear", "Enter", "", "", //0x00 - 0x0F
		"Shift", "Control", "Alt", "Pause", "Caps", "", "", "", "", "", "", "Escape", "", "", "", "", //0x10 - 0x1F
		"Space", "PgUP", "PgDN", "End", "Home", "Left", "Up", "Right", "Down", "", "", "", "PrSrc", "Ins", "Del", "", //0x20 - 0x2F
		"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "", "", "", "", "", //0x30 - 0x3F
		"", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", //0x40 - 0x4F
		"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "LWin", "RWin", "Apps", "", "Sleep", //0x50 - 0x5F
		"num0", "num1", "num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9", "num*", "num+", "", "num-", "num.", "num/", //0x60 - 0x6F
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", //0x70 - 0x7F
		"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "LDBL", "RDBL", "", "", "", "", "", "", //0x80 - 0x8F
		"NumLock", "ScrollLock", "LShift", "RShift", "LCtrl", "RCtrl", "LAlt", "RAlt", "", "", "", "", "", "", "", "", //0x90 - 0x9F
		"", "", "", "", "", "", "", "WheelUp", "WheelDn", "", "", "", "", "", "", "", //0xA0 - 0xAF
		"", "", "", "", "", "", "", "", "", "", ";", "+", ",", "-", ".", "/", //0xB0 - 0xBF
		"~", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", //0xC0 - 0xCF
		"", "", "", "", "", "", "", "", "", "", "", "[", "\\", "]", "\"", "", //0xD0 - 0xDF
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", //0xE0 - 0xEF
		"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "WakeUp" //0xF0 - 0xFF
};

KeyPress::KeyPress(int _fullCode)
{
	fullCode = _fullCode;
	if(key == KEY_CONTROL) 
		control = 1;
	if(key == KEY_SHIFT)
		shift = 1;
	if(key == KEY_MENU)
		alt = 1;
}

KeyPress KeyPress::addModifiers(Key key)
{
	KeyPress kp(key);
	kp.control |= (GetKeyState(VK_CONTROL) >> 15) ? 1 : 0; 
	kp.shift |= (GetKeyState(VK_SHIFT) >> 15) ? 1 : 0; 
	kp.alt |= (GetKeyState(VK_MENU) >> 15) ? 1 : 0; 
	return kp;
}

string KeyPress::toString(bool compact) const
{
	string keyName;
	const char* plus = compact ? "+" : " + ";
	
	if(control && key != VK_CONTROL)
		keyName += KeyPress::nameCtrl();
	
	if(shift && key != VK_SHIFT){
		if(!keyName.empty())
			keyName += plus;
		keyName += KeyPress::nameShift();
	}
	
	if(alt && key != VK_MENU){
		if(!keyName.empty())
			keyName += plus;
		keyName += KeyPress::nameAlt();
	}

	if(key){
		if(!keyName.empty())
			keyName += plus;

		const char* locName = KeyPress::name(key);
		if(locName && *locName)
			keyName += locName;
		else{
			keyName += "0x";
			keyName += toHex(key);
		}
	}
	return keyName;
}

void KeyPress::serialize(Archive& ar)
{
	Key k = (Key)key;
	ar( k, "", "Key" );
	key = k;
	BitVector<KeyModifiers> mod;
	if(control)
		mod |= KEY_MOD_CONTROL;
	if(shift)
		mod |= KEY_MOD_SHIFT;
	if(alt)
		mod |= KEY_MOD_ALT;
	ar( mod, "", "Modifiers" );
	*this = KeyPress(Key(key), KeyModifiers(int(mod)));
}

}

/*
bool serialize(yasli::Archive& ar, ww::KeyPress &value, const char* name, const char* label)
{
	if(ar.isEdit())
		return ar( yasli::Serializer(value), name, label );
	else
      return ar( value.fullCode, name, label );
}
*/
