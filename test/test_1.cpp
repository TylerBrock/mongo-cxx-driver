#include <iostream>
#include <memory>
#include <cstring>

#include "bson/types.hpp"
#include "bson/builder.hpp"

int main() {
    bson::builder builder;

    builder << "hello" << "world" << "bar" << 10;

    bson::document::view doc(builder.view());

    std::cout << "Doc: " << doc << std::endl;

    for (auto&& x : doc) {
        std::cout << "type is: " << x.type() << std::endl;
        std::cout << "value is: ";

        if (x.type() == bson::type::k_int32) {
            std::cout << x.get_int32();
        } else {
            std::cout << x.get_string();
        }

        std::cout << std::endl;
    }

    std::cout << "key: " << doc["hello"].key() << std::endl;

    return 0;
}
