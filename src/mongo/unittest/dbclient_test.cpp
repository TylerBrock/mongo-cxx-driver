#include <boost/scoped_ptr.hpp>
#include <string>

#include "mongo/unittest/integration_test.h"

#include "mongo/bson/bson.h"
#include "mongo/client/dbclient.h"

using std::string;
using std::list;
using std::auto_ptr;
using boost::function;
using namespace mongo;
using namespace mongo::unittest;
using namespace bson;

namespace {
    const string TEST_NS = "test.foo";

    class DBClientTest : public ::testing::Test {
    public:
        DBClientTest() {
            c.connect(string("localhost:") + integrationTestParams.port);
            c.dropCollection("test.foo");
        }
        ~DBClientTest() {
        }
        DBClientConnection c;
    };
}

/*
 * From simple_client_demo.cpp
 */
TEST_F(DBClientTest, Count) {
    unsigned long long count = c.count("test.foo");
    ASSERT_EQUALS(count, 0);

    bo o = BSON( "hello" << "world" );
    c.insert(TEST_NS, o);
    string e = c.getLastError();

    ASSERT_TRUE(e.empty());

}

TEST_F(DBClientTest, CreateIndex) {
    // make an index with a unique key constraint
    c.ensureIndex(TEST_NS, BSON("hello"<<1), /*unique*/true);

    bo o = BSON( "hello" << "world" );
    c.insert(TEST_NS, o);
    c.insert(TEST_NS, o); // will cause a dup key error on "hello" field
    ASSERT_FALSE(c.getLastError().empty());
}

/*
 * From insert_demo.cpp
 */
TEST_F(DBClientTest, LargeInsert) {
    bo o = BSON("hello" << "world");

    for(unsigned i = 0; i < 100000; i++) {
        c.insert(TEST_NS, o);
    }

    // wait until all operations applied
    ASSERT_TRUE(c.getLastError().empty());
}

/*
 * From clientTest.cpp
 */
TEST_F(DBClientTest, Insert) {
    c.insert(TEST_NS, BSON("name" << "Tyler"));
    ASSERT_FALSE(c.findOne(TEST_NS, BSONObj()).isEmpty());
}

TEST_F(DBClientTest, Remove) {
    c.insert(TEST_NS, BSON("name" << "Tyler"));
    c.remove(TEST_NS, BSONObj());
    ASSERT_TRUE(c.findOne(TEST_NS, BSONObj()).isEmpty());
}

TEST_F(DBClientTest, FindOne) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 1));
    BSONObj doc = c.findOne(TEST_NS , BSONObj());
    ASSERT_EQUALS(doc.getStringField("name"), string("Tyler"));
    ASSERT_NOT_EQUALS(doc.getStringField("name2"), string("Tyler"));
    ASSERT_EQUALS(doc.getIntField( "num" ), 1);
}

TEST_F(DBClientTest, CursorCountEmptyQuery) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 1));
    c.insert(TEST_NS, BSON("name" << "Jason" << "num" << 2));
    auto_ptr<DBClientCursor> cursor = c.query(TEST_NS, BSONObj());
    int count = 0;
    while ( cursor->more() ) {
        count++;
        BSONObj obj = cursor->next();
    }
    ASSERT_EQUALS(count, 2);
}

TEST_F(DBClientTest, CursorCountMatching) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 1));
    c.insert(TEST_NS, BSON("name" << "Jason" << "num" << 2));
    auto_ptr<DBClientCursor> cursor = c.query(TEST_NS, BSON("num" << 1));
    int count = 0;
    while ( cursor->more() ) {
        count++;
        BSONObj obj = cursor->next();
    }
    ASSERT_EQUALS(count, 1);
}

TEST_F(DBClientTest, CursorCountNotMatching) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 1));
    c.insert(TEST_NS, BSON("name" << "Jason" << "num" << 2));
    auto_ptr<DBClientCursor> cursor = c.query(TEST_NS , BSON("num" << 3));
    int count = 0;
    while ( cursor->more() ) {
        count++;
        BSONObj obj = cursor->next();
    }
    ASSERT_EQUALS(count, 0);
}

TEST_F(DBClientTest, Update) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 1));

    BSONObj doc = c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler").obj());
    ASSERT_NOT_EQUALS(doc.getStringField("name2"), "Tyler");

    BSONObj after = BSONObjBuilder().appendElements(doc).append("name2", "h").obj();
    c.update(TEST_NS, BSONObjBuilder().append("name", "Tyler2").obj(), after);

    doc = c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler").obj());
    ASSERT_NOT_EQUALS(doc.getStringField("name2"), "Tyler");
    ASSERT_TRUE(c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler2").obj()).isEmpty());

    c.update(TEST_NS, BSONObjBuilder().append("name", "Tyler").obj(), after);
    doc = c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler").obj());
    ASSERT_EQUALS(doc.getStringField("name"), string("Tyler"));
    ASSERT_EQUALS(doc.getStringField("name2"), string("h"));
    ASSERT_TRUE(c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler2").obj()).isEmpty());
}

TEST_F(DBClientTest, Upsert) {
    BSONObj doc = c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler").obj());
    BSONObj after = BSONObjBuilder().appendElements(doc).append("name2", "h").obj();
    c.update(TEST_NS, BSONObjBuilder().append("name", "Tyler2").obj(), after, 1);
    ASSERT_TRUE(c.findOne(TEST_NS, BSONObjBuilder().append("name", "Tyler").obj()).isEmpty());
}

TEST_F(DBClientTest, EnsureIndex) {
    ASSERT_TRUE(c.ensureIndex(TEST_NS, BSON("name" << 1)));
    ASSERT_FALSE(c.ensureIndex(TEST_NS, BSON("name" << 1)));
}

TEST_F(DBClientTest, DISABLED_TTLIndex) {
    BSONObjBuilder b;
    b.appendTimeT("ttltime", time(0));
    b.append("name", "foo");
    c.insert(TEST_NS, b.obj());

    c.ensureIndex(TEST_NS, BSON("ttltime" << 1), false, "", true, false, -1, 5);
    ASSERT_FALSE(c.findOne(TEST_NS, BSONObjBuilder().append("name", "foo").obj()).isEmpty());
    // Sleep 66 seconds, 60 seconds for the TTL loop, 5 seconds for the TTL and 1 to ensure
    sleepsecs(66);
    ASSERT_TRUE(c.findOne(TEST_NS, BSONObjBuilder().append("name", "foo").obj()).isEmpty());
}

TEST_F(DBClientTest, HintUsage) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 1));
    c.insert(TEST_NS, BSON("name" << "Jason" << "num" << 2));
    c.ensureIndex(TEST_NS, BSON("name" << 1));

    ASSERT_EQUALS(c.findOne(TEST_NS, "{}")["name"].str(), "Tyler");
    ASSERT_EQUALS(c.findOne(TEST_NS, "{name: 'Jason'}")["name"].str(), "Jason");
    ASSERT_THROWS(
        c.findOne(TEST_NS, Query("{name: 'Jason'}").hint("{foo: 1}"));
    , DBException);
}

TEST_F(DBClientTest, Timestamp) {
    mongo::BSONObjBuilder b;
    b.appendTimestamp("ts");
    c.insert(TEST_NS, b.obj());

    BSONObj out = c.findOne(TEST_NS, mongo::BSONObj());
    Date_t oldTime = out["ts"].timestampTime();
    unsigned int oldInc = out["ts"].timestampInc();

    mongo::BSONObjBuilder b1;
    b1.append(out["_id"]);

    mongo::BSONObjBuilder b2;
    b2.append(out["_id"]);
    b2.appendTimestamp("ts");

    c.update(TEST_NS, b1.obj(), b2.obj());
    BSONObj found = c.findOne(TEST_NS, BSONObj());
    ASSERT_TRUE(
        (oldTime < found["ts"].timestampTime()) ||
        (oldTime == found["ts"].timestampTime() && oldInc < found["ts"].timestampInc())
    );
}

TEST_F(DBClientTest, KillCursors) {
    ASSERT_TRUE(c.getLastError().empty());

    BufBuilder b;
    b.appendNum((int)0); // reserved
    b.appendNum((int)-1); // invalid # of cursors triggers exception
    b.appendNum((int)-1); // bogus cursor id

    Message m;
    m.setData(dbKillCursors, b.buf(), b.len());

    // say() is protected in DBClientConnection, so get superclass
    static_cast<DBConnector*>(&c)->say(m);

    ASSERT_TRUE(c.getLastError().empty());
}

TEST_F(DBClientTest, GetDatabaseNames) {
    list<string> dbs = c.getDatabaseNames();

    for (list<string>::iterator i = dbs.begin(); i != dbs.end(); i++) {
        // TODO: Create a database and check if it's in this list
    }
}

TEST_F(DBClientTest, GetCollectionNames) {
    list<string> cols = c.getCollectionNames("test");

    for (list<string>::iterator i = cols.begin(); i != cols.end(); i++) {
        // TODO: Create a collection and check if it's in this list
    }
}

TEST_F(DBClientTest, MapReduce) {
    c.insert(TEST_NS, BSON("a" << 1));
    c.insert(TEST_NS, BSON("a" << 1));

    const char* map = "function() { emit(this.a, 1); }";
    const char* reduce = "function(key, values) { return Array.sum(values); }";

    const string outcoll = TEST_NS + ".out";

    BSONObj out;
    // TODO: Do something with the output
    out = c.mapreduce(TEST_NS, map, reduce, BSONObj()); // default to inline
    out = c.mapreduce(TEST_NS, map, reduce, BSON("a" << 1));
    out = c.mapreduce(TEST_NS, map, reduce, BSONObj(), outcoll);
    out = c.mapreduce(TEST_NS, map, reduce, BSONObj(), outcoll.c_str());
    out = c.mapreduce(TEST_NS, map, reduce, BSONObj(), BSON("reduce" << outcoll));
}

TEST_F(DBClientTest, Timeout) {
    DBClientConnection conn(true, 0, 1);
    conn.connect(string("127.0.0.1:") + integrationTestParams.port);

    conn.insert(TEST_NS, BSON("x" << 1));
    conn.getLastError();

    BSONObj res;
    string eval_str = "return db.foo.findOne().x";

    ASSERT_TRUE(c.eval("test", eval_str, res));
    ASSERT_THROWS(
        conn.eval( "test" , "sleep(2000); " + eval_str)
    , DBException);
    ASSERT_TRUE(c.eval("test", eval_str, res));
}

/*
 * From whereExample.cpp
 */

TEST_F(DBClientTest, Where) {
    c.insert(TEST_NS, BSON("name" << "Tyler" << "num" << 17));
    c.insert(TEST_NS, BSON("name" << "Jason" << "num" << 24));

    Query q = Query("{}").where("this.name == name", BSON("name" << "Jason"));

    std::auto_ptr<DBClientCursor> cursor;
    cursor = c.query(TEST_NS, q);
    ASSERT_TRUE(cursor.get());

    int num = 0;
    while(cursor->more()) {
        BSONObj obj = cursor->next();
        num++;
    }
    ASSERT_EQUALS(num, 1);
}

/* Added examples */

TEST_F(DBClientTest, ServerAlive) {
    ASSERT_TRUE(serverAlive("localhost:" + integrationTestParams.port));
    ASSERT_FALSE(serverAlive("fakehost"));
}

TEST_F(DBClientTest, ErrField) {
    ASSERT_FALSE(hasErrField(BSONObj()));
    ASSERT_TRUE(hasErrField(BSON("$err" << true)));
}

TEST_F(DBClientTest, Explain) {
    Query q;
    q.explain();
    ASSERT_TRUE(q.isComplex());
    ASSERT_TRUE(q.isExplain());
}

TEST_F(DBClientTest, Snapshot) {
    Query q;
    q.snapshot();
    ASSERT_TRUE(q.isComplex());
    // TODO: figure out how to test this worked
}

TEST_F(DBClientTest, Sort) {
    Query q;
    q.sort(BSON("a" << 1));
    ASSERT_TRUE(q.isComplex());
    BSONObj sort = q.getSort();
    ASSERT_TRUE(sort.hasField("a"));
    ASSERT_EQUALS(sort.getIntField("a"), 1);
}

TEST_F(DBClientTest, Hint) {
    Query q;
    q.hint(BSON("a" << 1));
    BSONObj hint = q.getHint();
    ASSERT_TRUE(hint.hasField("a"));
    ASSERT_EQUALS(hint.getIntField("a"), 1);
}

TEST_F(DBClientTest, MinKey) {
    Query q;
    q.minKey(BSONObj());
    // TODO: figure out how to test this
}

TEST_F(DBClientTest, MaxKey) {
    Query q;
    q.maxKey(BSONObj());
    // TODO: figure out how to test this
}

TEST(ConnectionString, SameLogicalEndpoint) {
    string err1;
    string err2;
    ConnectionString cs1;
    ConnectionString cs2;

    // INVALID -- default non parsed state
    ASSERT_TRUE(cs1.sameLogicalEndpoint(cs2));
    cs2 = ConnectionString::parse("host1,host2,host3", err1);
    ASSERT_TRUE(cs1.sameLogicalEndpoint(cs2));

    // MASTER
    cs1 = ConnectionString::parse("localhost:1234", err1);
    cs2 = ConnectionString::parse("localhost:1234", err2);
    ASSERT_TRUE(cs1.sameLogicalEndpoint(cs2));

    // PAIR -- compares the host + port even in swapped order
    cs1 = cs1.parse("localhost:1234,localhost:5678", err1);
    cs2 = cs2.parse("localhost:1234,localhost:5678", err2);
    ASSERT_TRUE(cs1.sameLogicalEndpoint(cs2));
    cs2 = cs2.parse("localhost:5678,localhost:1234", err2);
    ASSERT_TRUE(cs1.sameLogicalEndpoint(cs2));

    // SET -- compares the set name only
    cs1 = cs1.parse("testset/localhost:1234,localhost:5678", err1);
    cs2 = cs2.parse("testset/localhost:5678,localhost:1234", err2);
    ASSERT_TRUE(cs1.sameLogicalEndpoint(cs2));

    // Different types
    cs1 = cs1.parse("testset/localhost:1234,localhost:5678", err1);
    cs2 = cs2.parse("localhost:5678,localhost:1234", err2);
    ASSERT_FALSE(cs1.sameLogicalEndpoint(cs2));
}

TEST(ConnectionString, TypeToString) {
    ASSERT_EQUALS(
        ConnectionString::typeToString(ConnectionString::INVALID),
        "invalid"
    );
    ASSERT_EQUALS(
        ConnectionString::typeToString(ConnectionString::MASTER),
        "master"
    );
    ASSERT_EQUALS(
        ConnectionString::typeToString(ConnectionString::PAIR),
        "pair"
    );
    ASSERT_EQUALS(
        ConnectionString::typeToString(ConnectionString::SET),
        "set"
    );
    ASSERT_EQUALS(
        ConnectionString::typeToString(ConnectionString::CUSTOM),
        "custom"
    );
}

// Ported from dbtests/querytests.cpp
// d4fad81da14cf89587592f0188ca0cdfb5d4d38c
TEST_F(DBClientTest, ManualGetMore) {
    for(int i = 0; i < 3; ++i) {
        c.insert(TEST_NS, BSON("num" << i));
    }
    auto_ptr<DBClientCursor> cursor = c.query(TEST_NS, Query("{}"), 2);
    uint64_t cursor_id = cursor->getCursorId();
    cursor->decouple();
    cursor.reset();
    cursor = c.getMore(TEST_NS, cursor_id);
    ASSERT_TRUE(cursor->more());
    ASSERT_EQUALS(cursor->next().getIntField("num"), 2);
}

TEST_F(DBClientTest, InsertVector) {
    std::vector<BSONObj> v;
    v.push_back(BSON("num" << 1));
    v.push_back(BSON("num" << 2));
    c.insert(TEST_NS, v);
    ASSERT_EQUALS(c.count(TEST_NS), 2);
}

TEST_F(DBClientTest, InsertVectorContinueOnError) {
    std::vector<BSONObj> v;
    v.push_back(BSON("_id" << 1));
    v.push_back(BSON("_id" << 1));
    v.push_back(BSON("_id" << 2));
    c.insert(TEST_NS, v, InsertOption_ContinueOnError);
    ASSERT_EQUALS(c.count(TEST_NS), 2);
}

TEST_F(DBClientTest, GetIndexes) {
    auto_ptr<DBClientCursor> cursor = c.getIndexes(TEST_NS);
    ASSERT_FALSE(cursor->more());

    c.insert(TEST_NS, BSON("test" << true));
    cursor = c.getIndexes(TEST_NS);
    ASSERT_EQUALS(cursor->itcount(), 1);

    c.ensureIndex(TEST_NS, BSON("test" << 1));
    cursor = c.getIndexes(TEST_NS);
    std::vector<BSONObj> v;
    while(cursor->more())
        v.push_back(cursor->next());
    ASSERT_EQUALS(v.size(), 2);
    ASSERT_EQUALS(v[0]["name"].String(), "_id_");
    ASSERT_EQUALS(v[1]["name"].String(), "test_1");
}

TEST_F(DBClientTest, DropIndexes) {
    c.ensureIndex(TEST_NS, BSON("test" << 1));
    unsigned index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 2);
    c.dropIndexes(TEST_NS);
    index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 1);
}

TEST_F(DBClientTest, DropIndex) {
    c.ensureIndex(TEST_NS, BSON("test" << 1));
    c.ensureIndex(TEST_NS, BSON("test2" << -1));
    unsigned index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 3);

    // Interface that takes an index key obj
    c.dropIndex(TEST_NS, BSON("test" << 1));
    index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 2);

    // Interface that takes an index name
    c.dropIndex(TEST_NS, "test2_-1");
    index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 1);

    // Drop of unknown index should throw an error
    ASSERT_THROWS(c.dropIndex(TEST_NS, "test3_1"), DBException);
}

TEST_F(DBClientTest, ReIndex) {
    c.ensureIndex(TEST_NS, BSON("test" << 1));
    c.ensureIndex(TEST_NS, BSON("test2" << -1));
    unsigned index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 3);
    c.reIndex(TEST_NS);
    index_count = c.getIndexes(TEST_NS)->itcount();
    ASSERT_EQUALS(index_count, 3);
}

TEST_F(DBClientTest, CreateCollection) {
    ASSERT_FALSE(c.exists(TEST_NS));
    ASSERT_TRUE(c.createCollection(TEST_NS));
    ASSERT_FALSE(c.createCollection(TEST_NS));
    ASSERT_TRUE(c.exists(TEST_NS));
}

TEST_F(DBClientTest, CopyDatabase) {
    c.dropDatabase("copy");
    c.insert(TEST_NS, BSON("test" << true));
    ASSERT_TRUE(c.copyDatabase("test", "copy"));
    c.exists("copy.foo");
    BSONObj doc = c.findOne("copy.foo", Query("{}"));
    ASSERT_TRUE(doc["test"].boolean());
}

TEST_F(DBClientTest, DBProfilingLevel) {
    DBClientWithCommands::ProfilingLevel level;
    ASSERT_TRUE(c.setDbProfilingLevel("test", c.ProfileAll));
    ASSERT_TRUE(c.getDbProfilingLevel("test", level, 0));
    ASSERT_EQUALS(level, c.ProfileAll);

    ASSERT_TRUE(c.setDbProfilingLevel("test", c.ProfileSlow));
    ASSERT_TRUE(c.getDbProfilingLevel("test", level, 0));
    ASSERT_EQUALS(level, c.ProfileSlow);

    ASSERT_TRUE(c.setDbProfilingLevel("test", c.ProfileOff));
    ASSERT_TRUE(c.getDbProfilingLevel("test", level, 0));
    ASSERT_EQUALS(level, c.ProfileOff);
}

TEST_F(DBClientTest, QueryJSON) {
    Query q(string("{name: 'Tyler'}"));
    BSONObj filter = q.getFilter();
    ASSERT_TRUE(filter.hasField("name"));
    ASSERT_EQUALS(filter["name"].String(), "Tyler");
}

boost::function<void(const BSONObj &)> f;

struct func {
    void operator()(const BSONObj &) {
        // NOP
    }
};

// This also excercises availableOptions (which is protected)
TEST_F(DBClientTest, Exhaust) {
    for(int i=0; i<1000; ++i)
        c.insert(TEST_NS, BSON("num" << i));
    f = func();
    c.query(f, TEST_NS, Query("{}"));
}

TEST_F(DBClientTest, GetLastError) {
    c.insert(TEST_NS, BSON("_id" << 1));
    c.insert(TEST_NS, BSON("_id" << 1));
    // TODO: MOAR TESTS
}

TEST_F(DBClientTest, GetPrevError) {
    c.insert(TEST_NS, BSON("_id" << 1));
    c.insert(TEST_NS, BSON("_id" << 1));
    c.insert(TEST_NS, BSON("_id" << 2));
    ASSERT_TRUE(c.getLastError().empty());
    ASSERT_FALSE(c.getPrevError().isEmpty());
}

TEST_F(DBClientTest, MaxScan) {
    for(int i = 0; i < 100; ++i) {
        c.insert(TEST_NS, fromjson("{}"));
    }
    std::vector<BSONObj> results;
    c.findN(results, TEST_NS, Query("{}"), 100);
    ASSERT_EQUALS(results.size(), 100);
    results.clear();
    c.findN(results, TEST_NS, Query("{$query: {}, $maxScan: 50}"), 100);
    ASSERT_EQUALS(results.size(), 50);
}

TEST_F(DBClientTest, ReturnKey) {
    c.insert(TEST_NS, BSON("a" << true << "b" << true));

    BSONObj result;

    result = c.findOne(TEST_NS, Query("{$query: {a: true}}"));
    ASSERT_TRUE(result.hasField("a"));
    ASSERT_TRUE(result.hasField("b"));

    result = c.findOne(TEST_NS, Query("{$query: {a: true}, $returnKey: true}"));
    ASSERT_FALSE(result.hasField("a"));
    ASSERT_FALSE(result.hasField("b"));

    c.ensureIndex(TEST_NS, BSON("a" << 1));
    result = c.findOne(TEST_NS, Query("{$query: {a: true}, $returnKey: true}"));
    ASSERT_TRUE(result.hasField("a"));
    ASSERT_FALSE(result.hasField("b"));
}

TEST_F(DBClientTest, ShowDiskLoc) {
    c.insert(TEST_NS, BSON("a" << true));

    BSONObj result;

    result = c.findOne(TEST_NS, Query("{$query: {}}"));
    ASSERT_FALSE(result.hasField("$diskLoc"));

    result = c.findOne(TEST_NS, Query("{$query: {}, $showDiskLoc: true}"));
    ASSERT_TRUE(result.hasField("$diskLoc"));
}

TEST_F(DBClientTest, DISABLED_MaxTimeMS) {
    // Requires --setParameter=enableTestCommands=1
    c.insert(TEST_NS, BSON("a" << true));
    BSONObj result;

    c.runCommand("admin", BSON("buildinfo" << true), result);
    if (result["version"].toString() >= "2.5.3"){
        c.runCommand("admin", BSON(
            "configureFailPoint" << "maxTimeAlwaysTimeOut" <<
            "mode" << BSON("times" << 2)
        ), result);

        // First test with a query
        ASSERT_NO_THROW(
            c.findOne(TEST_NS, Query("{$query: {}}"));
        );
        ASSERT_THROWS(
            c.findOne(TEST_NS, Query("{$query: {}, $maxTimeMS: 1}"));
        , DBException);

        // Then test with a command
        ASSERT_TRUE(
            c.runCommand("test", BSON("count" << "test"), result)
        );
        ASSERT_FALSE(
            c.runCommand("test",
                BSON("count" << "test" << "maxTimeMS" << 1), result)
        );
    } else {
        // we are not connected to MongoDB >= 2.5.3, skip
        SUCCEED();
    }
}

TEST_F(DBClientTest, Comment) {
    c.insert(TEST_NS, BSON("a" << true));
    c.dropCollection("test.system.profile");
    c.setDbProfilingLevel("test", c.ProfileAll);
    c.findOne(TEST_NS, Query("{$query: {a: 'z'}, $comment: 'wow'})"));
    c.setDbProfilingLevel("test", c.ProfileOff);
    BSONObj result = c.findOne("test.system.profile", BSON(
        "ns" << "test.foo" <<
        "op" << "query" <<
        "query.$comment" << "wow"
    ));
    ASSERT_FALSE(result.isEmpty());
}
