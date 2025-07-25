###########################################################################
#   fheroes2: https://github.com/ihhub/fheroes2                           #
#   Copyright (C) 2022 - 2025                                             #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
###########################################################################

cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0074 NEW)

file(STRINGS ${CMAKE_SOURCE_DIR}/version.txt FHEROES2_VERSION LIMIT_COUNT 1 REGEX "^[0-9]+\.[0-9]+\.[0-9]+$")

project(fheroes2 VERSION ${FHEROES2_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CMakeDependentOption)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#
# Compile-time options
#
option(ENABLE_IMAGE "Enable the use of SDL_image (requires libpng)" OFF)
option(ENABLE_TOOLS "Enable the build of additional tools" OFF)

# Available only on macOS
cmake_dependent_option(MACOS_APP_BUNDLE "Create a Mac app bundle" OFF "APPLE" OFF)

# Should not be available if we are creating a Mac app bundle
cmake_dependent_option(GET_HOMM2_DEMO "Fetch and install the HoMM II demo data" OFF "NOT MACOS_APP_BUNDLE" OFF)

set(USE_SDL_VERSION SDL2 CACHE STRING "Version of the SDL library used")
set(FHEROES2_DATA ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME} CACHE STRING "Path to the fheroes2 data directory (relative to CMAKE_INSTALL_PREFIX)")

# For ComboBox in CMake GUI
set_property(CACHE USE_SDL_VERSION PROPERTY STRINGS SDL2)

#
# Library & feature detection
#
find_package(${USE_SDL_VERSION} REQUIRED)
find_package(${USE_SDL_VERSION}_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads)

if(ENABLE_IMAGE)
	find_package(${USE_SDL_VERSION}_image REQUIRED)
	find_package(PNG REQUIRED)
endif(ENABLE_IMAGE)

#
# Source files
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)

#
# Translation files
#
if(NOT WIN32)
	find_program(MAKE_EXECUTABLE make)

	add_custom_target(
		translations
		ALL
		COMMAND ${MAKE_EXECUTABLE}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/files/lang
		)
endif(NOT WIN32)

#
# macOS app bundle
#
if(MACOS_APP_BUNDLE)
	add_custom_target(
		app_bundle_fh2m_maps
		ALL
		COMMAND mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/maps
		COMMAND cp *.fh2m ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/maps
		DEPENDS fheroes2
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/maps
		)
	add_custom_target(
		app_bundle_h2d_files
		ALL
		COMMAND mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/h2d
		COMMAND cp *.h2d ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/h2d
		DEPENDS fheroes2
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/files/data
		)
	add_custom_target(
		app_bundle_translations
		ALL
		COMMAND mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/translations
		COMMAND cp *.mo ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/Resources/translations
		DEPENDS translations
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/files/lang
		)
	add_custom_target(
		run_dylibbundler
		ALL
		COMMAND dylibbundler -od -b -x ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME} -d ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/libs
		DEPENDS fheroes2 app_bundle_fh2m_maps app_bundle_h2d_files app_bundle_translations
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		)
endif(MACOS_APP_BUNDLE)

#
# Installation of auxiliary files
#
if(NOT MACOS_APP_BUNDLE)
	install(
		DIRECTORY files/data/
		DESTINATION ${FHEROES2_DATA}/files/data
		FILES_MATCHING
		PATTERN *.h2d
		)
	install(
		DIRECTORY maps/
		DESTINATION ${FHEROES2_DATA}/maps
		FILES_MATCHING
		PATTERN *.fh2m
		)
	install(
		FILES docs/README.txt LICENSE changelog.txt
		DESTINATION ${CMAKE_INSTALL_DOCDIR}
		)

	if(GET_HOMM2_DEMO)
		include(FetchContent)

		set(FETCHCONTENT_QUIET OFF)

		FetchContent_Declare(
			demodata
			URL https://archive.org/download/HeroesofMightandMagicIITheSuccessionWars_1020/h2demo.zip
			URL_HASH SHA256=12048c8b03875c81e69534a3813aaf6340975e77b762dc1b79a4ff5514240e3c
			)
		FetchContent_MakeAvailable(demodata)

		file(COPY ${demodata_SOURCE_DIR}/DATA/ DESTINATION data PATTERN *.*)
		file(COPY ${demodata_SOURCE_DIR}/MAPS/ DESTINATION maps PATTERN *.*)

		install(
			DIRECTORY ${demodata_SOURCE_DIR}/DATA/
			DESTINATION ${FHEROES2_DATA}/data
			FILES_MATCHING
			PATTERN *.*
			)
		install(
			DIRECTORY ${demodata_SOURCE_DIR}/MAPS/
			DESTINATION ${FHEROES2_DATA}/maps
			FILES_MATCHING
			PATTERN *.*
			)
	endif(GET_HOMM2_DEMO)

	if(WIN32)
		install(
			FILES script/demo/download_demo_version.bat script/demo/download_demo_version.ps1
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/demo
			)
		install(
			FILES script/homm2/extract_homm2_resources.bat script/homm2/extract_homm2_resources.ps1
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/homm2
			)
		install(
			FILES script/homm2/resource_extraction_toolset.bat script/homm2/resource_extraction_toolset.ps1
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/homm2
			)
	else(WIN32)
		install(
			DIRECTORY files/lang/
			DESTINATION ${FHEROES2_DATA}/files/lang
			FILES_MATCHING
			PATTERN *.mo
			)
		install(
			FILES script/demo/download_demo_version.sh
			PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/demo
			)
		install(
			FILES script/homm2/extract_homm2_resources.sh
			PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
			DESTINATION ${CMAKE_INSTALL_DOCDIR}/homm2
			)
		install(
			FILES script/packaging/common/fheroes2.metainfo.xml
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
			)
		install(
			FILES src/resources/fheroes2.png
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps
			)
		install(
			FILES script/packaging/common/fheroes2.desktop
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
			)
	endif(WIN32)
endif(NOT MACOS_APP_BUNDLE)
