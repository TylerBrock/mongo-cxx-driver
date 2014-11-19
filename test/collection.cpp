#include "catch.hpp"

#include "bson/builder.hpp"
#include "bson/types.hpp"
#include "mongocxx.hpp"

using namespace mongo::driver;

TEST_CASE("CRUD functionality", "[driver::collection]") {
    mongoc_init();
    client mongodb_client;
    database db = mongodb_client["test"];
    collection coll = db["mongo-cxx-driver"];
    coll.drop();

    SECTION("insert and read single document", "[collection]") {
        bson::builder::document b;
        b << "_id" << bson::oid{bson::oid::init_tag} << "x" << 1;

        REQUIRE(coll.insert_one(b));

        auto cursor = coll.find(b.view());

        std::size_t i = 0;
        for (auto&& x : cursor) {
            REQUIRE(x["_id"].get_oid().value == b.view()["_id"].get_oid().value);
            i++;
        }

        REQUIRE(i == 1);
    }

    SECTION("insert and read multiple documents", "[collection]") {
        bson::builder::document b1, b2;

        b1 << "_id" << 1;
        b2 << "_id" << 2;

        std::vector<bson::document::view> inserts{b1, b2};
        result::insert_many result = coll.insert_many(inserts).value();

        auto cursor = coll.find(bson::document::view());

        std::int32_t sum = 0;
        for (auto&& doc : cursor) {
            sum += doc["_id"].get_int32();
        }

        REQUIRE(sum == 3);
    }

    SECTION("insert and update single document", "[collection]") {
        using namespace bson::builder::helpers;
        bson::builder::document b1;
        b1 << "_id" << 1;

        coll.insert_one(b1.view());

        auto doc = coll.find_one(bson::document::view());
        REQUIRE(doc);
        REQUIRE(doc->view()["_id"].get_int32() == 1);

        bson::builder::document update_doc;
        update_doc << "$set" << open_doc << "changed" << true << close_doc;

        coll.update_one(b1, update_doc);

        auto updated = coll.find_one(bson::document::view());
        REQUIRE(updated);
        REQUIRE(updated->view()["changed"].get_bool() == true);
    }

    SECTION("insert and update multiple documents", "[collection]") {
        bson::builder::document b1;
        b1 << "x" << 1;

        coll.insert_one(b1.view());
        coll.insert_one(b1.view());

        bson::builder::document b2;
        b2 << "x" << 2;

        coll.insert_one(b2.view());

        REQUIRE(coll.count(b1.view()) == 2);

        bson::builder::document bchanged;
        bchanged << "changed" << true;

        bson::builder::document update_doc;
        update_doc << "$set" << bson::types::b_document{bchanged.view()};

        coll.update_many(b1, update_doc);

        REQUIRE(coll.count(bchanged.view()) == 2);
    }

    SECTION("replace document replaces only one document", "[collection]") {
        bson::builder::document doc;
        doc << "x" << 1;

        coll.insert_one(doc.view());
        coll.insert_one(doc.view());

        REQUIRE(coll.count(doc.view()) == 2);

        bson::builder::document replacement;
        replacement << "x" << 2;

        coll.replace_one(doc.view(), replacement.view());

        REQUIRE(coll.count(doc.view()) == 1);
    }

    SECTION("non-matching upsert creates document", "[collection]") {
        using namespace bson::builder::helpers;
        bson::builder::document b1;
        b1 << "_id" << 1;

        bson::builder::document update_doc;
        update_doc << "$set" << open_doc << "changed" << true << close_doc;

        options::update options;
        options.upsert(true);

        coll.update_one(b1, update_doc, options);

        auto updated = coll.find_one(bson::document::view());
        REQUIRE(updated);
        REQUIRE(updated->view()["changed"].get_bool() == true);
        REQUIRE(coll.count(bson::document::view()) == 1);
    }

    SECTION("matching upsert updates document", "[collection]") {
        using namespace bson::builder::helpers;
        bson::builder::document b1;
        b1 << "_id" << 1;

        coll.insert_one(b1.view());

        bson::builder::document update_doc;
        update_doc << "$set" << open_doc << "changed" << true << close_doc;

        options::update options;
        options.upsert(true);

        coll.update_one(b1, update_doc, options);

        auto updated = coll.find_one(bson::document::view());
        REQUIRE(updated);
        REQUIRE(updated->view()["changed"].get_bool() == true);
        REQUIRE(coll.count(bson::document::view()) == 1);
    }

    //SECTION("matching upsert updates document", "[collection]") {
        //bson::builder::document b1;
        //b1 << "x" << 1;
        //model::insert_many docs{std::initializer_list<bson::document::view>{b1, b1, b1}};
        //coll.insert_many(docs);

        //coll.insert_one(bson::document::view{});
        //REQUIRE(coll.count(b1.view()) == 3);
        //REQUIRE(coll.count() == 4);
    //}

    SECTION("document replacement", "[collection]") {
        bson::builder::document b1;
        b1 << "x" << 1;
        coll.insert_one(b1.view());

        bson::builder::document b2;
        b2 << "x" << 2;

        coll.replace_one(b1, b2);

        auto replaced = coll.find_one(b2.view());

        REQUIRE(replaced);
        REQUIRE(coll.count(bson::document::view()) == 1);
    }

    SECTION("filtered document delete one works", "[collection]") {
        bson::builder::document b1;
        b1 << "x" << 1;

        coll.insert_one(b1.view());

        bson::builder::document b2;
        b2 << "x" << 2;

        coll.insert_one(b2.view());
        coll.insert_one(b2.view());

        REQUIRE(coll.count(bson::document::view()) == 3);

        coll.delete_one(b2.view());

        REQUIRE(coll.count(bson::document::view()) == 2);

        auto cursor = coll.find(bson::document::view());

        unsigned seen = 0;
        for (auto&& x : cursor) {
            seen |= x["x"].get_int32();
        }

        REQUIRE(seen == 3);

        coll.delete_one(b2.view());

        REQUIRE(coll.count(bson::document::view()) == 1);

        cursor = coll.find(bson::document::view());

        seen = 0;
        for (auto&& x : cursor) {
            seen |= x["x"].get_int32();
        }

        REQUIRE(seen == 1);

        coll.delete_one(b2.view());

        REQUIRE(coll.count(bson::document::view()) == 1);

        cursor = coll.find(bson::document::view());

        seen = 0;
        for (auto&& x : cursor) {
            seen |= x["x"].get_int32();
        }

        REQUIRE(seen == 1);
    }

    SECTION("delete many works", "[collection]") {
        bson::builder::document b1;
        b1 << "x" << 1;

        coll.insert_one(b1.view());

        bson::builder::document b2;
        b2 << "x" << 2;

        coll.insert_one(b2);
        coll.insert_one(b2);

        REQUIRE(coll.count(bson::document::view()) == 3);

        coll.delete_many(b2);

        REQUIRE(coll.count(bson::document::view()) == 1);

        auto cursor = coll.find(bson::document::view());

        unsigned seen = 0;
        for (auto&& x : cursor) {
            seen |= x["x"].get_int32();
        }

        REQUIRE(seen == 1);

        coll.delete_many(b2);

        REQUIRE(coll.count(bson::document::view()) == 1);

        cursor = coll.find(bson::document::view());

        seen = 0;
        for (auto&& x : cursor) {
            seen |= x["x"].get_int32();
        }

        REQUIRE(seen == 1);
    }

    SECTION("find_one_and_replace works", "[collection]") {
        bson::builder::document b1;
        b1 << "x" << 1;

        coll.insert_one(b1);
        coll.insert_one(b1);

        REQUIRE(coll.count(bson::document::view()) == 2);

        bson::builder::document criteria;
        bson::builder::document replacement;

        criteria << "x" << 1;
        replacement << "x" << 2;

        SECTION("without return replacement returns original") {
            auto doc = coll.find_one_and_replace(criteria, replacement);
            REQUIRE(doc->view()["x"].get_int32() == 1);
        }

        SECTION("with return replacement returns new") {
            options::find_one_and_replace options;
            options.return_replacement(true);
            auto doc = coll.find_one_and_replace(criteria, replacement);

            REQUIRE(doc->view()["x"].get_int32() == 2);
        }

        SECTION("bad criteria returns negative optional") {
            bson::builder::document bad_criteria;
            bad_criteria << "x" << 3;

            auto doc =
                coll.find_one_and_replace(bad_criteria, replacement);

            REQUIRE(!doc);
        }
    }

    SECTION("find_one_and_update works", "[collection]") {
        using namespace bson::builder::helpers;

        bson::builder::document b1;
        b1 << "x" << 1;

        coll.insert_one(b1.view());
        coll.insert_one(b1.view());

        REQUIRE(coll.count(bson::document::view()) == 2);

        bson::builder::document criteria;
        bson::builder::document update;

        criteria << "x" << 1;
        update << "$set" << open_doc << "x" << 2 << close_doc;

        SECTION("without return update returns original") {
            auto doc = coll.find_one_and_update(criteria, update);

            REQUIRE(doc);

            REQUIRE(doc->view()["x"].get_int32() == 1);
        }

        SECTION("with return update returns new") {
            options::find_one_and_update options;
            options.return_replacement(true);
            auto doc = coll.find_one_and_update(criteria, update);

            REQUIRE(doc);
            REQUIRE(doc->view()["x"].get_int32() == 2);
        }

        SECTION("bad criteria returns negative optional") {
            bson::builder::document bad_criteria;
            bad_criteria << "x" << 3;

            auto doc = coll.find_one_and_update(bad_criteria, update);

            REQUIRE(!doc);
        }
    }

    SECTION("find_one_and_delete works", "[collection]") {
        using namespace bson::builder::helpers;

        bson::builder::document b1;
        b1 << "x" << 1;

        coll.insert_one(b1.view());
        coll.insert_one(b1.view());

        REQUIRE(coll.count(bson::document::view()) == 2);

        bson::builder::document criteria;

        criteria << "x" << 1;

        SECTION("delete one deletes one and returns it") {
            auto doc = coll.find_one_and_delete(criteria);

            REQUIRE(doc);

            REQUIRE(doc->view()["x"].get_int32() == 1);
            REQUIRE(coll.count(bson::document::view()) == 1);
        }
    }

    SECTION("aggregate some shit", "[collection]") {
        bson::builder::document b1;
        b1 << "x" << 1;

        bson::builder::document b2;
        b2 << "x" << 2;

        coll.insert_one(b1.view());
        coll.insert_one(b2.view());
        coll.insert_one(b2.view());

        pipeline p;
        p.match(b1.view());

        auto results = coll.aggregate(p);
    }
}
