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

#include "driver/base/database.hpp"
#include "driver/base/read_preference.hpp"
#include "driver/base/uri.hpp"
#include "driver/base/write_concern.hpp"
#include "driver/options/client.hpp"

namespace mongo {
namespace driver {

/// The client_t class is the entry point into the MongoDB driver. It acts as a logical gateway for
/// accessing the databases of MongoDB clusters. Databases that are accessed via a client_t inherit
/// all of the options specified on the client_t.
class LIBMONGOCXX_EXPORT client_t {

    // TODO: iterable for databases on the server
    // TODO: add + implement client_t api methods
   public:
    client_t(
        const uri_t& mongodb_uri = uri_t(),
        const options::client_t& options = options::client_t()
    );

    client_t(client_t&& rhs) noexcept;
    client_t& operator=(client_t&& rhs) noexcept;

    ~client_t();

    // TODO: document that modifications at this level do not affect existing client_ts + databases
    void read_preference(read_preference_t rp);
    read_preference_t read_preference() const;

    // TODO: document that modifications at this level do not affect existing client_ts + databases
    void write_concern(write_concern_t wc);
    write_concern_t write_concern() const;

    database_t database(const std::string& name) const &;
    database_t database(const std::string& name) const && = delete;

    inline database_t operator[](const std::string& name) const &;
    inline database_t operator[](const std::string& name) const && = delete;

   private:
    friend class database_t;
    friend class collection_t;

    class impl;
    std::unique_ptr<impl> _impl;

}; // class client_t

inline database_t client_t::operator[](const std::string& name) const & {
    return database(name);
}

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
