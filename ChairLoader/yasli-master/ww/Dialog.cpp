/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Dialog.h"
#include "ww/Win32/Window32.h"

#include "ww/Serialization.h"

namespace ww
{

class ButtonResponse : public Button
{
public:
	ButtonResponse(const char* text, int response, int border = 0)
	: ww::Button(text, border)
	, response_(response)
	{
		HWND wnd = _window()->handle();
		SetWindowLongPtr(wnd, GWLP_ID, (LONG_PTR)response);
	}

	void onPressed(){
		signalPressed_.emit(response_);
	}
	signal1<int>& signalPressed() { return signalPressed_; }

protected:
	signal1<int> signalPressed_;
	int response_;

};

typedef std::vector<Widget*> SpawnedDialogs;
static SpawnedDialogs spawnedDialogs;

static ww::Widget* lastDialog()
{
	if(spawnedDialogs.empty())
		return 0;
	ww::Widget* last = spawnedDialogs.back();
	YASLI_ASSERT(last);
	Win32::Window32* window = _findWindow(last);
	if(window && !IsWindowEnabled(window->handle()))
		return 0;
	return last;
}

static HWND findOwner(HWND wnd){
	if(!wnd){
		Widget* last = lastDialog();
		if(last) {
			Win32::Window32* window = _findWindow(last);
			return window ? window->handle() : 0;
		}
		else
			return 0;
	}
	while(HWND parent = GetParent(wnd))
		wnd = parent;
	return wnd;
}

static HWND findOwner(Widget* widget){
	if(!widget){
		Widget* last = lastDialog();
		if(last) {
			Win32::Window32* window = _findWindow(last);
			return window ? window->handle() : 0;
		}
		else
			return 0;
	}
	HWND parentWnd = 0;
	Win32::Window32* window = _findWindow(widget);
	YASLI_ASSERT(window);
	if(window)
		return findOwner(window->handle());
	else
		return 0;
}

Dialog::Dialog(HWND parent, int border)
: Window(parentWnd_ = findOwner(parent), border)
{
	init(border);
}

Dialog::Dialog(ww::Widget* widget, int border)
: Window(parentWnd_ = findOwner(widget), border)
{
	init(border);
}

void Dialog::onKeyDefault()
{
	onResponse(defaultResponse_);
}

void Dialog::onKeyCancel()
{
	onResponse(cancelResponse_);
}


void Dialog::init(int border)
{
	activeDialogLoop_ = 0;

	setDefaultPosition(POSITION_CENTER);
	setResizeable(false);
	setMinimizeable(false);
	setBorder(10);
	
	vbox_ = new ww::VBox(3, 0);
	Window::add(vbox_);
	{
		hboxButtons_ = new ww::HBox(4, 0);
		vbox_->add(new ww::HLine());
		vbox_->add(hboxButtons_);
	}

	response_ = defaultResponse_ = cancelResponse_ = RESPONSE_CANCEL;

}

void Dialog::add(Widget* widget, PackMode packMode)
{
	vbox_->insert(widget, int(vbox_->size() - 2), packMode, 0);
}

void Dialog::addButton(const char* label, int response, bool atRight)
{
	ButtonResponse* button = new ButtonResponse(label, response, 0);
	button->setRequestSize(68, 24);
	button->signalPressed().connect(this, &Dialog::onResponse);
	hboxButtons_->add(button, atRight ? PACK_END : PACK_BEGIN);
	if(response == RESPONSE_OK || response == RESPONSE_YES){
		defaultResponse_ = response;
		setDefaultWidget(button);
	}
	if(response == RESPONSE_CANCEL || response == RESPONSE_NO)
		cancelResponse_ = response;
}


void Dialog::onResponse(int response)
{
	response_ = response;
	
	if (activeDialogLoop_)
		activeDialogLoop_->interruptDialogLoop();
}

void Dialog::onClose()
{
	onResponse(cancelResponse_);
}

void Dialog::setCancelResponse(int response)
{
	cancelResponse_ = response; 
}

void Dialog::setDefaultResponse(int response)
{
	defaultResponse_ = response; 
}

int Dialog::showModal()
{
	spawnedDialogs.push_back(this);

	if(parentWnd_){
		YASLI_ESCAPE(::IsWindow(parentWnd_), return 0);
		if(::IsWindowEnabled(parentWnd_)){
			::SetWindowPos(_window()->handle(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            showAll();
			::EnableWindow(parentWnd_, FALSE);
		}
	}
    else
    {
        showAll();
    }
	vbox_->setFocus();
	
	Win32::MessageLoop loop;
	signalClose().connect(this, &Dialog::onClose);

	activeDialogLoop_ = &loop;
	loop.runDialogLoop(_window()->handle());
	activeDialogLoop_ = 0;

	if (parentWnd_) {
		::EnableWindow(parentWnd_, TRUE);
		::SetActiveWindow(parentWnd_);
	}
	hide();

	YASLI_ASSERT(!spawnedDialogs.empty() && spawnedDialogs.back() == this);
	spawnedDialogs.pop_back();
	return response_;
}

void Dialog::_onWMCommand(int command)
{
	if (command == IDOK) {
		onResponse(defaultResponse_);
	}
	else if (command == IDCANCEL || command == 0){
		onResponse(cancelResponse_);
	}
	else{
		YASLI_ASSERT(0);
	}
}


Dialog::~Dialog()
{
}

void Dialog::serialize(Archive& ar)
{
	Window::serialize(ar);
}

}
