# Install script for directory: /Users/jay/AndroidStudioProjects/Ne10/android/NE10Demo/jni

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/jay/AndroidStudioProjects/Ne10/android/NE10Demo/jni/../libs/armeabi/libNE10_test_demo.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/Users/jay/AndroidStudioProjects/Ne10/android/NE10Demo/jni/../libs/armeabi" TYPE SHARED_LIBRARY FILES "/Users/jay/AndroidStudioProjects/Ne10/build/android/NE10Demo/jni/CMakeFiles/CMakeRelink.dir/libNE10_test_demo.so")
  if(EXISTS "$ENV{DESTDIR}/Users/jay/AndroidStudioProjects/Ne10/android/NE10Demo/jni/../libs/armeabi/libNE10_test_demo.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/jay/AndroidStudioProjects/Ne10/android/NE10Demo/jni/../libs/armeabi/libNE10_test_demo.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Users/jay/Library/Android/sdk/ndk-bundle/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-strip" "$ENV{DESTDIR}/Users/jay/AndroidStudioProjects/Ne10/android/NE10Demo/jni/../libs/armeabi/libNE10_test_demo.so")
    endif()
  endif()
endif()

