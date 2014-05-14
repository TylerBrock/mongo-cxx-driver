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

#include "mongo/unittest/integration_test.h"

#include "mongo/client/command_writer.h"
#include "mongo/client/delete_write_operation.h"
#include "mongo/client/insert_write_operation.h"
#include "mongo/client/update_write_operation.h"
#include "mongo/client/wire_protocol_writer.h"

using std::string;
using std::vector;

using namespace mongo;
using namespace mongo::unittest;

namespace {
    const string TEST_NS = "test.dbclient_writer";

    template <typename T>
    class DBClientWriterTest : public ::testing::Test {
    public:
        DBClientWriterTest() {
            c.connect(string("localhost:") + integrationTestParams.port);
            c.dropCollection(TEST_NS);
            writer = new T(&c);
        }
        ~DBClientWriterTest() {
            delete writer;
        }
        DBClientWriter* writer;
        DBClientConnection c;
    };

    typedef ::testing::Types<WireProtocolWriter, CommandWriter> DBClientWriters;
    TYPED_TEST_CASE(DBClientWriterTest, DBClientWriters);

    TYPED_TEST(DBClientWriterTest, SingleInsert) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert(BSON("a" << 1));
        inserts.push_back(&insert);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, inserts, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.findOne(TEST_NS, Query())["a"].numberInt(), 1);
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedInserts) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert1(BSON("a" << 1));
        InsertWriteOperation insert2(BSON("a" << 2));
        inserts.push_back(&insert1);
        inserts.push_back(&insert2);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, inserts, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_TRUE(results.front()["ok"].trueValue());
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedInserts) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert1(BSON("a" << 1));
        InsertWriteOperation insert2(BSON("a" << 2));
        inserts.push_back(&insert1);
        inserts.push_back(&insert2);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, inserts, false, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_TRUE(results.front()["ok"].trueValue());
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedInsertsWithError) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert1(BSON("_id" << 1));
        InsertWriteOperation insert2(BSON("_id" << 2));
        inserts.push_back(&insert1);
        inserts.push_back(&insert1);
        inserts.push_back(&insert2);
        vector<BSONObj> results;
        ASSERT_THROWS(
            this->writer->write(TEST_NS, inserts, true, &WriteConcern::acknowledged, &results),
            OperationException
        );
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 2}").obj), 0U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedInsertsWithError) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert1(BSON("_id" << 1));
        InsertWriteOperation insert2(BSON("_id" << 2));
        inserts.push_back(&insert1);
        inserts.push_back(&insert1);
        inserts.push_back(&insert2);
        vector<BSONObj> results;
        ASSERT_THROWS(
            this->writer->write(TEST_NS, inserts, false, &WriteConcern::acknowledged, &results),
            OperationException
        );
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 2}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedInsertsWithErrorNoConcern) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert1(BSON("_id" << 1));
        InsertWriteOperation insert2(BSON("_id" << 2));
        inserts.push_back(&insert1);
        inserts.push_back(&insert1);
        inserts.push_back(&insert2);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, inserts, true, &WriteConcern::unacknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 2}").obj), 0U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedInsertsWithErrorNoConcern) {
        vector<WriteOperation*> inserts;
        InsertWriteOperation insert1(BSON("_id" << 1));
        InsertWriteOperation insert2(BSON("_id" << 2));
        inserts.push_back(&insert1);
        inserts.push_back(&insert1);
        inserts.push_back(&insert2);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, inserts, false, &WriteConcern::unacknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{_id: 2}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, SingleUpdate) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("a" << 2));

        vector<WriteOperation*> updates;
        UpdateWriteOperation update(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        updates.push_back(&update);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 2U);
    }

    TYPED_TEST(DBClientWriterTest, SingleUpsertDoesUpdate) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("a" << 2));

        vector<WriteOperation*> updates;
        UpdateWriteOperation update(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), UpdateOption_Upsert);
        updates.push_back(&update);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 0U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 2U);
    }

    TYPED_TEST(DBClientWriterTest, SingleUpsertDoesUpsert) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("a" << 2));

        vector<WriteOperation*> updates;
        UpdateWriteOperation update(BSON("a" << 3), BSON("$set" << BSON("a" << 2)), UpdateOption_Upsert);
        updates.push_back(&update);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 2U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedUpdates) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> updates;
        UpdateWriteOperation updateA(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        UpdateWriteOperation updateB(BSON("b" << 1), BSON("$set" << BSON("b" << 2)), 0);
        updates.push_back(&updateA);
        updates.push_back(&updateB);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 2}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedUpdates) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> updates;
        UpdateWriteOperation updateA(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        UpdateWriteOperation updateB(BSON("b" << 1), BSON("$set" << BSON("b" << 2)), 0);
        updates.push_back(&updateA);
        updates.push_back(&updateB);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, false, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 2}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedUpdatesWithError) {
        this->c.ensureIndex(TEST_NS, BSON("a" << 1), true);
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> updates;
        UpdateWriteOperation updateA(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        UpdateWriteOperation updateB(BSON("b" << 1), BSON("$set" << BSON("a" << 2)), 0);
        updates.push_back(&updateA);
        updates.push_back(&updateB);
        vector<BSONObj> results;
        ASSERT_THROWS(
            this->writer->write(TEST_NS, updates, true, &WriteConcern::acknowledged, &results),
            OperationException
        );
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedUpdatesWithError) {
        this->c.ensureIndex(TEST_NS, BSON("a" << 1), true);
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> updates;
        UpdateWriteOperation updateA(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        UpdateWriteOperation updateB(BSON("b" << 1), BSON("$set" << BSON("a" << 2)), 0);
        updates.push_back(&updateA);
        updates.push_back(&updateB);
        vector<BSONObj> results;
        ASSERT_THROWS(
            this->writer->write(TEST_NS, updates, false, &WriteConcern::acknowledged, &results),
            OperationException
        );
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedUpdatesWithErrorNoConcern) {
        this->c.ensureIndex(TEST_NS, BSON("a" << 1), true);
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> updates;
        UpdateWriteOperation updateA(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        UpdateWriteOperation updateB(BSON("b" << 1), BSON("$set" << BSON("a" << 2)), 0);
        updates.push_back(&updateA);
        updates.push_back(&updateB);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, true, &WriteConcern::unacknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedUpdatesWithErrorNoConcern) {
        this->c.ensureIndex(TEST_NS, BSON("a" << 1), true);
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> updates;
        UpdateWriteOperation updateA(BSON("a" << 1), BSON("$set" << BSON("a" << 2)), 0);
        UpdateWriteOperation updateB(BSON("b" << 1), BSON("$set" << BSON("a" << 2)), 0);
        updates.push_back(&updateA);
        updates.push_back(&updateB);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, updates, false, &WriteConcern::unacknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, SingleDelete) {
        this->c.insert(TEST_NS, BSON("a" << 0));
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("a" << 2));

        vector<WriteOperation*> deletes;
        DeleteWriteOperation delete_op(BSON("a" << BSON("$gt" << 0)), 0);
        deletes.push_back(&delete_op);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, deletes, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 0}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 0U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 0U);
    }

    TYPED_TEST(DBClientWriterTest, SingleDeleteJustOne) {
        this->c.insert(TEST_NS, BSON("a" << 0));
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("a" << 2));

        vector<WriteOperation*> deletes;
        DeleteWriteOperation delete_op(BSON("a" << BSON("$gt" << 0)), RemoveOption_JustOne);
        deletes.push_back(&delete_op);
        vector<BSONObj> results;
        this->writer->write(TEST_NS, deletes, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 0}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 0U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 2}").obj), 1U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleOrderedDeletes) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> deletes;
        DeleteWriteOperation deleteA(BSON("a" << 1), 0);
        DeleteWriteOperation deleteB(BSON("b" << 1), 0);
        deletes.push_back(&deleteA);
        deletes.push_back(&deleteB);
        vector<BSONObj> results;
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 1U);
        this->writer->write(TEST_NS, deletes, true, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 0U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 0U);
    }

    TYPED_TEST(DBClientWriterTest, MultipleUnorderedDeletes) {
        this->c.insert(TEST_NS, BSON("a" << 1));
        this->c.insert(TEST_NS, BSON("b" << 1));

        vector<WriteOperation*> deletes;
        DeleteWriteOperation deleteA(BSON("a" << 1), 0);
        DeleteWriteOperation deleteB(BSON("b" << 1), 0);
        deletes.push_back(&deleteA);
        deletes.push_back(&deleteB);
        vector<BSONObj> results;
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 1U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 1U);
        this->writer->write(TEST_NS, deletes, false, &WriteConcern::acknowledged, &results);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{a: 1}").obj), 0U);
        ASSERT_EQUALS(this->c.count(TEST_NS, Query("{b: 1}").obj), 0U);
    }
}
