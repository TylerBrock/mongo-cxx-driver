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
#include <map>
#include <memory>

#include "mongo/orchestration/orchestration.h"
#include "mongo/unittest/unittest.h"

// Act like we are using the driver externally
#ifdef MONGO_EXPOSE_MACROS
#undef MONGO_EXPOSE_MACROS
#endif

namespace mongo {
namespace integration {

    using mongo::orchestration::Service;
    using mongo::orchestration::Server;
    using mongo::orchestration::ReplicaSet;
    using mongo::orchestration::ShardedCluster;

    //
    // MongoDB Integration Test Environment
    //
    // Augments the testing environment such that the orchestration service is available.
    //
    // This Environment is available globally in tests and test fixtures that use the provided
    // integration_test_main.cpp.
    //
    // Google test takes ownership and is responsible for destructing new heap allocated
    // instances of this object via AddGlobalTestEnvironment.
    //
    class Environment : public ::testing::Environment {
    public:
        Environment(std::string uri) {
            _orchestration.reset(new Service(uri));
        }

        static const std::auto_ptr<Service>& Orchestration() {
            return _orchestration;
        }

    private:
        static std::auto_ptr<mongo::orchestration::Service> _orchestration;
    };

    //
    // MongoDB Integration Test Fixture for single server ("standalone") tests
    //
    // Creates a Mongod instance which is unique per test-case and destroys it upon test
    // termination.
    //
    class StandaloneTest : public ::testing::Test {
    public:
        Server server() {
            return Environment::Orchestration()->server(_id);
        }

        static void SetUpTestCase() {
            _id = Environment::Orchestration()->createMongod();
        }

        static void TearDownTestCase() {
            Environment::Orchestration()->server(_id).destroy();
        }

    private:
        static std::string _id;
    };

    //
    // MongoDB Integration Test Fixture for replica set tests
    //
    // Creates a Replica Set which is unique per test-case and destroys it upon test
    // termination. Uses the arbiter preset which creates a 3 node set with 2 data bearing
    // members and an arbiter.
    //
    class ReplicaSetTest : public ::testing::Test {
    public:
        ReplicaSet rs() {
            return Environment::Orchestration()->replica_set(_id);
        }

        static void SetUpTestCase() {
            Json::Value parameters;
            parameters["preset"] = "arbiter.json";
            _id = Environment::Orchestration()->createReplicaSet(parameters);
        }

        static void TearDownTestCase() {
            Environment::Orchestration()->replica_set(_id).destroy();
        }

    private:
        static std::string _id;
    };

} // namespace integration
} // namespace mongo
