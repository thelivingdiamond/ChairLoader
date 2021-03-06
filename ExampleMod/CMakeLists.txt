cmake_minimum_required(VERSION 3.20)
project(ExampleMod) # CHANGE ME

if((NOT MSVC) OR (MSVC_TOOLSET_VERSION LESS 141))
	message(FATAL_ERROR "Mods must be built using Visual Studio 2017 or newer")
endif()

# Set C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prey was build in Release so mods must match it even in Debug
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
add_compile_definitions(
	CHAIRLOADER_MOD_SDK
	$<IF:$<CONFIG:Debug>,DEBUG_BUILD,RELEASE_BUILD>
)

# Common path
set(CHAIRLOADER_COMMON_PATH "" CACHE PATH "Path to Chairloader/Common. See Common/README_ModSDK.txt.")

# Macros and functions

# Creates a configuration variable with mod's DLL path. If set, mod's DLL will be put there.
function(mod_output_dir MOD_TARGET MOD_PREFIX)
	set("${MOD_PREFIX}_DLL_PATH" "" CACHE PATH "Where to put the DLL")
	set(mod_dll_path ${${MOD_PREFIX}_DLL_PATH})
	if (mod_dll_path)
		set_target_properties(${MOD_TARGET} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY ${mod_dll_path}
			RUNTIME_OUTPUT_DIRECTORY_DEBUG ${mod_dll_path}
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${mod_dll_path}
			RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${mod_dll_path}
			RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${mod_dll_path}
		)
	endif()
endfunction()

# Third-party libraries
# add_subdirectory(ThirdParty/ExampleLibrary)

# Project libraries
add_subdirectory(CommonMod)

# Mods
add_subdirectory(ExampleMod) # CHANGE ME
