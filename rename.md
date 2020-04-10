cmake -DCMAKE_BUILD_TYPE=Release .. //编译release 
cmake -DCMAKE_BUILD_TYPE=debug .. //编译debug
cmake -DCMAKE_INSTALL_PREFIX=/usr //指定安装路径
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Release .. 
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Release -DUSE_ZIBLIBARY=ON .. 
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Debug -DUSE_ZIBLIBARY=ON  ..