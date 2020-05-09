#include "TestRemoteMethodBinding.h"
#include "TestSerializer.h"
#include "TestVariant.h"
#include <iostream>

int main()
{
    using namespace pht;
    TestVariant::test();
    TestSerializer::test();
    TestRemoteMethodBinding::test();

    std::cout << "All tests passed" << std::endl;
    return 0;
}