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

#include "driver/base/collection.hpp"
#include "driver/base/write_concern.hpp"
#include "driver/base/read_preference.hpp"

namespace mongo {
namespace driver {

class client;

/// The database_t class serves as a representation of a MongoDB database_t. It acts as a gateway
/// for accessing collections that are contained within the particular database_t that an instance
/// of the class represents. It inherits all of its default settings from the client that calls
/// it's constructor.
class LIBMONGOCXX_EXPORT database_t {

    // TODO: iterable for collections in the database_t
    // TODO: make copyable when c-driver supports this
    // TODO: add auth functions (add_user, remove_all_users, remove_user)
   public:
    database_t(database_t&& other) noexcept;
    database_t& operator=(database_t&& rhs) noexcept;

    ~database_t();

    bson::document::value command(bson::document::view command);

    collection_t create_collection(const std::string& name, bson::document::view options);

    void drop();

    bool has_collection(const std::string& name);

    cursor_t list_collections();

    const std::string& name() const;

    // TODO: move this next to write concern
    void read_preference(read_preference_t rp);
    read_preference_t read_preference() const;

    // TODO: should this be called move?
    void rename(
        const std::string& new_name,
        bool drop_target_before_rename
    );

    bson::document::value stats();

    void write_concern(write_concern_t wc);
    write_concern_t write_concern() const;

    collection_t collection(const std::string& name) const;
    inline collection_t operator[](const std::string& name) const;

   private:
    friend client_t;
    friend collection_t;

    database_t(const client_t& client, const std::string& name);

    class impl;
    std::unique_ptr<impl> _impl;

}; // class database_t

inline collection_t database_t::operator[](const std::string& name) const {
    return collection(name);
}

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
