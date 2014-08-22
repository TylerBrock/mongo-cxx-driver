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

#pragma once

#include <string>

#include "mongo/orchestration/orchestration.h"
#include "mongo/unittest/unittest.h"

// Act like we are using the driver externally
#ifdef MONGO_EXPOSE_MACROS
#undef MONGO_EXPOSE_MACROS
#endif

namespace mongo {
    namespace integration {

        class Environment : public ::testing::Environment {
        public:
            Environment(std::string uri) : _uri(uri) {}
            virtual ~Environment() {}
            virtual void SetUp() { _api = new mongo::orchestration::API(_uri); }
            virtual void TearDown() { delete _api; }
            static mongo::orchestration::API* Orchestration() { return _api; }
        private:
            static mongo::orchestration::API* _api;
            std::string _uri;
        };

        class Standalone : public ::testing::Test {
        public:
            static void SetUpTestCase() {
                Environment::Orchestration()->createMongod("standalone");
                _uri = Environment::Orchestration()->host("standalone").uri();
            }

            static void TearDownTestCase() {
                Environment::Orchestration()->host("standalone").destroy();
            }

            static std::string _uri;
        };

    } // namespace integration
} // namespace mongo
