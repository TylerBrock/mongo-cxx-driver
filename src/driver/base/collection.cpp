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

#include "driver/base/private/client.hpp"
#include "driver/base/private/collection.hpp"
#include "driver/base/private/database.hpp"
#include "driver/base/private/pipeline.hpp"
#include "driver/base/private/bulk_write.hpp"
#include "driver/base/private/write_concern.hpp"
#include "driver/base/private/read_preference.hpp"

#include "driver/base/write_concern.hpp"
#include "driver/base/collection.hpp"
#include "driver/base/client.hpp"
#include "driver/model/write.hpp"
#include "driver/result/bulk_write.hpp"
#include "driver/result/delete.hpp"
#include "driver/result/insert_many.hpp"
#include "driver/result/insert_one.hpp"
#include "driver/result/replace_one.hpp"
#include "driver/result/update.hpp"
#include "driver/result/write.hpp"
#include "driver/util/libbson.hpp"
#include "driver/util/optional.hpp"

namespace mongo {
namespace driver {

using namespace bson::libbson;

collection::collection(collection&&) noexcept = default;
collection& collection::operator=(collection&&) noexcept = default;
collection::~collection() = default;

collection::collection(const database& database, const std::string& collection_name)
    : _impl(stdx::make_unique<impl>(
          mongoc_database_get_collection(database._impl->database_t, collection_name.c_str()),
          database._impl.get(), database._impl->client_impl, collection_name.c_str())) {}

optional<result::bulk_write> collection::bulk_write(const class bulk_write& bulk_write) {
    mongoc_bulk_operation_t* b = bulk_write._impl->operation_t;
    mongoc_bulk_operation_set_database(b, _impl->database_impl->name.c_str());
    mongoc_bulk_operation_set_collection(b, _impl->name.c_str());
    mongoc_bulk_operation_set_client(b, _impl->client_impl->client_t);

    if (bulk_write.write_concern()) {
        priv::write_concern wc(*bulk_write.write_concern());
        mongoc_bulk_operation_set_write_concern(b, wc.get_write_concern());
    } else {
        mongoc_bulk_operation_set_write_concern(
            b, mongoc_collection_get_write_concern(_impl->collection_t));
    }

    scoped_bson_t reply;
    reply.flag_init();

    bson_error_t error;

    if (!mongoc_bulk_operation_execute(b, reply.bson(), &error)) {
        throw std::runtime_error(error.message);
    }

    result::bulk_write result(reply.steal());

    return optional<result::bulk_write>(std::move(result));
}

cursor collection::find(const bson::document::view& filter, const options::find& options) {
    using namespace bson;

    builder::document filter_builder;

    scoped_bson_t filter_bson;
    scoped_bson_t projection(options.projection());

    if (options.modifiers()) {
        filter_builder << "$query" << types::b_document{filter}
                       << builder::helpers::concat{options.modifiers().value_or(document::view{})};

        filter_bson.init_from_static(filter_builder.view());
    } else {
        filter_bson.init_from_static(filter);
    }

    optional<priv::read_preference> read_prefs;
    const mongoc_read_prefs_t* rp_ptr;

    if (options.read_preference()) {
        read_prefs = priv::read_preference{*options.read_preference()};
        rp_ptr = read_prefs->get_read_preference();
    } else {
        rp_ptr = mongoc_collection_get_read_prefs(_impl->collection_t);
    }

    return cursor(mongoc_collection_find(_impl->collection_t, mongoc_query_flags_t(0),
                                         options.skip().value_or(0), options.limit().value_or(0),
                                         options.batch_size().value_or(0), filter_bson.bson(),
                                         projection.bson(), rp_ptr));
}

optional<bson::document::value> collection::find_one(const bson::document::view& filter,
                                                     const options::find& options) {
    options::find copy(options);
    copy.limit(1);
    return optional<bson::document::value>(*find(filter, copy).begin());
}

cursor collection::aggregate(const pipeline& pipeline, const options::aggregate& options) {
    using namespace bson::builder::helpers;

    scoped_bson_t stages(pipeline._impl->view());

    bson::builder::document b;

    if (options.allow_disk_use()) {
        /* TODO */
    }
    if (options.use_cursor()) {
        auto inner = b << "cursor" << open_doc;

        if (options.batch_size()) {
            inner << "batchSize" << *options.batch_size();
        }

        inner << close_doc;
    }

    if (options.max_time_ms()) {
        /* TODO */
    }

    scoped_bson_t options_bson(b.view());

    const mongoc_read_prefs_t* rp_ptr;

    if (options.read_preference()) {
        rp_ptr = read_preference()._impl->read_preference_t;
    } else {
        rp_ptr = mongoc_collection_get_read_prefs(_impl->collection_t);
    }

    return cursor(mongoc_collection_aggregate(_impl->collection_t,
                                              static_cast<mongoc_query_flags_t>(0), stages.bson(),
                                              options_bson.bson(), rp_ptr));
}

optional<result::insert_one> collection::insert_one(const bson::document::view& document,
                                                    const options::insert& options) {
    class bulk_write bulk_op(false);
    model::insert_one insert(document);
    bulk_op.append(insert);

    if (options.write_concern()) bulk_op.write_concern(*options.write_concern());

    optional<result::insert_one> result(std::move(bulk_write(bulk_op).value()));

    return result;
}

optional<result::replace_one> collection::replace_one(const bson::document::view& filter,
                                                      const bson::document::view& replacement,
                                                      const options::update& options) {
    class bulk_write bulk_op(false);
    model::replace_one replace_op(filter, replacement);
    bulk_op.append(replace_op);

    if (options.write_concern()) bulk_op.write_concern(*options.write_concern());

    optional<result::bulk_write> res(bulk_write(bulk_op));

    optional<result::replace_one> result;
    return result;
}

optional<result::update> collection::update_many(const bson::document::view& filter,
                                                 const bson::document::view& update,
                                                 const options::update& options) {
    class bulk_write bulk_op(false);
    model::update_many update_op(filter, update);

    if (options.upsert()) update_op.upsert(options.upsert().value());

    bulk_op.append(update_op);

    if (options.write_concern()) bulk_op.write_concern(*options.write_concern());

    optional<result::bulk_write> res(bulk_write(bulk_op));
    optional<result::update> result;
    return result;
}

optional<result::delete_result> collection::delete_many(const bson::document::view& filter,
                                                        const options::delete_options& options) {
    class bulk_write bulk_op(false);
    model::delete_many delete_op(filter);
    bulk_op.append(delete_op);

    if (options.write_concern()) bulk_op.write_concern(*options.write_concern());

    optional<result::bulk_write> res(bulk_write(bulk_op));

    optional<result::delete_result> result;
    return result;
}

optional<result::update> collection::update_one(const bson::document::view& filter,
                                                const bson::document::view& update,
                                                const options::update& options) {
    class bulk_write bulk_op(false);
    model::update_many update_op(filter, update);

    if (options.upsert()) update_op.upsert(options.upsert().value());

    bulk_op.append(update_op);

    if (options.write_concern()) bulk_op.write_concern(*options.write_concern());

    optional<result::bulk_write> res(bulk_write(bulk_op));
    optional<result::update> result;
    return result;
}

optional<result::delete_result> collection::delete_one(const bson::document::view& filter,
                                                       const options::delete_options& options) {
    class bulk_write bulk_op(false);
    model::delete_one delete_op(filter);
    bulk_op.append(delete_op);

    if (options.write_concern()) bulk_op.write_concern(*options.write_concern());

    optional<result::bulk_write> res(bulk_write(bulk_op));

    optional<result::delete_result> result;
    return result;
}

optional<bson::document::value> collection::find_one_and_replace(
    const bson::document::view& filter, const bson::document::view& replacement,
    const options::find_one_and_replace& options) {
    scoped_bson_t bson_filter{filter};
    scoped_bson_t bson_replacement{replacement};
    scoped_bson_t bson_sort{options.sort()};
    scoped_bson_t bson_projection{options.projection()};

    scoped_bson_t reply;
    reply.flag_init();

    bson_error_t error;

    options::ReturnDocument rd =
        options.return_document().value_or(options::ReturnDocument::BEFORE);

    bool r = mongoc_collection_find_and_modify(
        _impl->collection_t, bson_filter.bson(), bson_sort.bson(), bson_replacement.bson(),
        bson_projection.bson(), false, options.upsert().value_or(false),
        rd == options::ReturnDocument::AFTER, reply.bson(), &error);

    if (!r) {
        throw std::runtime_error("baddd");
    }

    bson::document::view result = reply.view();

    if (result["value"].type() == bson::type::k_null) return optional<bson::document::value>{};

    using namespace bson::builder::helpers;
    bson::builder::document b;
    b << concat{result["value"].get_document()};
    return b.extract();
}

optional<bson::document::value> collection::find_one_and_update(
    const bson::document::view& filter, const bson::document::view& update,
    const options::find_one_and_update& options) {
    scoped_bson_t bson_filter{filter};
    scoped_bson_t bson_update{update};
    scoped_bson_t bson_sort{options.sort()};
    scoped_bson_t bson_projection{options.projection()};

    scoped_bson_t reply;
    reply.flag_init();

    bson_error_t error;

    options::ReturnDocument rd =
        options.return_document().value_or(options::ReturnDocument::BEFORE);

    bool r = mongoc_collection_find_and_modify(
        _impl->collection_t, bson_filter.bson(), bson_sort.bson(), bson_update.bson(),
        bson_projection.bson(), false, options.upsert().value_or(false),
        rd == options::ReturnDocument::AFTER, reply.bson(), &error);

    if (!r) {
        throw std::runtime_error("baddd");
    }

    bson::document::view result = reply.view();

    if (result["value"].type() == bson::type::k_null) return optional<bson::document::value>{};

    using namespace bson::builder::helpers;
    bson::builder::document b;
    b << concat{result["value"].get_document()};
    return b.extract();
}

optional<bson::document::value> collection::find_one_and_delete(
    const bson::document::view& filter, const options::find_one_and_delete& options) {
    scoped_bson_t bson_filter{filter};
    scoped_bson_t bson_sort{options.sort()};
    scoped_bson_t bson_projection{options.projection()};

    scoped_bson_t reply;
    reply.flag_init();

    bson_error_t error;

    bool r = mongoc_collection_find_and_modify(_impl->collection_t, bson_filter.bson(),
                                               bson_sort.bson(), nullptr, bson_projection.bson(),
                                               true, false, false, reply.bson(), &error);

    if (!r) {
        throw std::runtime_error("baddd");
    }

    bson::document::view result = reply.view();

    if (result["value"].type() == bson::type::k_null) return optional<bson::document::value>{};

    using namespace bson::builder::helpers;
    bson::builder::document b;
    b << concat{result["value"].get_document()};
    return b.extract();
}

std::int64_t collection::count(const bson::document::view& filter,
                               const options::count& options) {
    scoped_bson_t bson_filter{filter};
    bson_error_t error;

    const mongoc_read_prefs_t* rp_ptr;

    if (options.read_preference()) {
        rp_ptr = options.read_preference()->_impl->read_preference_t;
    } else {
        rp_ptr = mongoc_collection_get_read_prefs(_impl->collection_t);
    }

    auto result = mongoc_collection_count(_impl->collection_t, static_cast<mongoc_query_flags_t>(0),
                                          bson_filter.bson(), options.skip().value_or(0),
                                          options.limit().value_or(0), rp_ptr, &error);

    /* TODO throw an exception if error
    if (result < 0)
    */

    return result;
}

void collection::drop() {
    bson_error_t error;

    if (mongoc_collection_drop(_impl->collection_t, &error)) {
        /* TODO handle errors */
    }
}

void collection::read_preference(class read_preference rp) {
    mongoc_collection_set_read_prefs(_impl->collection_t, rp._impl->read_preference_t);
}

class read_preference collection::read_preference() const {
    class read_preference rp(
        std::make_unique<read_preference::impl>(
            mongoc_read_prefs_copy(mongoc_collection_get_read_prefs(_impl->collection_t))
        )
    );
    return rp;
}

void collection::write_concern(class write_concern wc) {
    _impl->write_concern(std::move(wc));
}

const class write_concern& collection::write_concern() const {
    return _impl->write_concern();
}

}  // namespace driver
}  // namespace mongo
