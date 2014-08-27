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

#include "mongo/platform/basic.h"

#include "mongo/integration/integration_test.h"

#include "mongo/client/dbclient.h"

namespace {

    using namespace std;
    using namespace mongo;
    using namespace mongo::integration;

    const string TEST_NS = "test.basic";
    const string TEST_DB = "test";
    const string TEST_COLL = "basic";

    class RSBasicTest : public ReplicaSetTest {
    public:
        RSBasicTest() {
            std::string errmsg;

            ConnectionString cs = ConnectionString::parse(_uri, errmsg);
            conn = static_cast<DBClientReplicaSet*>(cs.connect(errmsg));
            if (!conn) {
                std::cout << "error connecting: " << errmsg << std::endl;
            } else {
                conn->dropCollection(TEST_NS);
            }
        }

        ~RSBasicTest() {
            delete conn;
        }

        DBClientReplicaSet* conn;
    };

    TEST_F(RSBasicTest, InsertRecoversFromPrimaryFailure) {
        WriteConcern wcAll = WriteConcern().nodes(3);
        std::cout << "doing first insert" << std::endl;
        conn->insert("test.test", BSON("x" << 1), 0, &wcAll);
        Environment::Orchestration()->replica_set(_id).primary().stop();
        std::cout << "doing second insert" << std::endl;
        while (true) {
            try {
                conn->insert("test.test", BSON("x" << 1), 0, &WriteConcern::acknowledged);
                break;
            } catch (std::exception& ex) {
                std::cout << "caught exception, sleeping" << std::endl;
                sleep(1);
            }
        }
        std::cout << "second insert succeeded" << std::endl;
    }
}
