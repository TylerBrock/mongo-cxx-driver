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

using namespace mongo;
using namespace mongo::integration;

namespace {
    TEST_F(ReplicaSetTest, FirstTest) {
        std::string errmsg;

        std::cout << "The uri we got was: " << _uri << std::endl;

        ConnectionString cs = ConnectionString::parse(_uri, errmsg);

        if (!cs.isValid()) {
            std::cout << "errr:" <<  errmsg <<std::endl;
        } else {
            std::cout << "good:" << cs.getServers()[0] << std::endl;
            std::cout << "good2:" << cs.getSetName() << std::endl;
        }

        DBClientReplicaSet * conn = static_cast<DBClientReplicaSet*>(cs.connect(errmsg));
        if (!conn) {
            std::cout << "error connecting: " << errmsg << std::endl;
        }

        std::cout << "inserting" << std::endl;
        conn->insert("test.test", BSON("a" << 1));
        std::cout << "finding" << std::endl;
        std::cout << conn->findOne("test.test", Query()) << std::endl;

    }
} // namespace
