#include "test.h"
#include <config.h>

Test::Test()
{
#ifdef TEST1
	std::cout << "111" << std::endl;
#endif // TEST1
}

Test::~Test() {

}

void Test::display(){
	std::cout << "��ʼ����" << std::endl;
}
