#Cmake最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8) 
#项目名 
PROJECT(test_wheel)
#安装路径写死
#SET(CMAKE_INSTALL_PREFIX "/home/test_wheel")
# 版本号 1.0.1
set (TEST_WHEEL_VERSION_MINOR 1)
set (TEST_WHEEL_VERSION_MINOR 0)
set (TEST_WHEEL_VERSION_MINOR 1)
#设置C编辑器						
SET(CMAKE_C_COMPILER gcc)
#设置c++编辑器			
SET(CMAKiE_CXX_COMPILER g++)
#默认开启debug
set(CMAKE_BUILD_TYPE "Debug")
#设置C++编辑选项			
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -Wall -fpic -fPIC -lpthread")
#设置C++debug版本编辑选项 --D_DEBUG宏 --ggdb生成debug信息	
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -ggdb")
#设置C++release编辑选项 -02表示优化级别
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2")
#预定义的宏
ADD_DEFINITIONS(-DPROTOBUF_INLINE_NOT_IN_HEADERS=0)  
#工程选项根目录，CMAKE_SOURCE_DIR为当前目录
SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/../")		
#可执行文件生成位置
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/") 
#库文件生成位置
SET(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")		
#第三方库搜索头文件
INCLUDE_DIRECTORIES($ENV{GSL_ROOT}/include /usr/local/include)
#编译链接时能搜索到自己的库，上面生成的lib库在这里链接
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/)

#第三方库的lib
INCLUDE_DIRECTORIES($ENV{GSL_ROOT}/lib/ /usr/lib/)
	
INCLUDE_DIRECTORIES(${PROJECT_ROOT_PATH}/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/game_logic)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/third_party)
INCLUDE_DIRECTORIES(.)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/ DESTINATION include/third_party/ )
install(DIRECTORY ./wheel/ DESTINATION include/wheel/ )
#增加子文件
ADD_SUBDIRECTORY(game_logic)
ADD_SUBDIRECTORY(src)
