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

#include <memory>
#include <string>
#include <vector>

#include "bson/document.hpp"
#include "driver/base/read_preference.hpp"
#include "driver/base/write_concern.hpp"

namespace mongo {
namespace driver {

class uri_t {

   struct host_t {
        std::string host;
        std::uint16_t port;
        std::int32_t family;
   }; // class host

   public:

    static const std::string kDefaultURI;

    // TODO: document the default is localhost:27017...
    // TODO: this should really take a stringview (polyfilled)?
    uri_t(const std::string& uri_string = kDefaultURI);

    uri_t(uri_t&& other) noexcept;
    uri_t& operator=(uri_t&& rhs) noexcept;

    ~uri_t();

    // TODO: return const char* instead or stringview
    // TODO: harmonize with C library (options, credentials, etc...)
    std::string auth_mechanism() const;
    std::string auth_source() const;
    std::vector<host_t> hosts() const;
    std::string database() const;
    bson::document::view options() const;
    std::string password() const;
    class read_preference read_preference() const;
    std::string replica_set() const;
    bool ssl() const;
    std::string to_string() const;
    std::string username() const;
    class write_concern write_concern() const;

   private:
    friend client_t;

    class impl;
    std::unique_ptr<impl> _impl;

}; // class uri_t

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
