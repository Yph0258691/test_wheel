#Cmake最低版本要求
cmake_minimum_required(VERSION 3.10.0) 

set(project_name test_wheel)
#设置项目名 
project(${project_name})
#安装路径写死
#SET(CMAKE_INSTALL_PREFIX "D:/test_wheel/")

#设置版本号 1.0.1
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 1)

#取消第三方库头文件进行依赖检查,liunx加快编译速度
set(CMAKE_DEPENDS_IN_PROJECT_ONLY ON)

#输出编译器的类型
if(CMAKE_CONFIGURATION_TYPES)
    message("--Multi-configuration generator")
else()
    message("----Single-configuration generator")
endif()

#输出安装目录
message("----CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}")

#默认开启Debug
#liunx下是not,注意为空格式
if(NOT CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" MATCHES  "None" OR "${CMAKE_BUILD_TYPE}" MATCHES  " ")
   set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build configuration" FORCE)
	 message("----CMAKE_BUILD_TYPE FORCE SET Debug")
else()
#校验
   if("${CMAKE_BUILD_TYPE}" MATCHES  "Release" OR "${CMAKE_BUILD_TYPE}" MATCHES  "RelWithDebInfo" OR "${CMAKE_BUILD_TYPE}" MATCHES  "MinSizeRel" 
   OR "${CMAKE_BUILD_TYPE}" MATCHES  "Debug")
     set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
     message("----set CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")  
  else()
     message("----CMAKE_BUILD_TYPE is not right")  
   endif()
endif()


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
   if (MSVC_VERSION LESS 1900)
     message("--- less vs2015 version")
    endif()

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
  set(CMAKAE_CXX_COMPILER g++)

  if(NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(make_mode c++14)
    message("-- set ${make_mode} make mode")
    set(CMAKE_CXX_FLAGS -std=${make_mode})
  endif()

    #设置C++编辑选项			
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fpic -fPIC -lpthread")
  
  if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    message("-- CMAKE_BUILD_TYPE is  ${CMAKE_BUILD_TYPE} mode")
	#设置C++debug版本编辑选项 --D_DEBUG宏 --ggdb生成debug信息	
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -ggdb")
  else()
    message("-- CMAKE_BUILD_TYPE is  ${CMAKE_BUILD_TYPE} mode")
	#设置C++release编辑选项 -02表示优化级别
   set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")
  endif()
endif ()

#提示默认安装路径
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   message("-- Default Install prefix: ${CMAKE_INSTALL_PREFIX}")
endif()

#预定义的宏
add_definitions(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0)  
#工程选项根目录，CMAKE_SOURCE_DIR为当前目录
set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/../")		
#可执行文件生成位置
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/") 
#库文件生成位置
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")	
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
  
#创建文件夹
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

if (CMAKE_SYSTEM_NAME MATCHES "Linux")	
  #第三方库搜索头文件
  include_directories($ENV{GSL_ROOT}/include /usr/local/include)
  #第三方库的lib
  link_directories($ENV{GSL_ROOT}/lib/ /usr/lib/)
endif() 

#windows下设置boost库的位置
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(Boost_USE_STATIC_LIBS ON) 
  set(Boost_USE_MULTITHREADED ON)  
  set(Boost_USE_STATIC_RUNTIME ON) 
 # add_definitions(-D_WIN32_WINNT=0x0501)

  set(boost_lib_path "D:/project/C++Project/thirdpaty/boost")
  set(BOOST_ROOT ${boost_lib_path})
  find_package(Boost) 
  if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS}) 
	if(CMAKE_CL_64)
     link_directories(${Boost_LIBRARY_DIRS}/x64/) 
	 else()
	 link_directories(${Boost_LIBRARY_DIRS}/x86/) 
	endif()
  endif()
endif()

include_directories(${PROJECT_ROOT_PATH}/)
include_directories(${CMAKE_SOURCE_DIR}/game_logic)
include_directories(${CMAKE_SOURCE_DIR}/third_party)
include_directories(${CMAKE_SOURCE_DIR}/third_party/mysql)
if(CMAKE_CL_64)
  link_directories(${CMAKE_SOURCE_DIR}/third_party/mysql/lib/x64)
 else()
  link_directories(${CMAKE_SOURCE_DIR}/third_party/mysql/lib/win32)
endif()
  #编译链接时能搜索到自己的库，上面生成的lib库在这里链接
link_directories(${CMAKE_SOURCE_DIR}/lib/)
include_directories(.)

#linux下存宏定义
#if(CMAKE_SYSTEM_NAME MATCHES "Linux")
 include_directories(${CMAKE_PKGCONFIG_OUTPUT_DIRECTORY})
#endif()

#安装相关库
install(DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/ DESTINATION include/third_party/ )
install(DIRECTORY ./wheel/ DESTINATION include/wheel/ )

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  install(FILES ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/x64/libmysql.so DESTINATION bin)
  install(FILES ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/x64/libmysql.so DESTINATION ${CMAKE_BINARY_DIR}/bin)
else() 
  if(CMAKE_CL_64)
    install(FILES ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/x64/libmysql.dll DESTINATION bin)
    install(FILES ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/x64/libmysql.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
  else()
    install(FILES ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/win32/libmysql.dll DESTINATION bin)
    install(FILES ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/win32/libmysql.dll DESTINATION ${CMAKE_BINARY_DIR}/bin)
  endif(CMAKE_CL_64)
endif()

#增加子文件
add_subdirectory(game_logic)
add_subdirectory(src)

#放在install后的目的是,先编译install再执行package此地方要注意
#包含此平台所有的东西
include (InstallRequiredSystemLibraries)
#让cmake给我生成证书,若linux下安装,Linux下安装会提示
set (CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
#判断系统平台
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	message(STATUS "current platform: Linux ")
else()
   set(CPACK_GENERATOR NSIS)
   set(CPACK_NSIS_MODIFY_PATH ON)
   message(STATUS "current platform: Windows")
endif() 

#当前Cmakelist先生成pack文件夹
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/pack)
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
#版本号
set(PROJECT_VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
#设置包的后缀名
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
#set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_SOURCE_GENERATOR "TGZ:ZIP")
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

#新增测试
#include(CTest)
#这运行起来会导致程序卡主
#dd_test("test1" "${EXECUTABLE_OUTPUT_PATH}/${project_name}")

