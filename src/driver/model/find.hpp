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
#include <set>

#include "bson/document.hpp"
#include "driver/model/read.hpp"
#include "driver/util/optional.hpp"

namespace mongo {
namespace driver {
namespace model {

enum class cursor_flag : uint32_t {
    k_tailable,
    k_oplog_replay,
    k_no_cursor_timeout,
    k_await_data,
    k_exhaust,
    k_partial
};

class LIBMONGOCXX_EXPORT find : public read<find> {

   public:
    find();

    find& batch_size(std::int32_t batch_size);
    find& criteria(bson::document::view criteria);
    find& cursor_flags(std::int32_t cursor_flags);
    find& limit(std::int32_t limit);
    find& modifiers(bson::document::view modifiers);
    find& projection(bson::document::view projection);
    find& skip(std::int32_t skip);
    find& sort(bson::document::view ordering);

    optional<std::int32_t> batch_size() const;
    optional<bson::document::view> criteria() const;
    optional<std::int32_t> cursor_flags() const;
    optional<std::int32_t> limit() const;
    optional<bson::document::view> modifiers() const;
    optional<bson::document::view> projection() const;
    optional<std::int32_t> skip() const;
    optional<bson::document::view> sort() const;

   private:
    optional<std::int32_t> _batch_size;
    optional<bson::document::view> _criteria;
    optional<std::int32_t> _cursor_flags;
    optional<std::int32_t> _limit;
    optional<bson::document::view> _modifiers;
    optional<bson::document::view> _projection;
    optional<std::int32_t> _skip;
    optional<bson::document::view> _ordering;
};

}  // namespace model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
