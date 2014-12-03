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
#include "driver/base/read_preference.hpp"
#include "driver/util/optional.hpp"

namespace mongo {
namespace driver {
namespace options {

// TODO: take hint as a std::string parameter in addition to bson::document::view

class LIBMONGOCXX_EXPORT count {

   public:
    void hint(bson::document::view hint);
    void limit(std::int64_t limit);
    void max_time_ms(std::int64_t max_time_ms);
    void skip(std::int64_t skip);
    void read_preference(class read_preference rp);

    const optional<bson::document::view>& hint() const;
    const optional<std::int64_t>& limit() const;
    const optional<std::int64_t>& max_time_ms() const;
    const optional<std::int64_t>& skip() const;
    const optional<class read_preference>& read_preference() const;

   private:
    optional<bson::document::view> _hint;
    optional<std::int64_t> _limit;
    optional<std::int64_t> _max_time_ms;
    optional<std::int64_t> _skip;
    optional<class read_preference> _read_preference;

}; // class count

}  // namespace options
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
