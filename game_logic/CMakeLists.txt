#查找当下的所有的cpp
set(lib_name "game_logic")

file(GLOB SOURCE_GAMELOGIC "./*.cpp")
#生成静态库
add_library(${lib_name} STATIC ${SOURCE_GAMELOGIC})
#生成动态库
#ADD_LIBRARY(game_logic SHARED ${SOURCE_GAMELOGIC})
