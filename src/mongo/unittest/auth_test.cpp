#include <iostream>
#include <cstdlib>
#include <string>

#include <boost/scoped_ptr.hpp>

#include "mongo/unittest/integration_test.h"
#include "mongo/client/dbclient.h"

using std::cout;
using std::endl;
using std::string;
using namespace mongo;
using namespace mongo::unittest;

// TODO: test logout, that auth actually allows some sort of access

TEST(Auth, BasicTest) {
    string errmsg;
    string port = integrationTestParams.port;
    ConnectionString cs = ConnectionString::parse(string("127.0.0.1:") + port, errmsg);
    ASSERT_TRUE(cs.isValid());

    boost::scoped_ptr<DBClientBase> conn(cs.connect(errmsg));
    ASSERT_TRUE(conn);

    bool worked;
    BSONObj ret;

    // clean up old data from any previous tests
    worked = conn->runCommand("test", BSON("dropAllUsersFromDatabase" << 1), ret);

    // check if it worked, fallback to old school remove if not
    if (!worked) {
        cout << "Running MongoDB < 2.5.3 so falling back to old remove" << endl;
        conn->remove("test.system.users" , BSONObj());
    }

    // create a new user
    worked = conn->runCommand( "test",
        BSON(
            "createUser" << "eliot" <<
            "pwd" << "bar" <<
            "roles" << BSON_ARRAY("readWrite")
        ),
    ret);

    // check if it worked, fallback to old createremove if not
    if (!worked) {
        cout << "Running MongoDB < 2.5.3 so falling back to old user creation" << endl;
        conn->insert(
            "test.system.users" , BSON(
                "user" << "eliot" <<
                "pwd" << conn->createPasswordDigest( "eliot" , "bar" )
            )
        );
    }

    conn->insert("test.foo", BSON("youcanreadme" << true));
    conn->findOne("test.foo", Query("{}"));

    errmsg.clear();
    ASSERT_NO_THROW(
        conn->auth(
            BSON(
                "user" << "eliot" <<
                "db" << "test" <<
                "pwd" << "bar" <<
                "mechanism" << "MONGODB-CR"
            )
        )
    );

    ASSERT_NO_THROW(
        conn->auth(
            "test",  // dbname
            "eliot", // username
            "bar",   // password
            errmsg,  // error string
            true     // digestpw
        )
    );

    ASSERT_THROWS(
        conn->auth(
            BSON("user" << "eliot" <<
                "db" << "test" <<
                "pwd" << "bars" << // incorrect password
                "mechanism" << "MONGODB-CR")
            );
    , DBException);

    conn->findOne("test.foo", Query("{}"));
    conn->logout("test", ret);
}

#ifdef MONGO_SSL
TEST(Auth, X509) {
    DBClientConnection conn;
    conn.connect("ldaptest.10gen.cc"); // only available internally or on jenkins

    conn.auth(BSON(
        "mechanism" << "MONGODB-X509" <<
        "user" << "CN=client,OU=kerneluser,O=10Gen,L=New York City,ST=New York,C=US" <<
        "db" << "$external"
    ));

    BSONObj result = conn.findOne("x509.test", Query("{}"));
    ASSERT_TRUE(result["x509"].trueValue());
    ASSERT_EQUALS(result["authenticated"].str(), "yeah");
}
#endif
