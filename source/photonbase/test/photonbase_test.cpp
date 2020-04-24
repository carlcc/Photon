#include "TestSerializer.h"
#include "TestVariant.h"
#include <iostream>

int main()
{
    using namespace pht;
    TestVariant::test();
    TestSerializer::test();

    std::cout << "All tests passed" << std::endl;
    return 0;
}