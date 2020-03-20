#Cmake最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 3.10.0) 

set(project_name test_wheel)
#设置项目名 
PROJECT(${project_name})
#安装路径写死
#SET(CMAKE_INSTALL_PREFIX "/home/test_wheel")

#设置版本号 1.0.1
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 1)

#取消第三方库头文件进行依赖检查
set(CMAKE_DEPENDS_IN_PROJECT_ONLY ON)

#通过开关设置debug还是release
option (MAKE_RELEASE
       "Use provided  MAKE_RELEASE implementation" OFF)
	   
IF(MAKE_RELEASE)
 SET(CMAKE_BUILD_TYPE Release)
else()
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

#默认开启Debug
IF(NOT CMAKE_CONFIGURAITON_TYPES)
 IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
     set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build configuration" FORCE)
	 message("-- CMAKE_BUILD_TYPE FORCE SET Debug")
	 ENDIF()
ENDIF()

#检测linux下c++编译版版本
if(CMAKE_SYSTEM_NAME MATCHES "Linux")

  include(CheckCXXCompilerFlag)

 foreach(cxx_feature_name "c++14" "c++1y" "c++11" "c++0x")
    set(cxx_feature_flag "-std=${cxx_feature_name}")
    string(REPLACE "+" "x" cxx_feature_suffix ${cxx_feature_name})
    CHECK_CXX_COMPILER_FLAG(${cxx_feature_flag} COMPILER_SUPPORTS_${cxx_feature_suffix})
    if(COMPILER_SUPPORTS_${cxx_feature_suffix})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cxx_feature_flag}")
        message("-- The compiler ${BUILDTOOL_PREFIX}-g++ has ${cxx_feature_name} support")
        break()
    endif()
 endforeach()
endif()

#设置编译器格式
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
	#add_definitions(-DTEST)
	#add_definitions(-DUNICODE)
	#add_definitions(-DWIN32_LEAN_AND_MEAN)
else ()
  #设置c++编辑器	  
  SET(CMAKiE_CXX_COMPILER g++)
    #设置C++编辑选项			
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fpic -fPIC -lpthread")
  
  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("-- Debug mode")
	#设置C++debug版本编辑选项 --D_DEBUG宏 --ggdb生成debug信息	
   SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -ggdb")
  else()
    message("-- Release mode")
	#设置C++release编辑选项 -02表示优化级别
   SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")
  endif()
endif ()

#提示默认安装路径
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   message("-- Default Install prefix: ${CMAKE_INSTALL_PREFIX}")
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
set(CMAKE_PKGCONFIG_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH}/pkgconfig)

#增加宏定义，整个相关目录都支持
#增加整个项目宏
#if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  option(TEST "Test" ON)
  option(TEST1 "TEST1" ON)
  configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${CMAKE_PKGCONFIG_OUTPUT_DIRECTORY}/config.h"
  )
#endif()
  
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")	
  #第三方库搜索头文件
  INCLUDE_DIRECTORIES($ENV{GSL_ROOT}/include /usr/local/include)
  #第三方库的lib
  INCLUDE_DIRECTORIES($ENV{GSL_ROOT}/lib/ /usr/lib/)
ENDIF() 

#windows下设置boost库的位置
if (MSVC)
  set(Boost_USE_STATIC_LIBS ON) 
  set(Boost_USE_MULTITHREADED ON)  
  set(Boost_USE_STATIC_RUNTIME ON) 

  set(BOOST_ROOT D:/project/C++Project/thirdpaty/boost)
  find_package(Boost) 
  if(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS}) 
	IF(CMAKE_CL_64)
     LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}/x64/) 
	 else()
	 LINK_DIRECTORIES(${Boost_LIBRARY_DIRS}/x86/) 
	endif()
  endif()
endif()

INCLUDE_DIRECTORIES(${PROJECT_ROOT_PATH}/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/game_logic)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/third_party)
INCLUDE_DIRECTORIES(.)

#linux下存宏定义
#if(CMAKE_SYSTEM_NAME MATCHES "Linux")
 INCLUDE_DIRECTORIES(${CMAKE_PKGCONFIG_OUTPUT_DIRECTORY})
#endif()

#安装相关库
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

# 写入库的版本信息
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        MyMathConfigVersion.cmake
        VERSION ${PROJECT_VERSION_FULL}
        COMPATIBILITY AnyNewerVersion  # 表示该函数库向下兼容
        )

#安装包
include(CPack)
