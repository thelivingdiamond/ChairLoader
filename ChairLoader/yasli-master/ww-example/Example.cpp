#include "ww/Win32/Types.h"
#include "ww/Application.h"
#include "ww/Window.h"

#include "ww/VBox.h"
#include "ww/HBox.h"
#include "ww/VSplitter.h"
#include "ww/HSplitter.h"
#include "ww/ScrolledWindow.h"
#include "ww/Tabs.h"

#include "ww/PropertyTree.h"

#include "ww/Frame.h"
#include "ww/Serialization.h"
#include "ww/Win32/Window32.h"
#include "yasli/TextOArchive.h"
#include "yasli/TextIArchive.h"

#include "TestData.h"

#include <string>
#include <windows.h>

class MainWindow : public ww::Window{
public:
	MainWindow(ww::Application& app);
	~MainWindow();

protected:
	ww::Application* app_;
};

TestData testData;
TestData testData1;

MainWindow::MainWindow(ww::Application& app)
: app_(&app)
{
	setTitle("wWidgets: PropertyTree Example");
	setBorder(0);
	setMinimizeable(true);
	setResizeable(true);

	setDefaultSize(800, 600);
	setDefaultPosition(ww::POSITION_CENTER);

	signalClose().connect(&app, &ww::Application::quit);

	ww::TabPages* pages = new ww::TabPages();

	ww::Widget* createPolymorphicTree();
	pages->add("Polymorphic Tree", createPolymorphicTree());

	ww::Widget* createCustomRows();
	pages->add("Custom Rows & Decorators", createCustomRows());

	ww::Widget* createDigestSample();
	pages->add("Digests", createDigestSample());

	ww::Widget* createTableSample();
	pages->add("Table", createTableSample());

	ww::Widget* createTwoTrees();
	pages->add("Two Trees", createTwoTrees());

	add(pages);

	TextIArchive sa;
	sa.setFilter(ww::SERIALIZE_STATE);
	if(sa.load("example.state"))
		sa(*this, "window", "Window");
}

MainWindow::~MainWindow()
{
	TextOArchive oa;
	oa.setFilter(ww::SERIALIZE_STATE);
	oa(*this, "window", "Window");
	oa.save("example.state");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ww::Application app(hInstance);

	SharedPtr<MainWindow> window = new MainWindow(app);
	window->showAll();

	app.run();
	return 0;
}
