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

#include "driver/base/collection.hpp"
#include "driver/base/database.hpp"
#include "driver/base/private/database.hpp"
#include "driver/base/private/read_preference.hpp"
#include "driver/base/private/write_concern.hpp"

#include "mongoc.h"

namespace mongo {
namespace driver {

class collection::impl {
   public:
    impl(mongoc_collection_t* collection, const class database* database, const class client* client, std::string name) :
        collection_t(collection),
        database(database),
        client(client),
        name(name)
    {
        read_preference(database->_impl->read_preference());
        write_concern(database->_impl->write_concern());
    }

    ~impl() { mongoc_collection_destroy(collection_t); }

    mongoc_collection_t* collection_t;
    const class database* database;
    const class client* client;
    std::string name;

    void read_preference(class read_preference rp) {
        priv::read_preference read_prefs{rp};

        mongoc_collection_set_read_prefs(collection_t, read_prefs.get_read_preference());

        _read_preference = std::move(rp);
    }

    void write_concern(class write_concern wc) {
        priv::write_concern write_conc{wc};

        mongoc_collection_set_write_concern(collection_t, write_conc.get_write_concern());

        _write_concern = std::move(wc);
    }

    const class read_preference& read_preference() const {
        return _read_preference;
    }

    const class write_concern& write_concern() const {
        return _write_concern;
    }
private:
    class read_preference _read_preference;
    class write_concern _write_concern;
};

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
