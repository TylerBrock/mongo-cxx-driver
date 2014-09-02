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
#include "driver/models/read.hpp"
#include "driver/util/optional.hpp"

namespace mongo {
namespace driver {
namespace model {

class LIBMONGOCXX_EXPORT count : public ReadModel<count> {

   public:
    count();

    count& criteria(bson::document::view criteria);
    count& hint(bson::document::view hint);
    count& limit(std::int32_t limit);
    count& max_time_ms(std::int64_t max_time_ms);
    count& skip(std::int32_t skip);

    optional<bson::document::view> criteria() const;
    optional<bson::document::view> hint() const;
    optional<std::int32_t> limit() const;
    optional<std::int64_t> max_time_ms() const;
    optional<std::int32_t> skip() const;

   private:
    optional<bson::document::view> _criteria;
    optional<bson::document::view> _hint;
    optional<std::int32_t> _limit;
    optional<std::int64_t> _max_time_ms;
    optional<std::int32_t> _skip;
};

}  // namesapce model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
