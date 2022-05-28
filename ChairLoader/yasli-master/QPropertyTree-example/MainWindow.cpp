#include "MainWindow.h"
#include <QBoxLayout>
#include "QPropertyTree/QPropertyTree.h"

#include <limits.h>
#include <float.h>

#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/decorators/Range.h"
using yasli::Archive;
using yasli::Range;
#include "yasli/STLImpl.h"

QWidget* createExampleCustomRows();
QWidget* createExampleControlCharacters();
QWidget* createExamplePolymorphicTree();
QWidget* createExampleTable();
QWidget* createExampleTwoTrees();

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
	resize(800, 600);

	QTabWidget* tabs = new QTabWidget();

	QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);

	tabs->addTab(createExampleCustomRows(), "Decorators and Custom Rows");
	tabs->addTab(createExamplePolymorphicTree(), "Polymorphic Tree");
	tabs->addTab(createExampleControlCharacters(), "Control Characters");
	tabs->addTab(createExampleTable(), "Table");
	tabs->addTab(createExampleTwoTrees(), "Two Trees");

	setCentralWidget(tabs);
}

MainWindow::~MainWindow()
{
}
