project(QPropertyTree)
set(CMAKE_AUTOMOC TRUE)

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Wno-deprecated-declarations -Werror")
endif (UNIX)

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996")
endif(MSVC)

if (NOT QT4_FOUND AND NOT Qt5_INCLUDE_DIRS)
	find_package(Qt5Widgets)
	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	add_definitions(${Qt5Widgets_DEFINITIONS})
endif()

set(SOURCES
	../PropertyTree/Color.cpp
	../PropertyTree/Color.h
	../PropertyTree/ConstStringList.cpp
	../PropertyTree/ConstStringList.h
	../PropertyTree/Factory.h
	../PropertyTree/MathUtils.h
	../PropertyTree/PropertyIArchive.cpp
	../PropertyTree/PropertyIArchive.h
	../PropertyTree/PropertyOArchive.cpp
	../PropertyTree/PropertyOArchive.h
	../PropertyTree/PropertyRow.cpp
	../PropertyTree/PropertyRow.h
	../PropertyTree/PropertyRowBool.cpp
	../PropertyTree/PropertyRowBool.h
	../PropertyTree/PropertyRowButton.cpp
	../PropertyTree/PropertyRowColor.cpp
	../PropertyTree/PropertyRowColor.h
	../PropertyTree/PropertyRowContainer.cpp
	../PropertyTree/PropertyRowContainer.h
	../PropertyTree/PropertyRowField.cpp
	../PropertyTree/PropertyRowField.h
	../PropertyTree/PropertyRowHorizontalLine.cpp
	../PropertyTree/PropertyRowIconXPM.cpp
	../PropertyTree/PropertyRowImpl.h
	../PropertyTree/PropertyRowNumber.cpp
	../PropertyTree/PropertyRowNumber.h
	../PropertyTree/PropertyRowNumberField.cpp
	../PropertyTree/PropertyRowNumberField.h
	../PropertyTree/PropertyRowObject.cpp
	../PropertyTree/PropertyRowObject.h
	../PropertyTree/PropertyRowPointer.cpp
	../PropertyTree/PropertyRowPointer.h
	../PropertyTree/PropertyRowString.cpp
	../PropertyTree/PropertyRowString.h
	../PropertyTree/PropertyRowStringListValue.cpp
	../PropertyTree/PropertyRowStringListValue.h
	../PropertyTree/PropertyTree.cpp
	../PropertyTree/PropertyTree.h
	../PropertyTree/PropertyTreeMenuHandler.h
	../PropertyTree/PropertyTreeModel.cpp
	../PropertyTree/PropertyTreeModel.h
	../PropertyTree/PropertyTreeOperator.cpp
	../PropertyTree/PropertyTreeOperator.h
	../PropertyTree/Serialization.h
	../PropertyTree/Unicode.cpp
	../PropertyTree/Unicode.h
	IconXPMCache.cpp
	IconXPMCache.h
	InplaceWidgetComboBox.h
	InplaceWidgetNumber.h
	InplaceWidgetString.h
	PropertyRowFileOpen.cpp
	PropertyRowFileOpen.h
	PropertyRowFileSave.cpp
	QDrawContext.cpp
	QDrawContext.h
	QPropertyTree.cpp
	QPropertyTree.h
	QUIFacade.cpp
	QUIFacade.h
	)
include_directories(..)
add_library(qpropertytree ${SOURCES})
source_group("" FILES ${SOURCES})
target_link_libraries(qpropertytree yasli ${Qt5Widgets_LIBRARIES})
set_target_properties(qpropertytree PROPERTIES DEBUG_POSTFIX "-debug")
