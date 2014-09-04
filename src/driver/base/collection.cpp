// Copyright 2014 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>

#include "bson/builder.hpp"

#include "driver/libmongoc.hpp"

#include "driver/base/collection.hpp"
#include "driver/base/client.hpp"
#include "driver/model/aggregate.hpp"
#include "driver/model/find.hpp"
#include "driver/model/insert_one.hpp"
#include "driver/model/update_one.hpp"
#include "driver/private/cast.hpp"
#include "driver/result/bulk_write.hpp"
#include "driver/result/insert_many.hpp"
#include "driver/result/insert_one.hpp"
#include "driver/result/remove.hpp"
#include "driver/result/remove.hpp"
#include "driver/result/replace_one.hpp"
#include "driver/result/update.hpp"
#include "driver/result/write.hpp"
#include "driver/result/bulk_write.hpp"
#include "driver/request/insert.hpp"
#include "driver/util/libbson.hpp"
#include "driver/model/bulk_write.hpp"
#include "driver/model/write.hpp"

namespace mongo {
namespace driver {

using namespace bson::libbson;

namespace {
static void mongoc_collection_dtor(void* collection_ptr) noexcept {
    mongoc_collection_destroy(static_cast<mongoc_collection_t*>(collection_ptr));
}
}  // namespace

collection::collection(const database& database, const std::string& collection_name)
    : _collection(mongoc_database_get_collection(util::cast<mongoc_database_t>(database._database),
                                                 collection_name.c_str()),
                  mongoc_collection_dtor) {}

result::bulk_write collection::bulk_write(const model::bulk_write& model) {
    using namespace model;

    mongoc_bulk_operation_t* b = mongoc_collection_create_bulk_operation(util::cast<mongoc_collection_t>(_collection), model.ordered(), NULL);

    result::bulk_write rval;

    std::size_t index = 0;

    for (auto&& operation : model.operations()) {
        switch (operation.type()) {
            case write_type::kInsertOne: {
                scoped_bson_t doc(operation.get_insert_one().document());

                mongoc_bulk_operation_insert(b, doc.bson());
                break;
            }
            case write_type::kInsertMany: {
                for (auto&& x : operation.get_insert_many().document()) {
                    scoped_bson_t doc(x);

                    mongoc_bulk_operation_insert(b, doc.bson());
                }
                break;
            }
            case write_type::kUpdateOne: {
                scoped_bson_t criteria(operation.get_update_one().criteria());
                scoped_bson_t update(operation.get_update_one().update());
                bool upsert = operation.get_update_one().upsert().value_or(false);

                mongoc_bulk_operation_update_one(b, criteria.bson(), update.bson(), upsert);
                break;
            }
            case write_type::kUpdateMany: {
                scoped_bson_t criteria(operation.get_update_many().criteria());
                scoped_bson_t update(operation.get_update_many().update());
                bool upsert = operation.get_update_many().upsert().value_or(false);

                mongoc_bulk_operation_update(b, criteria.bson(), update.bson(), upsert);
                break;
            }
            case write_type::kRemoveOne: {
                scoped_bson_t criteria(operation.get_remove_one().criteria());

                mongoc_bulk_operation_remove_one(b, criteria.bson());

                break;
            }
            case write_type::kRemoveMany: {
                scoped_bson_t criteria(operation.get_remove_many().criteria());

                mongoc_bulk_operation_remove(b, criteria.bson());

                break;
            }
            case write_type::kReplaceOne: {
                scoped_bson_t criteria(operation.get_replace_one().criteria());
                scoped_bson_t replace(operation.get_replace_one().replacement());
                bool upsert = operation.get_replace_one().upsert().value_or(false);

                mongoc_bulk_operation_replace_one(b, criteria.bson(), replace.bson(), upsert);
                break;
            }
            case write_type::kUninitialized: break; // TODO: something exceptiony
        }

        index++;
    }

    bson_t reply;
    bson_error_t error;

    if (! mongoc_bulk_operation_execute(b, &reply, &error)) {
        throw std::runtime_error(error.message);
    }

    bson::document::view reply_view{bson_get_data(&reply), reply.len};

    rval.is_acknowledged = true;
    rval.inserted_count = reply_view["nInserted"].get_int32();
    rval.matched_count = reply_view["nMatched"].get_int32();
    rval.modified_count = reply_view["nModified"].get_int32();
    rval.removed_count = reply_view["nRemoved"].get_int32();
    rval.upserted_count = reply_view["nUpserted"].get_int32();

    return rval;
}

cursor collection::find(const model::find& model) const {
    using namespace bson;

    builder filter_builder;

    scoped_bson_t filter;
    scoped_bson_t projection(model.projection());

    if (model.modifiers()) {
        filter_builder << "$query" << types::b_document{model.criteria().value_or(document::view{})}
                       << builder_helpers::concat{model.modifiers().value_or(document::view{})};

        filter.init_from_static(filter_builder.view());
    } else {
        filter.init_from_static(model.criteria());
    }

    return cursor(mongoc_collection_find(util::cast<mongoc_collection_t>(_collection),
                                         (mongoc_query_flags_t)model.cursor_flags().value_or(0),
                                         model.skip().value_or(0), model.limit().value_or(0),
                                         model.batch_size().value_or(0), filter.bson(),
                                         projection.bson(), nullptr));
}

cursor collection::aggregate(const model::aggregate& /* model */) { return cursor(nullptr); }

result::insert_one collection::insert_one(const model::insert_one& model) {
    result::bulk_write res(bulk_write(model::bulk_write(false).append(model::write{model})));

    result::insert_one result;
    result.is_acknowledged = true;
    return result;
}

result::replace_one collection::replace_one(const model::replace_one& /* model */) {
    return result::replace_one();
}

result::update collection::update_many(const model::update_many& /* model */) {
    return result::update();
}
result::remove collection::remove_many(const model::remove_many& /* model */) {
    return result::remove();
}

result::update collection::update_one(const model::update_one& /* model */) {
    return result::update();
}

result::remove collection::remove_one(const model::remove_one& /* model */) {
    return result::remove();
}

bson::document::value collection::find_one_and_replace(
    const model::find_one_and_replace& /* model */) {
    return bson::document::value((const std::uint8_t*)nullptr, 0);
}
bson::document::value collection::find_one_and_update(
    const model::find_one_and_update& /* model */) {
    return bson::document::value((const std::uint8_t*)nullptr, 0);
}
bson::document::value collection::find_one_and_remove(
    const model::find_one_and_remove& /* model */) {
    return bson::document::value((const std::uint8_t*)nullptr, 0);
}

bson::document::value collection::explain(const model::explain& /*model*/) const {
    return bson::document::value((const std::uint8_t*)nullptr, 0);
}

std::int64_t collection::count(const model::count& /* model */) const { return 0; }

void collection::drop() {
    bson_error_t error;

    if (mongoc_collection_drop(util::cast<mongoc_collection_t>(_collection), &error)) {
        /* TODO handle errors */
    }
}

}  // namespace driver
}  // namespace mongo