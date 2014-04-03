#include <iostream>
#include "mongo/integration/integration.h"

extern ParameterStruct TestParams;

TEST(DBClient, whatever) {
    std::cout << TestParams.port << std::endl;
}
