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
#include "stdx/make_unique.hpp"

namespace mongo {
namespace driver {

class client;
class database;
class pipeline;

class LIBMONGOCXX_EXPORT collection {

    class impl;

    friend class database;

   public:
    collection(collection&& other) noexcept;
    collection& operator=(collection&& rhs) noexcept;

    ~collection();

    cursor aggregate(
        const pipeline& pipeline,
        const options::aggregate& options = options::aggregate()
    );

    template<class Container>
    inline optional<result::bulk_write> bulk_write(
        const Container& requests,
        const options::bulk_write& options = options::bulk_write()
    );

    template<class WriteModelIterator>
    inline optional<result::bulk_write> bulk_write(
        WriteModelIterator begin,
        WriteModelIterator end,
        const options::bulk_write& options = options::bulk_write()
    );

    optional<result::bulk_write> bulk_write(
        const class bulk_write& bulk_write
    );

    // TODO: decide value vs. ref for bson::document::view types
    // TODO: almost certainly remove const, consider for client + db as well
    std::int64_t count(
        const bson::document::view& filter,
        const options::count& options = options::count()
    );

    optional<result::delete_result> delete_one(
        const bson::document::view& filter,
        const options::delete_options& options = options::delete_options()
    );

    optional<result::delete_result> delete_many(
        const bson::document::view& filter,
        const options::delete_options& options = options::delete_options()
    );

    cursor distinct(
        const std::string& field_name,
        const bson::document::view& filter,
        const options::distinct& options = options::distinct()
    );

    void drop();

    cursor find(
        const bson::document::view& filter,
        const options::find& options = options::find()
    );

    optional<bson::document::value> find_one(
        const bson::document::view& filter,
        const options::find& options = options::find()
    );

    optional<bson::document::value> find_one_and_delete(
        const bson::document::view& filter,
        const options::find_one_and_delete& options = options::find_one_and_delete()
    );

    optional<bson::document::value> find_one_and_update(
        const bson::document::view& filter,
        const bson::document::view& update,
        const options::find_one_and_update& options = options::find_one_and_update()
    );

    optional<bson::document::value> find_one_and_replace(
        const bson::document::view& filter,
        const bson::document::view& replacement,
        const options::find_one_and_replace& options = options::find_one_and_replace()
    );

    optional<result::insert_one> insert_one(
        const bson::document::view& document,
        const options::insert& options = options::insert()
    );

    template<class Container>
    inline optional<result::insert_many> insert_many(
        const Container& container,
        const options::insert& options = options::insert()
    );

    template<class DocumentViewIterator>
    inline optional<result::insert_many> insert_many(
        DocumentViewIterator begin,
        DocumentViewIterator end,
        const options::insert& options = options::insert()
    );

    void read_preference(class read_preference rp);
    class read_preference read_preference() const;

    optional<result::replace_one> replace_one(
        const bson::document::view& filter,
        const bson::document::view& replacement,
        const options::update& options = options::update()
    );

    optional<result::update> update_one(
        const bson::document::view& filter,
        const bson::document::view& update,
        const options::update& options = options::update()
    );

    optional<result::update> update_many(
        const bson::document::view& filter,
        const bson::document::view& update,
        const options::update& options = options::update()
    );

    void write_concern(class write_concern wc);
    const class write_concern& write_concern() const;

   private:
    collection(const database& database, const std::string& collection_name);

    std::unique_ptr<impl> _impl;

}; // class collection

template<class Container>
inline optional<result::bulk_write> collection::bulk_write(
    const Container& requests,
    const options::bulk_write& options
) {
    return bulk_write(requests.begin(), requests.end(), options);
}

template<class WriteModelIterator>
inline optional<result::bulk_write> collection::bulk_write(
    WriteModelIterator begin,
    WriteModelIterator end,
    const options::bulk_write& options
) {
    class bulk_write writes(options.ordered().value_or(true));

    std::for_each(begin, end, [&](const model::write& current){writes.append(current);});

    return bulk_write(writes);
}

template<class Container>
inline optional<result::insert_many> collection::insert_many(
    const Container& container,
    const options::insert& options
) {
    return insert_many(container.begin(), container.end(), options);
}

template<class DocumentViewIterator>
inline optional<result::insert_many> collection::insert_many(
    DocumentViewIterator begin,
    DocumentViewIterator end,
    const options::insert& options
) {
    class bulk_write writes(false);

    std::for_each(begin, end, [&](const bson::document::view& current){
        writes.append(model::insert_one(current));
    });

    if (options.write_concern())
        writes.write_concern(*options.write_concern());

    bulk_write(writes);
    // TODO: map result::bulk_write to result::insert_many
    return result::insert_many();
}

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
