#
#  yasli - Serialization Library.
#  Copyright (C) 2007-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
#                          Alexander Kotliar <alexander.kotliar@gmail.com>
# 
#  This code is distributed under the MIT License:
#                          http://www.opensource.org/licenses/MIT
#

project("yasli")

set(SOURCES
	Archive.cpp Archive.h
	Assert.cpp Assert.h
	BinArchive.cpp BinArchive.h
	BitVector.h BitVectorImpl.h
	ClassFactory.cpp ClassFactory.h ClassFactoryBase.h
	Config.h ConfigLocal.h
	Enum.cpp Enum.h
	Helpers.h
	JSONIArchive.cpp JSONIArchive.h
	JSONOArchive.cpp JSONOArchive.h
	KeyValue.h
	MemoryReader.cpp MemoryReader.h
	MemoryWriter.cpp MemoryWriter.h
	Object.h
	Pointers.h PointersImpl.h
	Serializer.h SerializerImpl.h
	StdAfx.cpp StdAfx.h
	STL.h STLImpl.h
	StringList.cpp StringList.h
	TextIArchive.cpp TextIArchive.h
	TextOArchive.cpp TextOArchive.h
	Token.h
	TypeID.h
	)

set(DECORATORS
	decorators/IconXPM.h
	decorators/FileOpen.h 
	decorators/FileOpen.cpp
	decorators/FileSave.h 
	decorators/FileSave.cpp
	decorators/BitFlags.h
	decorators/BitFlags.cpp
	)

source_group("" FILES ${SOURCES})
source_group("decorators" FILES ${DECORATORS})

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

include_directories(..)
add_library("yasli" ${SOURCES} ${DECORATORS})

set_target_properties("yasli" PROPERTIES DEBUG_POSTFIX "-debug")

#include("Testo")
#TESTO("yasli" "Test.cpp")
