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
#include "driver/base/write_concern.hpp"

namespace mongo {
namespace driver {

class options;

/// The client class is the entrypoint into the MongoDB driver. It acts as a logical gateway for
/// accessing the databases of MongoDB clusters. Databases that are accessed via a client inherit
/// all of the options specified on the client.
class LIBMONGOCXX_EXPORT client {

    class impl;

    friend class database;
    friend class collection;

   public:
    client();
    explicit client(const std::string& mongodb_uri);
    explicit client(const options& options);

    void read_preference(class read_preference rp);
    const class read_preference& read_preference() const;

    void write_concern(class write_concern wc);
    const class write_concern& write_concern() const;

    class database operator[](const std::string& database_name);
    class database database(const std::string& database_name);

    // TODO: add comments as to why these are here
    // TODO: move these up with the other lifecycle members
    client(client&& rhs);
    client& operator=(client&& rhs);
    ~client();

   private:
    std::unique_ptr<impl> _impl;
};

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
