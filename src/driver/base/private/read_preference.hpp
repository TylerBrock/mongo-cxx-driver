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

#include "mongoc.h"

#include "driver/base/read_preference.hpp"
#include "driver/util/libbson.hpp"

namespace mongo {
namespace driver {
namespace priv {

class read_preference {
 public:
    read_preference(mongoc_read_prefs_t* read_prefs)
        : _read_preference(read_prefs)
    {
    }

    ~read_preference() {
        mongoc_read_prefs_destroy(_read_preference);
    }

    const mongoc_read_prefs_t* get_read_preference() const {
        return _read_preference;
    }

 private:
    mongoc_read_prefs_t* _read_preference;

}; // class read_preference

}  // namespace priv
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
