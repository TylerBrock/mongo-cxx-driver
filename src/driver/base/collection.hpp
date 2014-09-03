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

#include <cstdint>
#include <string>

#include "bson/document.hpp"

#include "driver/base/cursor.hpp"
#include "driver/result/distinct.hpp"
#include "driver/result/explain.hpp"
#include "driver/result/write.hpp"
#include "driver/util/unique_ptr_void.hpp"

namespace mongo {
namespace driver {

class client;
class database;
class write_concern;
class read_preference;

namespace model {
class aggregate;
class bulk_write;
class find;
class find_one_and_replace;
class find_one_and_remove;
class find_one_and_update;
class remove_one;
class remove_many;
class insert_one;
class insert_many;
class replace_one;
class update_one;
class update_many;
class distinct;
class count;
class explain;
}  // namespace model

namespace result {
class write;
class explain;
class distinct;
class bulk_write;
}  // namespace result

namespace fluent {
class findable;
class aggregatable;
}  // namespace fluent

class LIBMONGOCXX_EXPORT collection {

    friend class database;

   public:
    cursor find(const model::find& model) const;
    cursor aggregate(const model::aggregate& model) const;

    result::write insert_one(const model::insert_one& model);
    result::write insert_many(const model::insert_many& model);
    result::write replace_one(const model::replace_one& model);
    result::write update_one(const model::update_one& model);
    result::write update_many(const model::update_many& model);
    result::write remove_one(const model::remove_one& model);
    result::write remove_many(const model::remove_many& model);

    result::bulk_write bulkwrite(const model::bulk_write);

    bson::document::value find_one_and_replace(const model::find_one_and_replace& model);
    bson::document::value find_one_and_update(const model::find_one_and_update& model);
    bson::document::value find_one_and_remove(const model::find_one_and_remove& model);

    bson::document::value explain(const model::explain& model) const;
    result::distinct distinct(const model::distinct& model) const;

    std::int64_t count(const model::count& model) const;

    void drop();

   private:
    collection(const database& database, const std::string& collection_name);

    util::unique_ptr_void _collection;
};

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
