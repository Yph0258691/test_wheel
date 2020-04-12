#ADD_EXECUTABLE(test_wheel main.cpp ProxyEngine.cpp)
#TARGET_LINK_LIBRARIES(test_wheel)
file(GLOB SOURCE_MAIN "./*.cpp")

add_executable("${PROJECT_NAME}" ${SOURCE_MAIN})

#链接指定的静态库
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(lib_name game_logic)
  list(APPEND lib_name ws2_32)
  list(APPEND lib_name wsock32)
  list(APPEND lib_name libmysql)

  if(USE_ZIBLIBARY)
    list(APPEND lib_name zdll)
  endif(USE_ZIBLIBARY)

  if(USE_BOOSTSSLLIBARY)
    list(APPEND lib_name ssleay32.lib)
    list(APPEND lib_name libeay32.lib)
  endif(USE_BOOSTSSLLIBARY)
  
  TARGET_LINK_LIBRARIES("${PROJECT_NAME}" ${lib_name})
  else()
   #注意在linux下的动态库，不能前面在lib的前缀，比如libmysql.so,这个和vs不同之处
   TARGET_LINK_LIBRARIES("${PROJECT_NAME}" game_logic mysql)
endif()

set(dll_path ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/x64/libmysql.dll)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(dll_path ${CMAKE_SOURCE_DIR}/third_party/mysql/lib/win32/libmysql.dll)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

#cmake之后，再编译运行的时候拷贝

if(MSVC)
  if("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    set(shared_dll_path  ${EXECUTABLE_OUTPUT_PATH}/Debug/)
  else()
    set(shared_dll_path  ${EXECUTABLE_OUTPUT_PATH}/Release/)
  endif()
 else()
   set(shared_dll_path  ${EXECUTABLE_OUTPUT_PATH})
endif(MSVC)

#编译之后导入d11
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    #安装文件夹
    file(MAKE_DIRECTORY ${shared_dll_path})

    add_custom_target(CopyBinaries 
      COMMAND ${CMAKE_COMMAND} -E copy ${dll_path} ${shared_dll_path}

       #Output Message
       COMMENT "Copying shared dll binaries from '${dll_path}' to '${shared_dll_path}'" VERBATIM
   )

    #和上一步骤合并一个步骤
   add_dependencies(${project_name} CopyBinaries)

endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

#install (DIRECTORY ${EXECUTABLE_OUTPUT_PATH} DESTINATION bin) 
INSTALL(TARGETS ${project_name} game_logic 
       RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
       ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
