#查找当下的所有的cpp
FILE(GLOB SOURCE_GAMELOGIC "./*.cpp")
#生成静态库
ADD_LIBRARY(game_logic STATIC ${SOURCE_GAMELOGIC})

