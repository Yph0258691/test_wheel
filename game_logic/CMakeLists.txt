#查找当下的所有的cpp
FILE(GLOB SOURCE_GAMELOGIC "./*.cpp")
#生成静态库
ADD_LIBRARY(game_logic STATIC ${SOURCE_GAMELOGIC})
#生成动态库
#ADD_LIBRARY(game_logic SHARED ${SOURCE_GAMELOGIC})
#install(TARGETS game_logic DESTINATION static_lib)
