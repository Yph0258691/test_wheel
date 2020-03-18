#ADD_EXECUTABLE(test_wheel main.cpp ProxyEngine.cpp)
#TARGET_LINK_LIBRARIES(test_wheel)
FILE(GLOB SOURCE_MAIN "./*.cpp")

ADD_EXECUTABLE("${PROJECT_NAME}" ${SOURCE_MAIN})

TARGET_LINK_LIBRARIES(test_wheel  game_logic)

install (TARGETS "${PROJECT_NAME}" DESTINATION bin) 
