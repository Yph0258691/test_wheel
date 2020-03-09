#查找当下的所有的cpp
FILE(GLOB SOURCE_MAIN "./*.cpp")
#生成可执行文件
ADD_EXECUTABLE(test_wheel ${SOURCE_MAIN})
TARGET_LINK_LIBRARIES(test_wheel  game_logic)

