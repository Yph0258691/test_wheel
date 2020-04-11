cmake -DCMAKE_BUILD_TYPE=Release .. //编译release 
cmake -DCMAKE_BUILD_TYPE=debug .. //编译debug
cmake -DCMAKE_INSTALL_PREFIX=/usr //指定安装路径
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Release .. 
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Debug ..
#使用gzip
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Release -DUSE_ZIBLIBARY=ON .. 
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Debug -DUSE_ZIBLIBARY=ON  ..
#使用ssl
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Release -DUSE_BOOSTSSLLIBARY=ON .. 
cmake -DCMAKE_INSTALL_PREFIX=/home/yinpinghua/test_wheel -DCMAKE_BUILD_TYPE=Debug -DUSE_BOOSTSSLLIBARY=ON ..