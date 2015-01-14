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

#pragma once

#include "driver/config/prelude.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

#include "bson/builder.hpp"
#include "bson/document.hpp"
#include "driver/base/bulk_write.hpp"
#include "driver/base/cursor.hpp"
#include "driver/base/read_preference.hpp"
#include "driver/base/write_concern.hpp"
#include "driver/options/aggregate.hpp"
#include "driver/options/bulk_write.hpp"
#include "driver/options/count.hpp"
#include "driver/options/delete.hpp"
#include "driver/options/distinct.hpp"
#include "driver/options/find.hpp"
#include "driver/options/find_one_and_delete.hpp"
#include "driver/options/find_one_and_replace.hpp"
#include "driver/options/find_one_and_update.hpp"
#include "driver/options/insert.hpp"
#include "driver/options/update.hpp"
#include "driver/result/bulk_write.hpp"
#include "driver/result/delete.hpp"
#include "driver/result/insert_many.hpp"
#include "driver/result/insert_one.hpp"
#include "driver/result/replace_one.hpp"
#include "driver/result/update.hpp"

namespace mongo {
namespace driver {

class client_t;
class database_t;
class pipeline_t;

// TODO: make copyable when c-driver supports this

class LIBMONGOCXX_EXPORT collection_t {

   public:
    collection_t(collection_t&& other) noexcept;
    collection_t& operator=(collection_t&& rhs) noexcept;

    ~collection_t();

    cursor_t aggregate(
        const pipeline_t& pipeline,
        const options::aggregate_t& options = options::aggregate_t()
    );

    template<class Container>
    inline optional<result::bulk_write_t> bulk_write(
        const Container& requests,
        const options::bulk_write_t& options = options::bulk_write_t()
    );

    template<class WriteModelIterator>
    inline optional<result::bulk_write_t> bulk_write(
        WriteModelIterator begin,
        WriteModelIterator end,
        const options::bulk_write_t& options = options::bulk_write_t()
    );

    optional<result::bulk_write_t> bulk_write(
        const bulk_write_t& bulk_write
    );

    std::int64_t count(
        bson::document::view filter,
        const options::count_t& options = options::count_t()
    );

    bson::document::value create_index(
        bson::document::view keys,
        bson::document::view options
    );

    optional<result::delete_t> delete_one(
        bson::document::view filter,
        const options::delete_t& options = options::delete_t()
    );

    optional<result::delete_t> delete_many(
        bson::document::view filter,
        const options::delete_t& options = options::delete_t()
    );

    cursor_t distinct(
        const std::string& field_name,
        bson::document::view filter,
        const options::distinct_t& options = options::distinct_t()
    );

    void drop();

    cursor_t find(
        bson::document::view filter,
        const options::find_t& options = options::find_t()
    );

    optional<bson::document::value> find_one(
        bson::document::view filter,
        const options::find_t& options = options::find_t()
    );

    optional<bson::document::value> find_one_and_delete(
        bson::document::view filter,
        const options::find_one_and_delete_t& options = options::find_one_and_delete_t()
    );

    optional<bson::document::value> find_one_and_update(
        bson::document::view filter,
        bson::document::view update,
        const options::find_one_and_update_t& options = options::find_one_and_update_t()
    );

    optional<bson::document::value> find_one_and_replace(
        bson::document::view filter,
        bson::document::view replacement,
        const options::find_one_and_replace_t& options = options::find_one_and_replace_t()
    );

    optional<result::insert_one_t> insert_one(
        bson::document::view document,
        const options::insert_t& options = options::insert_t()
    );

    template<class Container>
    inline optional<result::insert_many_t> insert_many(
        const Container& container,
        const options::insert_t& options = options::insert_t()
    );

    // TODO: document DocumentViewIterator concept or static assert
    template<class DocumentViewIterator>
    inline optional<result::insert_many_t> insert_many(
        DocumentViewIterator begin,
        DocumentViewIterator end,
        const options::insert_t& options = options::insert_t()
    );

    cursor_t list_indexes() const;

    const std::string& name() const;

    void read_preference(read_preference_t rp);
    read_preference_t read_preference() const;

    optional<result::replace_one_t> replace_one(
        bson::document::view filter,
        bson::document::view replacement,
        const options::update_t& options = options::update_t()
    );

    optional<result::update_t> update_one(
        bson::document::view filter,
        bson::document::view update,
        const options::update_t& options = options::update_t()
    );

    optional<result::update_t> update_many(
        bson::document::view filter,
        bson::document::view update,
        const options::update_t& options = options::update_t()
    );

    void write_concern(write_concern_t wc);
    write_concern_t write_concern() const;

   private:
    friend database_t;

    collection_t(const database_t& database, const std::string& collection_name);

    class impl;
    std::unique_ptr<impl> _impl;

}; // class collection_t

template<class Container>
inline optional<result::bulk_write_t> collection_t::bulk_write(
    const Container& requests,
    const options::bulk_write_t& options
) {
    return bulk_write(requests.begin(), requests.end(), options);
}

template<class WriteModelIterator>
inline optional<result::bulk_write_t> collection_t::bulk_write(
    WriteModelIterator begin,
    WriteModelIterator end,
    const options::bulk_write_t& options
) {
    class bulk_write_t writes(options.ordered().value_or(true));

    std::for_each(begin, end, [&](const model::write_t& current){
        writes.append(current);
    });

    return bulk_write(writes);
}

template<class Container>
inline optional<result::insert_many_t> collection_t::insert_many(
    const Container& container,
    const options::insert_t& options
) {
    return insert_many(container.begin(), container.end(), options);
}

template<class DocumentViewIterator>
inline optional<result::insert_many_t> collection_t::insert_many(
    DocumentViewIterator begin,
    DocumentViewIterator end,
    const options::insert_t& options
) {
    class bulk_write_t writes(false);

    std::map<std::size_t, bson::document::element> inserted_ids{};
    size_t index = 0;
    std::for_each(begin, end, [&](const bson::document::view& current){
        // TODO: put this somewhere else not in header scope (bson::builder)
        if ( !current.has_key("_id")) {
            bson::builder::document new_document;
            new_document << "_id" << bson::oid(bson::oid::init_tag);
            new_document << bson::builder::helpers::concat{current};

            writes.append(model::insert_one_t(new_document.view()));

            inserted_ids.emplace(index++, new_document.view()["_id"]);
        } else {
            writes.append(model::insert_one_t(current));

            inserted_ids.emplace(index++, current["_id"]);
        }

    });

    if (options.write_concern())
        writes.write_concern(*options.write_concern());
    result::bulk_write_t res(std::move(bulk_write(writes).value()));
    optional<result::insert_many_t> result(result::insert_many_t(std::move(res), std::move(inserted_ids)));
    return result;
}

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
