#include "ww/Application.h"
#include "ww/Window.h"

#include "ww/VBox.h"
#include "ww/HBox.h"
#include "ww/VSplitter.h"
#include "ww/HSplitter.h"
#include "ww/ScrolledWindow.h"
#include "ww/Tabs.h"

#include "ww/PropertyTree.h"

#include "ww/Entry.h"
#include "ww/ComboBox.h"
#include "ww/Label.h"
#include "ww/Button.h"
#include "ww/ProgressBar.h"
#include "ww/RadioButton.h"
#include "ww/RadioButtonBox.h"
#include "ww/CheckBox.h"
#include "ww/Frame.h"
#include "ww/Dialog.h"
#include "ww/Serialization.h"
#include "ww/HotkeyDialog.h"
#include "ww/Clipboard.h"
#include "ww/PropertyEditor.h"
#include "ww/PopupMenu.h"
#include "ww/Win32Proxy.h"
#include "ww/Win32/Window32.h"
#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"
#include "yasli/BinArchive.h"
#include "yasli/JSONOArchive.h"

#include "TestData.h"

#include <string>
#include <windows.h>
using std::string;

class IntButton : public ww::Button{
public:
	IntButton(const char* text, int value)
	: ww::Button(text, 0)
	, value_(value)
	{

	}

	void onPressed()
	{
		signalPressed_.emit(value_);
	}
	sigslot::signal1<int>& signalPressed() { return signalPressed_; }
protected:
	sigslot::signal1<int> signalPressed_;

	int value_;
};

class MainWindow : public ww::Window{
public:
	MainWindow(ww::Application& app);
	~MainWindow();

	void attach();
	void onTextChanged();
	void onIntButtonPressed(int index);
	void onChangeTextButton();
	void onChangeCheckBox();
	void onAddScrolledButton();
	void pressShowModal();

	void onPropertyTreeSaveToText();
	void onPropertyTreeLoadFromText();
	void onPropertyTreeToggleCompact();
	void onPropertyTreeToggleFullRowMode();

	ww::Widget* createTabControls();
	ww::Widget* createTabPropertyTree();
	ww::Widget* createTabTwoPropertyTrees();

protected:
	ww::Win32Proxy* proxy_;
	ww::VBox* vbox_;
	ww::VBox* scrolledVBox_;
	ww::Entry* edit_;
	ww::Button* buttonChangeText_;
	ww::ProgressBar* progressBar_;
	ww::PropertyTree* propertyTree_;
	ww::CheckBox* propertyTreeCompact_;
	ww::CheckBox* propertyTreeFullRowMode_;
	ww::CheckBox* propertyTreeMulti_;
	ww::PropertyTree* parentTree_;
	ww::PropertyTree* childTree_;
	ww::Application* app_;
};

TestData testData;
TestData testData1;

MainWindow::MainWindow(ww::Application& app)
: app_(&app)
{
	setTitle("wWidgets: Lightweight UI Library");
	setBorder(0);
	setMinimizeable(true);
	setResizeable(true);

	setDefaultSize(800, 600);
	setDefaultPosition(ww::POSITION_CENTER);

	ww::PopupMenu* menu = new ww::PopupMenu();
	ww::PopupMenuItem& file = menu->root().add("&File");
	file.add("Test");
	file.addSeparator();
	file.add("E&xit").connect((Win32::MessageLoop*)&app, &Win32::MessageLoop::quit);

	setMenu(menu);

	signalClose().connect((Win32::MessageLoop*)&app, &Win32::MessageLoop::quit);

    proxy_ = 0;

	ww::TabPages* pages = new ww::TabPages();
	pages->add("Layout && Controls", createTabControls());
	pages->add("PropertyTree", createTabPropertyTree());
	pages->add("Two trees", createTabTwoPropertyTrees());
    add(pages);

	attach();

	TextIArchive sa;
	sa.setFilter(ww::SERIALIZE_STATE);
	if(sa.load("states"))
		sa(*this, "window", "Window");
}

MainWindow::~MainWindow()
{
	TextOArchive oa;
	oa.setFilter(ww::SERIALIZE_STATE);
	oa(*this, "window", "Window");
	oa.save("states");
}

ww::Widget* MainWindow::createTabControls()
{
	ww::VBox* vbox = new ww::VBox;

	/*
	ww::CommandManager* commands = new ww::CommandManager();
	ww::Toolbar* toolbar = new ww::Toolbar(commands);
	*/

	/*
	ww::ImageStore* imageStore = new ww::ImageStore(24, 24);
	imageStore->addFromFile("res\\FX_PAN_256.bmp", RGB(255, 0, 255));
	toolbar->setImageStore(imageStore);
	*/

	ww::HSplitter* splitter_ = new ww::HSplitter(0, 0);
	vbox->add(splitter_, ww::PACK_FILL);
	{
		ww::VSplitter* vsplitter = new ww::VSplitter(0, 0);
		splitter_->add(vsplitter, 0.25f);
		{
			ww::VBox* vbox = new ww::VBox(4, 0);
			{
				vbox->add(new ww::Label("Группа 1:", true));

				ww::ComboBox* comboBox = new ww::ComboBox;
				comboBox->add("Вариант 1");
				comboBox->add("Вариант 2");
				comboBox->add("Вариант 3");
				vbox->add(comboBox);

				ww::Button* button = new ww::Button("В&ыход");
				button->signalPressed().connect((Win32::MessageLoop*)app_, &Win32::MessageLoop::quit);
				vbox->add(button, ww::PACK_COMPACT);

				ww::CheckBox* checkBox = new ww::CheckBox("Text Check Box");
				checkBox->signalChanged().connect(this, &MainWindow::onChangeCheckBox);
				vbox->add(checkBox, ww::PACK_COMPACT);
				
				ww::RadioButtonBox* radioButtonBox = new ww::RadioButtonBox("RadioBtnBox");
				radioButtonBox->addRadioButton("Radio 1");
				radioButtonBox->addRadioButton("Radio 2");
				radioButtonBox->addRadioButton("Radio 3");
				radioButtonBox->addRadioButton("Radio 4");
				vbox->add(radioButtonBox, ww::PACK_COMPACT);

				progressBar_ = new ww::ProgressBar();
				progressBar_->setPosition(0.5f);
				vbox->add(progressBar_, ww::PACK_COMPACT);

				ww::RadioButton* rBtn1 = new ww::RadioButton(0, "RadioButton 1");
				vbox->add(rBtn1, ww::PACK_COMPACT);

				buttonChangeText_ = new ww::Button("Добавить кнопок");
				buttonChangeText_->signalPressed().connect(this, &MainWindow::onChangeTextButton);
				vbox->add(buttonChangeText_, ww::PACK_END);

				button = new ww::Button("Редактировать");
				//button->signalPressed().connect(this, &MainWindow::onEditButton);
				vbox->add(button, ww::PACK_BEGIN);

				ww::Label* label = new ww::Label("Текстовое &поле:", true);
				vbox->add(label, ww::PACK_COMPACT);

				ww::RadioButton* rBtn2 = new ww::RadioButton(rBtn1, "RadioButton 2");
				vbox->add(rBtn2, ww::PACK_COMPACT);
				ww::RadioButton* rBtn3 = new ww::RadioButton(rBtn2, "RadioButton 3");
				vbox->add(rBtn3, ww::PACK_COMPACT);

				edit_ = new ww::Entry("Text", true);
				edit_->signalChanged().connect(this, &MainWindow::onTextChanged);

				ww::HBox* hbox = new ww::HBox(4, 0);
				{
					button = new ww::Button("Кнопка 3");
					button->setSensitive(false);
					button = new ww::Button("Кнопочка 4");
				}

			}
			ww::Frame* frame_ = new ww::Frame();
			frame_->add(vbox);
			vsplitter->add(frame_, 0.5f);

			ww::ScrolledWindow* scrolledWindow = new ww::ScrolledWindow(6);
			vsplitter->add(scrolledWindow);
			{
				vbox_ = new ww::VBox(4, 0);
				scrolledWindow->add(vbox_);
				scrolledWindow->setPolicy(ww::SCROLL_NEVER, ww::SCROLL_AUTOMATIC);
				{
					ww::Label* label = new ww::Label("Здесь будет много кнопочек");
					label->setAlignment(ww::ALIGN_CENTER, ww::ALIGN_MIDDLE);
					vbox_->add(label, ww::PACK_FILL);
				}
			}
		}

		ww::Box* box = new ww::VBox;
		box->add(new ww::Button("First button", 4), ww::PACK_COMPACT);

		ww::ScrolledWindow* scrolledWindow = new ww::ScrolledWindow();
		box->add(scrolledWindow, ww::PACK_FILL );
		{
			scrolledVBox_ = new ww::VBox();
			scrolledWindow->add(scrolledVBox_);
		}

		ww::Button *button = new ww::Button("Add Scrolled Button", 10);
		button->signalPressed().connect(this, &MainWindow::onAddScrolledButton);
		box->add(button, ww::PACK_COMPACT);
		splitter_->add(box);
	}

	vbox->add(new ww::Label("Статусная строка", false, 2), ww::PACK_COMPACT);
	return vbox;
}

struct ConditionEditorData
{
	void serialize(Archive& ar)
	{

	}

};

ww::Widget* MainWindow::createTabPropertyTree()
{
	ww::HBox *hbox = new ww::HBox();

	propertyTree_ = new ww::PropertyTree;
	propertyTree_->setExpandLevels(2);
	hbox->add(propertyTree_, ww::PACK_FILL);

	ww::VBox *vbox = new ww::VBox(4, 4);
	hbox->add(vbox, ww::PACK_COMPACT);
	{
		propertyTreeCompact_ = new ww::CheckBox("Compact");
		propertyTreeCompact_->signalChanged().connect(this, &MainWindow::onPropertyTreeToggleCompact);
		vbox->add(propertyTreeCompact_);

		propertyTreeFullRowMode_ = new ww::CheckBox("fullRowMode");
		propertyTreeFullRowMode_->signalChanged().connect(this, &MainWindow::onPropertyTreeToggleFullRowMode);
		vbox->add(propertyTreeFullRowMode_);

		propertyTreeMulti_ = new ww::CheckBox("MultiMode");
		propertyTreeMulti_->signalChanged().connect(this, &MainWindow::attach);
		vbox->add(propertyTreeMulti_);

		ww::Button* button;
		button = new ww::Button("&Save to Text");
		button->signalPressed().connect(this, &MainWindow::onPropertyTreeSaveToText);
		vbox->add(button);

		button = new ww::Button("&Load from Text");
		button->signalPressed().connect(this, &MainWindow::onPropertyTreeLoadFromText);
		vbox->add(button);
	}

	return hbox;
}

ww::Widget* MainWindow::createTabTwoPropertyTrees()
{
	ww::HSplitter* hSplitter = new ww::HSplitter();

	parentTree_ = new ww::PropertyTree;
	hSplitter->add(parentTree_);
	parentTree_->attach(Serializer(testData));

	childTree_ = new ww::PropertyTree;
	hSplitter->add(childTree_);
	parentTree_->attachPropertyTree(childTree_);


	return hSplitter;
}

void MainWindow::attach()
{
	propertyTree_->detach();
	if(!propertyTreeMulti_->status())
		propertyTree_->attach(Serializer(testData));
	else
	{
		//Serializers serializers;
		//serializers.push_back(Serializer(testData));
		//serializers.push_back(Serializer(testData1));
		propertyTree_->attach(Serializer(testData));
	}
}

void MainWindow::onTextChanged()
{
	buttonChangeText_->setText(edit_->text());
}

void MainWindow::onPropertyTreeToggleCompact()
{
	propertyTree_->setCompact( propertyTreeCompact_->status() );
}

void MainWindow::onPropertyTreeToggleFullRowMode()
{
	propertyTree_->setFullRowMode( propertyTreeFullRowMode_->status() );
}

void MainWindow::onPropertyTreeSaveToText()
{
	TextOArchive oa;
	oa(testData, "testData", "testData");
	oa.save("Test.ta");

	JSONOArchive joa;
	joa(testData, "testData", "testData");
	joa.save("Test.json");
}

void MainWindow::onPropertyTreeLoadFromText()
{
	TextIArchive oa;
    if(oa.load("Test.ta"))
        oa(testData, "testData", "testData");
	propertyTree_->revert();
}
	
void MainWindow::pressShowModal()
{
	//ww::HotkeyDialog dlg(this);
	//dlg.showModal();

	TestBase testBase;
	testBase.base_member = "Pasted from clipboard";
	testBase.base_float = 98765.f;
	//ww::Clipboard clip(this);
	//clip.copy(Serializer(testBase));
}

void MainWindow::onIntButtonPressed(int index)
{
	buttonChangeText_->setText("Button pressed");
}

void MainWindow::onChangeCheckBox()
{
	progressBar_->setPosition(rand()*1.f/RAND_MAX);
}

void MainWindow::onChangeTextButton()
{
	static int counter = 0;
	const char* buf = "Кнопка ся";

	IntButton* newButton = new IntButton(buf, counter++);
	newButton->signalPressed().connect(this, &MainWindow::onIntButtonPressed);
	vbox_->add(newButton, ww::PACK_FILL);

	newButton = new IntButton(buf, counter++);
	newButton->signalPressed().connect(this, &MainWindow::onIntButtonPressed);
	vbox_->add(newButton, ww::PACK_FILL);

	newButton = new IntButton(buf, counter++);
	newButton->signalPressed().connect(this, &MainWindow::onIntButtonPressed);
	vbox_->add(newButton, ww::PACK_FILL);

	vbox_->showAll();
}

void MainWindow::onAddScrolledButton()
{
	scrolledVBox_->add(new ww::Button("Scrolled Button"));
	scrolledVBox_->showAll();
}


class TestBase1 : public RefCounter
{
public:
    virtual void serialize(Archive& ar)
    {
    }
};

class TestClass
{
public:
    void serialize(Archive& ar)
    {
        ar(name, "name", 0);
        ar(values, "values", 0);
        ar(objects, "objects", 0);
    }

private:
    string name;
    std::vector<int> values;
    std:: vector<SharedPtr<TestBase1> > objects;
};



class TestA1 : public TestBase1
{
	virtual void serialize(Archive& ar)
	{
		ar(nameA_, "nameA", 0);
	}

	string nameA_;
};

YASLI_CLASS(TestBase1, TestA1, "TestA")

class TestB1 : public TestBase1
{
    virtual void serialize(Archive& ar)
    {
        ar(nameB_, "nameB", 0);
    }

    string nameB_;
};
YASLI_CLASS(TestBase1, TestB1, "TestB")


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ww::Application app(hInstance);

	SharedPtr<MainWindow> window = new MainWindow(app);
	window->showAll();

	app.run();
	return 0;
}
