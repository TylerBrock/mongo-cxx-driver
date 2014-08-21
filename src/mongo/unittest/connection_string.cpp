    /* Connection String */
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
