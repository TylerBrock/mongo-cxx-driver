#include <iostream>

#include <gtest/gtest.h>
#include <mongo/client/init.h>

int main(int argc, char **argv) {
    const char *port = "27017";
    if ( argc != 1 ) {
        if ( argc != 3 ) {
            std::cout << "need to pass port as second param" << std::endl;
            return EXIT_FAILURE;
        }
        port = argv[ 2 ];
    }
    ::testing::InitGoogleTest(&argc, argv);
    mongo::Status status = mongo::client::initialize();
    return RUN_ALL_TESTS();
}
