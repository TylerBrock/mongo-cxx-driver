/*    Copyright 2014 MongoDB Inthis->c->
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
#include "mongo/client/dbclient.h"
#include "mongo/client/write_result.h"
#include "mongo/client/wire_protocol_writer.h"

namespace {

    using std::string;
    using std::vector;

    using namespace mongo;
    using namespace mongo::unittest;

    const string TEST_DB = "test";
    const string TEST_COLL = "bulk_operation";
    const string TEST_NS = "test.bulk_operation";

    template <typename T>
    struct RequiredWireVersion;

    template <>
    struct RequiredWireVersion<WireProtocolWriter> {
        static const int value = 0;
    };

    template <>
    struct RequiredWireVersion<CommandWriter> {
        static const int value = 2;
    };

    template <typename T>
    class BulkOperationTest : public ::testing::Test {
    public:
        BulkOperationTest() {
            c = new DBClientConnection;
            c->connect(string("localhost:") + integrationTestParams.port);
            c->dropCollection(TEST_NS);
            c->setWireVersions(RequiredWireVersion<T>::value, RequiredWireVersion<T>::value);
        }

        ~BulkOperationTest() { delete c; }

        DBClientConnection* c;
    };

    typedef ::testing::Types<WireProtocolWriter, CommandWriter> DBClientWriters;
    TYPED_TEST_CASE(BulkOperationTest, DBClientWriters);

    TYPED_TEST(BulkOperationTest, InsertOrdered) {
        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.insert(BSON("a" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 1);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        BSONObj doc = this->c->findOne(TEST_NS, Query("{}").obj);
        ASSERT_EQUALS(doc["a"].numberInt(), 1);
    }


    TYPED_TEST(BulkOperationTest, InsertUnordered) {
        BulkOperationBuilder bulk(this->c, TEST_NS, false);
        bulk.insert(BSON("a" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 1);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        BSONObj doc = this->c->findOne(TEST_NS, Query("{}").obj);
        ASSERT_EQUALS(doc["a"].numberInt(), 1);
    }

    TYPED_TEST(BulkOperationTest, InsertBadKeyOrdered) {
        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.insert(BSON("$a" << 1));

        WriteResult result;
        ASSERT_THROW(
            bulk.execute(&WriteConcern::acknowledged, &result),
            OperationException
        );
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 0U);
    }

    TYPED_TEST(BulkOperationTest, InsertBadKeyUnordered) {
        BulkOperationBuilder bulk(this->c, TEST_NS, false);
        bulk.insert(BSON("$a" << 1));

        WriteResult result;
        ASSERT_THROW(
            bulk.execute(&WriteConcern::acknowledged, &result),
            OperationException
        );
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 0U);
    }

    TYPED_TEST(BulkOperationTest, UpdateOneMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).updateOne(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 1);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 1);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
    }

    TYPED_TEST(BulkOperationTest, UpdateMultiMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).update(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 2);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 2);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{a: 1, x: 1}").obj), 2U);
    }

    TYPED_TEST(BulkOperationTest, UpdateAllDocuments) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(fromjson("{}")).update(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 3);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 3);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());


        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, ReplaceEntireDocument) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).replaceOne(BSON("x" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 1);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 1);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());


        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
        ASSERT_FALSE(this->c->findOne(TEST_NS, Query("{x: 1}").obj).hasField("a"));
    }

    TYPED_TEST(BulkOperationTest, UpsertOneMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).upsert().updateOne(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 1);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 1);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, UpsertOneNotMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 2)).upsert().updateOne(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_EQUALS(result.upserted().size(), 1U);
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
        ASSERT_TRUE(this->c->findOne(TEST_NS, Query("{x: 1}").obj).hasField("a"));
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, UpsertMultiMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).upsert().update(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 2);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 2);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 2U);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, UpsertMultiNotMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 2)).upsert().update(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_EQUALS(result.upserted().size(), 1U);
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
        ASSERT_TRUE(this->c->findOne(TEST_NS, Query("{x: 1}").obj).hasField("a"));
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, MultipleUpsertsMixedBatchHaveCorrectSequence) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 2)).upsert().update(BSON("$inc" << BSON("x" << 1)));
        bulk.find(BSON("a" << 3)).upsert().update(BSON("$inc" << BSON("x" << 1)));
        bulk.insert(BSON("a" << 4));
        bulk.find(BSON("a" << 5)).upsert().update(BSON("$inc" << BSON("x" << 1)));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);
        ASSERT_EQUALS(result.upserted()[0].getIntField("index"), 0);
        ASSERT_EQUALS(result.upserted()[1].getIntField("index"), 1);
        ASSERT_EQUALS(result.upserted()[2].getIntField("index"), 3);
    }

    TYPED_TEST(BulkOperationTest, UpsertReplaceMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).upsert().replaceOne(BSON("x" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 1);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 1);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{a: 1}").obj), 1U);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, UpsertReplaceNotMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 2)).upsert().replaceOne(BSON("x" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_EQUALS(result.upserted().size(), 1U);
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{x: 1}").obj), 1U);
        ASSERT_FALSE(this->c->findOne(TEST_NS, Query("{x: 1}").obj).hasField("a"));
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 3U);
    }

    TYPED_TEST(BulkOperationTest, RemoveOneMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).removeOne();

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 1);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 2U);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{a: 1}").obj), 1U);
    }

    TYPED_TEST(BulkOperationTest, RemoveAllMatchingSelector) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(BSON("a" << 1)).remove();

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 2);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 1U);
        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{a: 1}").obj), 0U);
    }

    TYPED_TEST(BulkOperationTest, RemoveAll) {
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("a" << 1));
        this->c->insert(TEST_NS, BSON("b" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.find(fromjson("{}")).remove();

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);
        ASSERT_TRUE(result.upserted().empty());

        ASSERT_EQUALS(result.nInserted(), 0);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 3);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());

        ASSERT_EQUALS(this->c->count(TEST_NS, Query("{}").obj), 0U);
    }

    TYPED_TEST(BulkOperationTest, MultipleOrderedOperations) {
        this->c->insert(TEST_NS, BSON("c" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, true);
        bulk.insert(BSON("a" << 1));
        bulk.insert(BSON("a" << 1));
        bulk.insert(BSON("b" << 1));
        bulk.find(BSON("a" << 1)).updateOne(BSON("$set" << BSON("a" << 2)));
        bulk.find(BSON("c" << 1)).remove();
        bulk.find(BSON("_id" << 1)).upsert().update(BSON("$set" << BSON("b" << 1)));
        bulk.insert(BSON("b" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), 4);
        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(result.nMatched(), 1);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 1);
        ASSERT_EQUALS(result.nRemoved(), 1);
        ASSERT_EQUALS(result.upserted().size(), 1U);
        ASSERT_EQUALS(result.upserted().front().getIntField("index"), 5);
        ASSERT_EQUALS(result.upserted().front().getIntField("_id"), 1);
        ASSERT_FALSE(result.hasErrors());
    }

    TYPED_TEST(BulkOperationTest, MultipleUnorderedOperations) {
        this->c->insert(TEST_NS, BSON("c" << 1));

        BulkOperationBuilder bulk(this->c, TEST_NS, false);
        bulk.insert(BSON("a" << 1));
        bulk.insert(BSON("a" << 1));
        bulk.insert(BSON("b" << 1));
        bulk.find(BSON("a" << 1)).updateOne(BSON("$set" << BSON("a" << 2)));
        bulk.find(BSON("c" << 1)).remove();
        bulk.find(BSON("_id" << 1)).upsert().update(BSON("$set" << BSON("b" << 1)));
        bulk.insert(BSON("b" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        // this test passing depends on the current unordered optimization implementation
        ASSERT_EQUALS(result.nInserted(), 4);
        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(result.nMatched(), 1);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 1);
        ASSERT_EQUALS(result.nRemoved(), 1);
        ASSERT_EQUALS(result.upserted().size(), 1U);
        ASSERT_EQUALS(result.upserted().front().getIntField("index"), 5);
        ASSERT_EQUALS(result.upserted().front().getIntField("_id"), 1);
        ASSERT_FALSE(result.hasErrors());
    }

    TYPED_TEST(BulkOperationTest, ExceedBatchSize) {
        BulkOperationBuilder bulk(this->c, TEST_NS, false);
        for (int i=0; i < this->c->getMaxWriteBatchSize() + 1; ++i)
            bulk.insert(BSON("a" << 1));

        WriteResult result;
        bulk.execute(&WriteConcern::acknowledged, &result);

        ASSERT_EQUALS(result.nInserted(), this->c->getMaxWriteBatchSize() + 1);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_TRUE(result.upserted().empty());
        ASSERT_FALSE(result.hasErrors());
    }

    TYPED_TEST(BulkOperationTest, UnorderedBatchWithErrors) {
        this->c->ensureIndex(TEST_NS, BSON("a" << 1), true);

        BulkOperationBuilder bulk(this->c, TEST_NS, false);

        bulk.insert(BSON("b" << 1 << "a" << 1));

        // two of these upserts fail
        bulk.find(BSON("b" << 2)).upsert().update(BSON("$set" << BSON("a" << 1)));
        bulk.find(BSON("b" << 3)).upsert().update(BSON("$set" << BSON("a" << 2)));
        bulk.find(BSON("b" << 2)).upsert().update(BSON("$set" << BSON("a" << 1)));
        bulk.insert(BSON("b" << 4 << "a" << 3));

        // this and / or the first insert fails:
        bulk.insert(BSON("b" << 5 << "a" << 1));

        WriteResult result;
        ASSERT_THROWS(
            bulk.execute(&WriteConcern::acknowledged, &result),
            OperationException
        );

        ASSERT_EQUALS(result.nInserted(), 2);
        ASSERT_EQUALS(result.nUpserted(), 1);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_EQUALS(result.upserted().size(), 1U);
        ASSERT_EQUALS(result.upserted().front().getIntField("index"), 2);
        ASSERT_EQUALS(result.upserted().front().getField("_id").type(), 7);
        ASSERT_EQUALS(result.writeErrors().size(), 3U);

        BSONObj distinct_result;
        this->c->runCommand(TEST_DB, BSON("distinct" << TEST_COLL << "key" << "a"), distinct_result);
        std::vector<BSONElement> distinct_a = distinct_result.getField("values").Array();
        ASSERT_EQUALS(distinct_a[0].Int(), 1);
        ASSERT_EQUALS(distinct_a[1].Int(), 2);
        ASSERT_EQUALS(distinct_a[2].Int(), 3);
    }

    TYPED_TEST(BulkOperationTest, OrderedBatchWithErrors) {
        this->c->ensureIndex(TEST_NS, BSON("a" << 1), true);

        BulkOperationBuilder bulk(this->c, TEST_NS, true);

        bulk.insert(BSON("b" << 1 << "a" << 1));

        // one or two of these upserts fails
        bulk.find(BSON("b" << 2)).upsert().update(BSON("$set" << BSON("a" << 1)));
        bulk.find(BSON("b" << 3)).upsert().update(BSON("$set" << BSON("a" << 2)));
        bulk.find(BSON("b" << 2)).upsert().update(BSON("$set" << BSON("a" << 1)));
        bulk.insert(BSON("b" << 4 << "a" << 3));

        // this and / or the first insert fails:
        bulk.insert(BSON("b" << 5 << "a" << 1));

        WriteResult result;
        ASSERT_THROWS(
            bulk.execute(&WriteConcern::acknowledged, &result),
            OperationException
        );

        ASSERT_EQUALS(result.nInserted(), 1);
        ASSERT_EQUALS(result.nUpserted(), 0);
        ASSERT_EQUALS(result.nMatched(), 0);
        if (result.hasModifiedCount())
            ASSERT_EQUALS(result.nModified(), 0);
        ASSERT_EQUALS(result.nRemoved(), 0);
        ASSERT_EQUALS(result.upserted().size(), 0U);
        ASSERT_EQUALS(result.writeErrors().size(), 1U);

        BSONObj writeError = result.writeErrors().front();
        ASSERT_EQUALS(writeError.getIntField("code"), 11000);
        ASSERT_EQUALS(writeError.getIntField("index"), 1);
        ASSERT_TRUE(writeError.hasField("errmsg"));

        // { q: { b: 2 }, u: { $set: { a: 1 } }, multi: true, upsert: true }
        BSONObj op = writeError.getObjectField("op");
        ASSERT_EQUALS(op.getFieldDotted("q.b").Int(), 2);
        ASSERT_EQUALS(op.getFieldDotted("u.$set.a").Int(), 1);
        ASSERT_TRUE(op.getBoolField("multi"));
        ASSERT_TRUE(op.getBoolField("upsert"));
    }

} // namespace
