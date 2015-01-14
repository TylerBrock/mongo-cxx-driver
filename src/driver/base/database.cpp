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

#include "driver/base/database.hpp"

#include "driver/base/client.hpp"
#include "driver/base/private/database.hpp"
#include "driver/base/private/client.hpp"
#include "driver/base/private/read_preference.hpp"
#include "driver/private/libmongoc.hpp"

#include "stdx/make_unique.hpp"

namespace mongo {
namespace driver {

database_t::database_t(database_t&&) noexcept = default;
database_t& database_t::operator=(database_t&&) noexcept = default;

database_t::~database_t() = default;

database_t::database_t(const client_t& client, const std::string& name)
    : _impl(stdx::make_unique<impl>(
          libmongoc::client_get_database(client._impl->client_t, name.c_str()), client._impl.get(),
          name.c_str())) {}

const std::string& database_t::name() const { return _impl->name; }

void database_t::read_preference(read_preference_t rp) {
    libmongoc::database_set_read_prefs(_impl->database_t, rp._impl->read_preference_t);
}

read_preference_t database_t::read_preference() const {
    read_preference_t rp(stdx::make_unique<read_preference_t::impl>(
        libmongoc::read_prefs_copy(libmongoc::database_get_read_prefs(_impl->database_t))));
    return rp;
}

void database_t::write_concern(write_concern_t wc) {
    libmongoc::database_set_write_concern(_impl->database_t, wc._impl->write_concern_t);
}

write_concern_t database_t::write_concern() const {
    write_concern_t wc(stdx::make_unique<write_concern_t::impl>(
        libmongoc::write_concern_copy(libmongoc::database_get_write_concern(_impl->database_t)))
    );
    return wc;
}

collection_t database_t::collection(const std::string& name) const & {
    return collection_t(*this, name);
}

}  // namespace driver
}  // namespace mongo
