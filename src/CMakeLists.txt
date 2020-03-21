#ADD_EXECUTABLE(test_wheel main.cpp ProxyEngine.cpp)
#TARGET_LINK_LIBRARIES(test_wheel)
file(GLOB SOURCE_MAIN "./*.cpp")

add_executable("${PROJECT_NAME}" ${SOURCE_MAIN})

#可以解决vs链接问题
TARGET_LINK_LIBRARIES("${PROJECT_NAME}" game_logic)
link_directories("${PROJECT_NAME}"  ${LIBRARY_OUTPUT_PATH})

install (TARGETS "${PROJECT_NAME}" DESTINATION bin) 
