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

#include "driver/base/client.hpp"

#include "driver/base/private/client.hpp"
#include "driver/base/private/read_preference.hpp"
#include "driver/base/private/write_concern.hpp"
#include "driver/base/private/uri.hpp"
#include "stdx/make_unique.hpp"

namespace mongo {
namespace driver {

client_t::client_t(const uri_t& uri, const options::client_t&)
    : _impl(stdx::make_unique<impl>(libmongoc::client_new_from_uri(uri._impl->uri_t))) {}

client_t::client_t(client_t&&) noexcept = default;
client_t& client_t::operator=(client_t&&) noexcept = default;

client_t::~client_t() = default;

void client_t::read_preference(read_preference_t rp) {
    libmongoc::client_set_read_prefs(_impl->client_t, rp._impl->read_preference_t);
}

read_preference_t client_t::read_preference() const {
    read_preference_t rp(stdx::make_unique<read_preference_t::impl>(
        libmongoc::read_prefs_copy(libmongoc::client_get_read_prefs(_impl->client_t)))
    );
    return rp;
}

void client_t::write_concern(write_concern_t wc) {
    libmongoc::client_set_write_concern(_impl->client_t, wc._impl->write_concern_t);
}

write_concern_t client_t::write_concern() const {
    write_concern_t wc(stdx::make_unique<write_concern_t::impl>(
        libmongoc::write_concern_copy(libmongoc::client_get_write_concern(_impl->client_t)))
    );
    return wc;
}

database_t client_t::database(const std::string& name) const & {
    return database_t(*this, name);
}

}  // namespace driver
}  // namespace mongo
