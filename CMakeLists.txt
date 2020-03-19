#Cmake最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8) 

#项目名 
PROJECT(test_wheel)
#安装路径写死
#SET(CMAKE_INSTALL_PREFIX "/home/test_wheel")
# 版本号 1.0.1
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 1)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2 /bigobj")

	# was causing conflics with gtest build
	string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

	if ("${MSVC_RUNTIME_LIBRARY_CONFIG}" STREQUAL "")
		set(MSVC_RUNTIME_LIBRARY_CONFIG "/MT")
	endif ()

	foreach (flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		if ("${MSVC_RUNTIME_LIBRARY_CONFIG}" STREQUAL "/MT")
			string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		elseif ("${MSVC_RUNTIME_LIBRARY_CONFIG}" STREQUAL "/MD")
			string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
		else ()
			string(REPLACE "/MD" "${MSVC_RUNTIME_LIBRARY_CONFIG}" ${flag_var} "${${flag_var}}")
			string(REPLACE "/MT" "${MSVC_RUNTIME_LIBRARY_CONFIG}" ${flag_var} "${${flag_var}}")
		endif ()
	endforeach ()
	
    #增加宏定义
	#add_definitions(-D_UNICODE)
	#add_definitions(-DUNICODE)
	#add_definitions(-DWIN32_LEAN_AND_MEAN)
else ()
#设置C编辑器						
   SET(CMAKE_C_COMPILER gcc)
#设置c++编辑器			
   SET(CMAKiE_CXX_COMPILER g++)
#设置C++编辑选项			
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -Wall -fpic -fPIC -lpthread")
#设置C++debug版本编辑选项 --D_DEBUG宏 --ggdb生成debug信息	
   SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -ggdb")
#设置C++release编辑选项 -02表示优化级别
   SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")
endif ()

#默认开启Debug
IF(NOT CMAKE_CONFIGURAITON_TYPES)
 IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
     set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build configuration" FORCE)
	 message(FATAL_ERROR,"CMAKE_BUILD_TYPE FORCE SET Debug")
	 ENDIF()
ENDIF()

#提示默认安装路径
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   message(FATAL_ERROR,"MAKE_INSTALL_PREFIX FORCE SET DEFAULT PATH")
ENDIF()

#预定义的宏
ADD_DEFINITIONS(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0)  
#工程选项根目录，CMAKE_SOURCE_DIR为当前目录
SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/../")		
#可执行文件生成位置
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/") 
#库文件生成位置
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")	
#编译链接时能搜索到自己的库，上面生成的lib库在这里链接
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")	
  #第三方库搜索头文件
  INCLUDE_DIRECTORIES($ENV{GSL_ROOT}/include /usr/local/include)
  #第三方库的lib
  INCLUDE_DIRECTORIES($ENV{GSL_ROOT}/lib/ /usr/lib/)
ENDIF() 

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

#windows下设置boost库的位置
if (MSVC)
  set(BOOST_ROOT D:/project/C++Project/thirdpaty/boost)
  find_package(Boost) 
  if(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS}) 
	IF(CMAKE_CL_64)
     LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}/x64/) 
	 else()
	 LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}/) 
	endif()
  endif()
endif()

INCLUDE_DIRECTORIES(${PROJECT_ROOT_PATH}/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/game_logic)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/third_party)
INCLUDE_DIRECTORIES(.)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/ DESTINATION include/third_party/ )
install(DIRECTORY ./wheel/ DESTINATION include/wheel/ )
#增加子文件
ADD_SUBDIRECTORY(game_logic)
ADD_SUBDIRECTORY(src)

#放在install后的目的是,先编译install再执行package此地方要注意
#包含此平台所有的东西
include (InstallRequiredSystemLibraries)
#让cmake给我生成证书,若linux下安装,Linux下安装会提示
set (CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
#判断系统平台
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	MESSAGE(STATUS "current platform: Linux ")
ELSE()
set(CPACK_GENERATOR NSIS)
SET(CPACK_NSIS_MODIFY_PATH ON)
	MESSAGE(STATUS "current platform: Windows")
ENDIF() 

#当前Cmakelist先生成pack文件夹
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/pack)
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
#版本号
set(PROJECT_VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
#设置包的后缀名
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION_FULL})
#安装包
include(CPack)
