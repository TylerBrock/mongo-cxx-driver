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

            ConnectionString cs = ConnectionString::parse(ReplicaSet().uri(), errmsg);
            replset_conn = static_cast<DBClientReplicaSet*>(cs.connect(errmsg));
            if (!replset_conn) {
                std::cout << "error connecting: " << errmsg << std::endl;
            } else {
                replset_conn->dropCollection(TEST_NS);
            }

            primary_conn = new DBClientConnection();
            primary_conn->connect(ReplicaSet().primary().uri());
            secondary_conn = new DBClientConnection();
            secondary_conn->connect(ReplicaSet().secondaries().front().uri());
        }

        ~ReadPreferenceTest() {
            delete replset_conn;
            delete primary_conn;
            delete secondary_conn;
        }

        DBClientReplicaSet* replset_conn;
        DBClientConnection* primary_conn;
        DBClientConnection* secondary_conn;
    };

    int op_count(DBClientConnection* connection, std::string op_type) {
        BSONObj cmd = BSON("serverStatus" << 1);
        BSONObj info;
        connection->runCommand("admin", cmd, info);
        return info["opcounters"][op_type].Int();
    }

    void assert_route(
        DBClientReplicaSet* test_conn,
        DBClientConnection* expected_target,
        void (*op)(DBClientReplicaSet*, ReadPreference),
        ReadPreference rp,
        std::string op_type)
    {
        // Record operations of this type before running operation
        int ops_before = op_count(expected_target, op_type);

        // Run the Operation
        op(test_conn, rp);

        // Record operations of this type after running operation
        int ops_after = op_count(expected_target, op_type);

        // The serverStatus command itself adds a command to the count
        ASSERT_EQUALS(ops_after - ops_before, op_type == "command" ? 2 : 1);
    }

    void query(DBClientReplicaSet* test_conn, ReadPreference rp) {
        Query q = Query().readPref(rp, BSONArray());
        test_conn->findOne(TEST_NS, q);
    }

    void count(DBClientReplicaSet* test_conn, ReadPreference rp) {
        Query q = Query().readPref(rp, BSONArray());
        test_conn->count(TEST_NS, q);
    }

    void distinct(DBClientReplicaSet* test_conn, ReadPreference rp) {
        Query q = Query().readPref(rp, BSONArray());
        test_conn->distinct(TEST_NS, "a", q);
    }

    TEST_F(ReadPreferenceTest, Routing) {
        assert_route(replset_conn, primary_conn, query, ReadPreference_PrimaryOnly, "query");
        assert_route(replset_conn, primary_conn, query, ReadPreference_PrimaryPreferred, "query");
        assert_route(replset_conn, secondary_conn, query, ReadPreference_SecondaryOnly, "query");
        assert_route(replset_conn, secondary_conn, query, ReadPreference_SecondaryPreferred, "query");

        assert_route(replset_conn, primary_conn, count, ReadPreference_PrimaryOnly, "command");
        assert_route(replset_conn, primary_conn, count, ReadPreference_PrimaryPreferred, "command");
        assert_route(replset_conn, secondary_conn, count, ReadPreference_SecondaryOnly, "command");
        assert_route(replset_conn, secondary_conn, count, ReadPreference_SecondaryPreferred, "command");

        assert_route(replset_conn, primary_conn, distinct, ReadPreference_PrimaryOnly, "command");
        assert_route(replset_conn, primary_conn, distinct, ReadPreference_PrimaryPreferred, "command");
        assert_route(replset_conn, secondary_conn, distinct, ReadPreference_SecondaryOnly, "command");
        assert_route(replset_conn, secondary_conn, distinct, ReadPreference_SecondaryPreferred, "command");

        Server primary = ReplicaSet().primary();
        primary.stop();

        while (true) {
            try {
                replset_conn->findOne(TEST_NS, Query().readPref(ReadPreference_SecondaryOnly, BSONArray()));
                break;
            } catch (DBException& ex) {
                std::cout << ex.what() <<std::endl;
                mongo::sleepsecs(1);
            }
        }

        assert_route(replset_conn, secondary_conn, query, ReadPreference_PrimaryPreferred, "query");
        assert_route(replset_conn, secondary_conn, query, ReadPreference_SecondaryOnly, "query");
        assert_route(replset_conn, secondary_conn, query, ReadPreference_SecondaryPreferred, "query");

        primary.start();

        while (true) {
            try {
                WriteConcern wcAll = WriteConcern().nodes(2);
                replset_conn->insert(TEST_NS, BSON("x" << 2), 0, &wcAll);
                break;
            } catch (DBException& ex) {
                mongo::sleepsecs(1);
            }
        }
    }

    TEST_F(ReadPreferenceTest, QueryPrimaryOnly) {
        Query q = Query().readPref(ReadPreference_PrimaryOnly, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = replset_conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() == replset_conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, QueryPrimaryPreferred) {
        Query q = Query().readPref(ReadPreference_PrimaryPreferred, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = replset_conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() == replset_conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, QuerySecondaryOnly) {
        Query q = Query().readPref(ReadPreference_SecondaryOnly, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = replset_conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() != replset_conn->masterConn().getServerAddress());
    }

    TEST_F(ReadPreferenceTest, QuerySecondaryPreferred) {
        Query q = Query().readPref(ReadPreference_SecondaryPreferred, BSONArray());
        auto_ptr<DBClientCursor> cursor_ptr = replset_conn->query(TEST_NS, q);
        ASSERT_TRUE(cursor_ptr->originalHost() != replset_conn->masterConn().getServerAddress());
    }

} // namespace
