#ADD_EXECUTABLE(test_wheel main.cpp ProxyEngine.cpp)
#TARGET_LINK_LIBRARIES(test_wheel)
file(GLOB SOURCE_MAIN "./*.cpp")

add_executable("${PROJECT_NAME}" ${SOURCE_MAIN})

#链接指定的静态库
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  TARGET_LINK_LIBRARIES("${PROJECT_NAME}" game_logic ws2_32 wsock32 libmysql)
  else()
   TARGET_LINK_LIBRARIES("${PROJECT_NAME}" game_logic)
endif()

install (TARGETS "${PROJECT_NAME}" DESTINATION bin) 
