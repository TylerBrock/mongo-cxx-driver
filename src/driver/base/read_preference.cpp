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

#include "driver/base/read_preference.hpp"

#include "mongoc.h"

namespace mongo {
namespace driver {

read_preference::read_preference(read_mode mode) : _impl(std::make_unique<impl>(mongoc_read_prefs_new(static_cast<mongoc_read_mode_t>(mode)))) {
    if (arg.tags()) {
        bson::libbson::scoped_bson_t btags(arg.tags());
        mongoc_read_prefs_set_tags(wc, btags.bson());
    }
}

void read_preference::mode(read_mode mode) { _mode = mode; }

void read_preference::tags(bson::document::view tags) { _tags = std::move(tags); }

read_mode read_preference::mode() const { return _mode; }

const optional<bson::document::view>& read_preference::tags() const { return _tags; }

}  // namespace driver
}  // namespace mongo
