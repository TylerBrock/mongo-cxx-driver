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

#include "driver/model/find.hpp"
#include "driver/model/count.hpp"
#include "driver/base/cursor.hpp"
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
class find_one_and_delete;
class find_one_and_update;
class delete_one;
class delete_many;
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
struct bulk_write;
struct insert_one;
struct insert_many;
struct replace_one;
struct update;
struct delete_result;
class write;
}  // namespace result

namespace fluent {
class findable;
class aggregatable;
}  // namespace fluent

class LIBMONGOCXX_EXPORT collection {

    class impl;

    friend class database;

   public:
    cursor find(const model::find& model = model::find{}) const;
    optional<bson::document::value> find_one(const model::find& model = model::find{}) const;

    cursor aggregate(const model::aggregate& model);

    result::insert_one insert_one(const model::insert_one& model);
    result::insert_many insert_many(const model::insert_many& model);
    result::replace_one replace_one(const model::replace_one& model);
    result::update update_one(const model::update_one& model);
    result::update update_many(const model::update_many& model);
    result::delete_result delete_one(const model::delete_one& model);
    result::delete_result delete_many(const model::delete_many& model);

    result::bulk_write bulk_write(const model::bulk_write& model);

    optional<bson::document::value> find_one_and_replace(const model::find_one_and_replace& model);
    optional<bson::document::value> find_one_and_update(const model::find_one_and_update& model);
    optional<bson::document::value> find_one_and_delete(const model::find_one_and_delete& model);

    bson::document::value explain(const model::explain& model) const;
    bson::document::value distinct(const model::distinct& model) const;

    std::int64_t count(const model::count& model = model::count{}) const;

    void drop();

    collection(collection&& rhs);
    collection& operator=(collection&& rhs);
    ~collection();

   private:
    collection(const database& database, const std::string& collection_name);

    std::unique_ptr<impl> _impl;
};

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
