############################################################################
# CMakeLists.txt
# Copyright (C) 2014  Belledonne Communications, Grenoble France
#
############################################################################
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
############################################################################

cmake_minimum_required(VERSION 2.6)
project(MEDIASTREAMER2 C CXX)


set(MEDIASTREAMER_MAJOR_VERSION "2")
set(MEDIASTREAMER_MINOR_VERSION "10")
set(MEDIASTREAMER_MICRO_VERSION "0")
set(MEDIASTREAMER_VERSION "${MEDIASTREAMER_MAJOR_VERSION}.${MEDIASTREAMER_MINOR_VERSION}.${MEDIASTREAMER_MICRO_VERSION}")
set(MEDIASTREAMER_SO_VERSION "3")

string(REGEX MATCH "^arm*" FIXED_POINT_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
if(FIXED_POINT_PROCESSOR)
	set(ENABLE_FIXED_POINT_DEFAULT_VALUE YES)
else()
	set(ENABLE_FIXED_POINT_DEFAULT_VALUE NO)
endif()
if(IOS)
	set(ENABLE_MACAQSND_DEFAULT_VALUE YES)
else()
	set(ENABLE_MACAQSND_DEFAULT_VALUE NO)
endif()
set(ENABLE_GLX_DEFAULT_VALUE NO)
set(ENABLE_X11_DEFAULT_VALUE NO)
if(UNIX)
	if(NOT APPLE)
		set(ENABLE_GLX_DEFAULT_VALUE YES)
		set(ENABLE_X11_DEFAULT_VALUE YES)
	endif()
endif()
if(UNIX AND NOT APPLE)
	set(LINUX_OR_BSD 1)
endif()

include(CMakeDependentOption)

option(ENABLE_STATIC "Build static library (default is shared library)." NO)
option(ENABLE_DEBUG_LOGS "Enable the display of traces showing the execution of the library." NO)
option(ENABLE_FIXED_POINT "Turn on fixed point computations." ${ENABLE_FIXED_POINT_DEFAULT_VALUE})
option(ENABLE_RELATIVE_PREFIX "Find resources relatively to the installation directory." NO)

option(ENABLE_SOUND "Can be used to turn off all possible sound backends." YES)
cmake_dependent_option(ENABLE_ALSA "Enable ALSA support." YES "ENABLE_SOUND;LINUX_OR_BSD" NO)
cmake_dependent_option(ENABLE_ARTSC "Enable artsc (KDE<4) sound input/output." NO "ENABLE_SOUND;LINUX_OR_BSD" NO)
cmake_dependent_option(ENABLE_MACSND "Enable MAC OS X Audio Units sound support." YES "ENABLE_SOUND;APPLE" NO)
cmake_dependent_option(ENABLE_MACAQSND "Enable native MAC OS X Audio Queue sound support." ${ENABLE_MACAQSND_DEFAULT_VALUE} "ENABLE_SOUND;APPLE" NO)
cmake_dependent_option(ENABLE_QSA "Enable QSA (QNX Sound Architecture) support." NO "ENABLE_SOUND" NO)
cmake_dependent_option(ENABLE_OSS "Enable OSS support." YES "ENABLE_SOUND;LINUX_OR_BSD" NO)
cmake_dependent_option(ENABLE_PORTAUDIO "Enable portaudio native support." NO "ENABLE_SOUND" NO)
cmake_dependent_option(ENABLE_PULSEAUDIO "Enable pulseaudio support." YES "ENABLE_SOUND" NO)
option(ENABLE_GSM "Build mediastreamer2 with the GSM codec." YES)
option(ENABLE_OPUS "Build mediastreamer2 with the OPUS codec." YES)
option(ENABLE_RESAMPLE "Build mediastreamer2 with the resampling capability." YES)
option(ENABLE_SPEEX "Build mediastreamer2 with the SPEEX codec." YES)

option(ENABLE_VIDEO "Build mediastreamer2 with video support." YES)
cmake_dependent_option(ENABLE_FFMPEG "Build mediastreamer2 with ffmpeg video support." YES "ENABLE_VIDEO" NO)
cmake_dependent_option(ENABLE_GLX "Enable X11+OpenGL rendering support (requires glx and glew)." ${ENABLE_GLX_DEFAULT_VALUE} "ENABLE_VIDEO;UNIX" NO)
cmake_dependent_option(ENABLE_SDL "Enable SDL support." NO "ENABLE_FFMPEG" NO)
cmake_dependent_option(ENABLE_V4L "Enable Video4Linux support." YES "ENABLE_VIDEO;LINUX_OR_BSD" NO)
cmake_dependent_option(ENABLE_X11 "Enable X11 support." ${ENABLE_X11_DEFAULT_VALUE} "ENABLE_FFMPEG;UNIX" NO)
cmake_dependent_option(ENABLE_XV "Enable XV support." ${ENABLE_X11_DEFAULT_VALUE} "ENABLE_FFMPEG;UNIX" NO)
cmake_dependent_option(ENABLE_THEORA "Enable theora video codec support." YES "ENABLE_VIDEO" NO)
cmake_dependent_option(ENABLE_VPX "Build mediastreamer2 with VPX codec." YES "ENABLE_VIDEO" NO)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_PREFIX_PATH}/share/cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckIncludeFile)
include(CheckLibraryExists)

if(MSVC)
	list(APPEND CMAKE_REQUIRED_INCLUDES ${CMAKE_PREFIX_PATH}/include/MSVC)
endif()

find_package(Threads)

check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(poll.h HAVE_POLL_H)
check_include_file(sys/poll.h HAVE_SYS_POLL_H)
check_include_file(sys/uio.h HAVE_SYS_UIO_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(sys/shm.h HAVE_SYS_SHM_H)
check_include_file(windows.h HAVE_WINDOWS_H)
check_include_file(dlfcn.h HAVE_DLFCN_H)
check_include_file(alloca.h HAVE_ALLOCA_H)
if(ENABLE_OSS)
	check_include_file(soundcard.h HAVE_SOUNDCARD_H)
	check_include_file(sys/soundcard.h HAVE_SYS_SOUNDCARD_H)
	check_include_file(machine/soundcard.h HAVE_MACHINE_SOUNDCARD_H)
	check_include_file(sys/audio.h HAVE_SYS_AUDIO_H)
	if(NOT (HAVE_SOUNDCARD_H OR HAVE_SYS_SOUNDCARD_H OR HAVE_MACHINE_SOUNDCARD_H OR HAVE_SYS_AUDIO_H))
		set(ENABLE_OSS OFF CACHE BOOL "Enable OSS support." FORCE)
	endif()
endif()

check_library_exists("dl" "dlopen" "" HAVE_DLOPEN)

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

find_package(ORTP REQUIRED)
if(ENABLE_GSM)
	find_package(GSM)
	if(NOT GSM_FOUND)
		message(WARNING "Could not find GSM library, mediastreamer2 will be compiled without the GSM codec.")
		set(ENABLE_GSM OFF CACHE BOOL "Build mediastreamer2 with the GSM codec." FORCE)
	endif()
endif()
if(ENABLE_OPUS)
	find_package(Opus)
	if(NOT OPUS_FOUND)
		message(WARNING "Could not find opus library, mediastreamer2 will be compiled without the OPUS codec.")
		set(ENABLE_OPUS OFF CACHE BOOL "Build mediastreamer2 with the OPUS codec." FORCE)
	endif()
endif()
if(ENABLE_SPEEX)
	find_package(Speex)
	if(NOT SPEEX_FOUND)
		message(WARNING "Could not find speex library, mediastreamer2 will be compiled without the SPEEX codec.")
		set(ENABLE_SPEEX OFF CACHE BOOL "Build mediastreamer2 with the SPEEX codec." FORCE)
	endif()
endif()
if(ENABLE_ALSA)
	find_package(ALSA)
	if(NOT ALSA_FOUND)
		set(ENABLE_ALSA OFF CACHE BOOL "Enable ALSA support." FORCE)
	endif()
endif()
if(ENABLE_ARTSC)
	find_package(Arts REQUIRED)
endif()
if(ENABLE_PORTAUDIO)
	find_package(PortAudio REQUIRED)
endif()
if(ENABLE_PULSEAUDIO)
	find_package(PulseAudio)
	if(NOT PULSEAUDIO_FOUND)
		set(ENABLE_PULSEAUDIO OFF CACHE BOOL "Enable pulseaudio support." FORCE)
	endif()
endif()
if(ENABLE_QSA)
	find_package(QSA REQUIRED)
endif()
if(ENABLE_VIDEO)
	add_definitions(-DVIDEO_ENABLED)
endif()
if(ENABLE_FFMPEG)
	find_package(FFMpeg REQUIRED)
else()
	add_definitions(-DNO_FFMPEG)
endif()
if(ENABLE_V4L)
	find_package(V4L)
	if(NOT V4L_FOUND)
		message(FATAL_ERROR "Missing V4L. It is highly recommended to build with libv4l2 headers and library. Many camera will won't work or will crash your application if libv4l2 is not installed. If you know what you are doing, you can use -DENABLE_V4L=0 to disable this check.")
	endif()
endif()
if(ENABLE_X11)
	find_package(X11)
	if(NOT X11_FOUND)
		set(ENABLE_X11 OFF CACHE BOOL "Enable X11 support." FORCE)
	endif()
endif()
if(ENABLE_XV)
	find_package(Xv)
	if(XV_FOUND)
		set(HAVE_XV 1)
	else()
		set(ENABLE_XV OFF CACHE BOOL "Enable XV support." FORCE)
	endif()
endif()
if(ENABLE_GLX)
	find_package(GLX)
	if(GLX_FOUND)
		set(HAVE_GL 1)
	else()
		set(ENABLE_GLX OFF CACHE BOOL "Enable X11+OpenGL rendering support (requires glx and glew)." FORCE)
	endif()
endif()
# TODO: Handle SDL option
# TODO: Handle Theora option
if(ENABLE_VPX)
	find_package(VPX)
	if(NOT VPX_FOUND)
		set(ENABLE_VPX OFF CACHE BOOL "Build mediastreamer2 with VPX codec." FORCE)
	endif()
endif()


if(ENABLE_SOUND AND NOT(WIN32 OR ENABLE_ALSA OR ENABLE_ARTSC OR ENABLE_MACSND OR ENABLE_MACAQSND OR ENABLE_OSS OR ENABLE_PORTAUDIO OR ENABLE_PULSEAUDIO OR ENABLE_QSA))
	message(FATAL_ERROR "Could not find a support sound driver API. Use -DENABLE_SOUND=NO if you don't care about having sound.")
endif()


include_directories(
	include/
	src/utils/
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/src/
)

if(ENABLE_VIDEO)
	include_directories(src/voip)
endif()
if(GSM_FOUND)
	include_directories(${GSM_INCLUDE_DIRS})
endif()
if(OPUS_FOUND)
	include_directories(${OPUS_INCLUDE_DIRS})
endif()
if(SPEEX_FOUND)
	include_directories(${SPEEX_INCLUDE_DIRS})
endif()
if(ALSA_FOUND)
	include_directories(${ALSA_INCLUDE_DIRS})
endif()
if(ARTS_FOUND)
	include_directories(${ARTS_INCLUDE_DIRS})
endif()
if(PORTAUDIO_FOUND)
	include_directories(${PORTAUDIO_INCLUDE_DIRS})
endif()
if(PULSEAUDIO_FOUND)
	include_directories(${PULSEAUDIO_INCLUDE_DIRS})
endif()
if(QSA_FOUND)
	include_directories(${QSA_INCLUDE_DIRS})
endif()
if(FFMPEG_FOUND)
	include_directories(${FFMPEG_INCLUDE_DIRS})
endif()
if(V4L_FOUND)
	include_directories(${V4L_INCLUDE_DIRS})
endif()
if(X11_FOUND)
	include_directories(${X11_INCLUDE_DIRS})
endif()
if(XV_FOUND)
	include_directories(${XV_INCLUDE_DIRS})
endif()
if(GLX_FOUND)
	include_directories(${GLX_INCLUDE_DIRS})
endif()
if(VPX_FOUND)
	include_directories(${VPX_INCLUDE_DIRS})
endif()
if(MSVC)
	include_directories(${CMAKE_PREFIX_PATH}/include/MSVC)
endif()


if(ENABLE_DEBUG_LOGS)
	add_definitions(-DDEBUG)
endif()
if(ENABLE_FIXED_POINT)
	set(MS_FIXED_POINT 1)
endif()
if(ENABLE_ALSA)
	set(__ALSA_ENABLED__ 1)
endif()
if(ENABLE_ARTSC)
	set(__ARTS_ENABLED__ 1)
endif()
if(ENABLE_MACSND)
	set(__MACSND_ENABLED__ 1)
endif()
if(ENABLE_MACAQSND)
	set(__MAC_AQ_ENABLED__ 1)
endif()
if(ENABLE_PORTAUDIO)
	set(__PORTAUDIO_ENABLED__ 1)
endif()
if(ENABLE_PULSEAUDIO)
	set(__PULSEAUDIO_ENABLED__ 1)
endif()
if(ENABLE_QSA)
	set(__QSA_ENABLED__ 1)
endif()
if(SPEEX_FOUND)
	set(HAVE_SPEEXDSP 1)
endif()
if(ENABLE_RELATIVE_PREFIX)
	set(PACKAGE_PLUGINS_DIR "./lib/mediastreamer/plugins")
	set(PACKAGE_DATA_DIR "./share")
else()
	set(PACKAGE_PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/lib/mediastreamer/plugins")
	set(PACKAGE_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")
endif()
if(APPLE)
	set(APPLE_SPECIFIC_LINK_FLAGS )
	if(ENABLE_VIDEO)
		list(APPEND APPLE_SPECIFIC_LINK_FLAGS "-framework Cocoa" "-framework OpenGL" "-framework QuartzCore" "-framework QTKit")
	endif()
	if(ENABLE_MACSND)
		list(APPEND APPLE_SPECIFIC_LINK_FLAGS "-framework Carbon" "-framework AudioUnit" "-framework CoreAudio -framework CoreServices")
	endif()
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mediastreamer-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/mediastreamer-config.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/mediastreamer-config.h PROPERTIES GENERATED ON)
add_definitions(-DHAVE_CONFIG_H)
add_definitions(${ORTP_CPPFLAGS})


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindMS2.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/FindMS2.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FindMS2.cmake
	DESTINATION share/cmake/Modules
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)


add_subdirectory(cmake)
add_subdirectory(help)
add_subdirectory(include)
add_subdirectory(src)
