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

#include "driver/base/database.hpp"

#include "mongoc.h"

namespace mongo {
namespace driver {

class database::impl {
public:
    ~impl() { mongoc_database_destroy(database_t); }
    mongoc_database_t* database_t;
    const class client* client;
    std::string name;
};

}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
