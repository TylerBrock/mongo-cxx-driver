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

#include "driver/options/find.hpp"
#include "driver/base/private/read_preference.hpp"

namespace mongo {
namespace driver {
namespace options {

enum class find_t::cursor_type: std::uint8_t {
    k_non_tailable,
    k_tailable,
    k_tailable_await
};

void find_t::allow_partial_results(bool allow_partial) { _allow_partial_results = allow_partial; }

void find_t::batch_size(std::int32_t batch_size) { _batch_size = batch_size; }

void find_t::comment(std::string comment) { _comment = comment; }

void find_t::limit(std::int32_t limit) { _limit = limit; }

void find_t::max_time_ms(std::int64_t max_time_ms) { _max_time_ms = max_time_ms; }

void find_t::modifiers(bson::document::view modifiers) { _modifiers = modifiers; }

void find_t::no_cursor_timeout(bool no_cursor_timeout) { _no_cursor_timeout = no_cursor_timeout; }

void find_t::oplog_replay(bool oplog_replay) { _oplog_replay = oplog_replay; }

void find_t::projection(bson::document::view projection) { _projection = projection; }

void find_t::read_preference(read_preference_t rp) { _read_preference = std::move(rp); }

void find_t::skip(std::int32_t skip) { _skip = skip; }

void find_t::sort(bson::document::view ordering) { _ordering = ordering; }

const optional<bool>& find_t::allow_partial_results() const { return _allow_partial_results; }

const optional<std::int32_t>& find_t::batch_size() const { return _batch_size; }

const optional<std::int32_t>& find_t::limit() const { return _limit; }

const optional<std::int64_t>& find_t::max_time_ms() const { return _max_time_ms; }

const optional<bson::document::view>& find_t::modifiers() const { return _modifiers; }

const optional<bool>& find_t::no_cursor_timeout() const { return _no_cursor_timeout; }

const optional<bool>& find_t::oplog_replay() const { return _oplog_replay; }

const optional<bson::document::view>& find_t::projection() const { return _projection; }

const optional<std::int32_t>& find_t::skip() const { return _skip; }

const optional<bson::document::view>& find_t::sort() const { return _ordering; }

const optional<read_preference_t>& find_t::read_preference() const { return _read_preference; }

}  // namespace options
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
