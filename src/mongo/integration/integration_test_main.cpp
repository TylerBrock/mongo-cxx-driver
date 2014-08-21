/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <iostream>

#include "mongo/integration/integration_test.h"
#include "mongo/client/init.h"

namespace mongo {
    namespace integration {
        mongo::orchestration::API* orchestration_api = NULL;
        std::string Standalone::_uri = "";
    } // namespace integration
} // namespace mongo

int main(int argc, char **argv) {
    mongo::Status status = mongo::client::initialize();

    if (!status.isOK())
        ::abort();

    mongo::integration::orchestration_api = new mongo::orchestration::API("localhost:8889");

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
