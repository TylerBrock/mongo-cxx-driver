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

    const string TEST_NS = "test.read_pref";
    const string TEST_DB = "test";
    const string TEST_COLL = "read_pref";

    class ReadPreferenceTest : public ReplicaSetTest {
    public:
        ReadPreferenceTest() {
            std::string errmsg;

            ConnectionString cs = ConnectionString::parse(_uri, errmsg);
            conn = static_cast<DBClientReplicaSet*>(cs.connect(errmsg));
            if (!conn) {
                std::cout << "error connecting: " << errmsg << std::endl;
            } else {
                conn->dropCollection(TEST_NS);
            }
        }

        ~ReadPreferenceTest() {
            delete conn;
        }

        DBClientReplicaSet* conn;
    };

    TEST_F(ReadPreferenceTest, FirstTest) {
        WriteConcern wc = WriteConcern().nodes(3);
        conn->insert(TEST_NS, BSON("a" << 1), 0, &wc);
    }

    TEST_F(ReadPreferenceTest, QueryPrimaryOnly) {
        Query q = Query().readPref(ReadPreference_PrimaryOnly, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() == conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, QueryPrimaryPreferred) {
        Query q = Query().readPref(ReadPreference_PrimaryPreferred, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() == conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, QuerySecondaryOnly) {
        Query q = Query().readPref(ReadPreference_SecondaryOnly, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() != conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, QuerySecondaryPreferred) {
        Query q = Query().readPref(ReadPreference_SecondaryPreferred, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() != conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, CountPrimaryOnly) {
        WriteConcern wc = WriteConcern().nodes(3);
        conn->insert(TEST_NS, BSON("a" << 1), 0, &wc);

        Query q = Query().readPref(ReadPreference_PrimaryOnly, BSONArray());
        ASSERT_EQUALS(conn->count(TEST_NS, q), 1U);
    }

    TEST_F(ReadPreferenceTest, CountPrimaryPreferred) {
        WriteConcern wc = WriteConcern().nodes(3);
        conn->insert(TEST_NS, BSON("a" << 1), 0, &wc);

        Query q = Query().readPref(ReadPreference_PrimaryPreferred, BSONArray());
        ASSERT_EQUALS(conn->count(TEST_NS, q), 1U);
    }

    TEST_F(ReadPreferenceTest, CountSecondaryOnly) {
        WriteConcern wc = WriteConcern().nodes(3);
        conn->insert(TEST_NS, BSON("a" << 1), 0, &wc);

        Query q = Query().readPref(ReadPreference_SecondaryOnly, BSONArray());
        ASSERT_EQUALS(conn->count(TEST_NS, q), 1U);
    }

    TEST_F(ReadPreferenceTest, CountSecondaryPreferred) {
        WriteConcern wc = WriteConcern().nodes(3);
        conn->insert(TEST_NS, BSON("a" << 1), 0, &wc);

        Query q = Query().readPref(ReadPreference_SecondaryPreferred, BSONArray());
        ASSERT_EQUALS(conn->count(TEST_NS, q), 1U);
    }

} // namespace
