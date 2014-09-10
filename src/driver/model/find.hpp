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
#include "driver/base/read_preference.hpp"
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

class LIBMONGOCXX_EXPORT find {

   public:
    find(bson::document::view criteria = bson::document::view{});

    find& batch_size(std::int32_t batch_size);
    find& criteria(bson::document::view criteria);
    find& cursor_flags(std::int32_t cursor_flags);
    find& limit(std::int32_t limit);
    find& modifiers(bson::document::view modifiers);
    find& projection(bson::document::view projection);
    find& skip(std::int32_t skip);
    find& sort(bson::document::view ordering);
    find& max_time_ms(std::int64_t max_time_ms);
    find& read_preference(class read_preference rp);

    bson::document::view criteria() const;

    const optional<std::int32_t>& batch_size() const;
    const optional<std::int32_t>& cursor_flags() const;
    const optional<std::int32_t>& limit() const;
    const optional<bson::document::view>& modifiers() const;
    const optional<bson::document::view>& projection() const;
    const optional<std::int32_t>& skip() const;
    const optional<bson::document::view>& sort() const;
    const optional<std::int64_t>& max_time_ms() const;
    const optional<class read_preference>& read_preference() const;

   private:
    bson::document::view _criteria;

    optional<std::int32_t> _batch_size;
    optional<std::int32_t> _cursor_flags;
    optional<std::int32_t> _limit;
    optional<bson::document::view> _modifiers;
    optional<bson::document::view> _projection;
    optional<std::int32_t> _skip;
    optional<bson::document::view> _ordering;
    optional<std::int64_t> _max_time_ms;
    optional<class read_preference> _read_preference;
};

}  // namespace model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
