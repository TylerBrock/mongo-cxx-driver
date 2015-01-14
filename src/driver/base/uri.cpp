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

#include "driver/base/uri.hpp"

#include "driver/base/private/uri.hpp"
#include "driver/private/libmongoc.hpp"

#include "stdx/make_unique.hpp"

namespace mongo {
namespace driver {

const std::string uri_t::kDefaultURI = "mongodb://localhost:27017";

uri_t::uri_t(const std::string& uri_t_string)
    : _impl(stdx::make_unique<impl>(mongoc_uri_new(uri_t_string.c_str()))) {}

uri_t::uri_t(uri_t&&) noexcept = default;
uri_t& uri_t::operator=(uri_t&&) noexcept = default;

uri_t::~uri_t() = default;

std::string uri_t::auth_mechanism() const { return mongoc_uri_get_auth_mechanism(_impl->uri_t); }

std::string uri_t::auth_source() const { return mongoc_uri_get_auth_source(_impl->uri_t); }

std::string uri_t::database() const { return mongoc_uri_get_database(_impl->uri_t); }

std::vector<uri_t::host_t> uri_t::hosts() const {
    std::vector<host_t> result;

    for (auto host_list = mongoc_uri_get_hosts(_impl->uri_t); host_list;
         host_list = host_list->next) {
        result.push_back(host_t{host_list->host, host_list->port, host_list->family});
    }

    return result;
}

std::string uri_t::password() const { return mongoc_uri_get_password(_impl->uri_t); }

std::string uri_t::replica_set() const { return mongoc_uri_get_replica_set(_impl->uri_t); }

std::string uri_t::to_string() const { return mongoc_uri_get_string(_impl->uri_t); }

bool uri_t::ssl() const { return mongoc_uri_get_ssl(_impl->uri_t); }

std::string uri_t::username() const { return mongoc_uri_get_username(_impl->uri_t); }

// const write_concern uri::write_concern() const {
// return mongoc_uri_get_write_concern(_impl->uri_t);
//}

}  // namespace driver
}  // namespace mongo
