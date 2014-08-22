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
#include <memory>

#include "mongo/integration/integration_test.h"
#include "mongo/client/init.h"

namespace mongo {
    namespace integration {
        mongo::orchestration::API* Environment::_api = NULL;
        std::string mongo::integration::Standalone::_uri;
    } // namespace integration
} // namespace mongo

int main(int argc, char **argv) {

    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " MONGO_ORCHESTRATION_URL" << std::endl;
    }

    mongo::Status status = mongo::client::initialize();

    if (!status.isOK())
        ::abort();

    ::testing::AddGlobalTestEnvironment(new mongo::integration::Environment(argv[1]));
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
