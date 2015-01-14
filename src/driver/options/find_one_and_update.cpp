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

#include "driver/options/find_one_and_update.hpp"

namespace mongo {
namespace driver {
namespace options {

void find_one_and_update_t::projection(bson::document::view projection) { _projection = projection; }

void find_one_and_update_t::return_document(return_document_t return_document) {
    _return_document = return_document;
}

void find_one_and_update_t::sort(bson::document::view ordering) { _ordering = ordering; }

void find_one_and_update_t::upsert(bool upsert) { _upsert = upsert; }

const optional<bson::document::view>& find_one_and_update_t::projection() const {
    return _projection;
}

const optional<return_document_t>& find_one_and_update_t::return_document() const {
    return _return_document;
}

const optional<bson::document::view>& find_one_and_update_t::sort() const { return _ordering; }

const optional<bool>& find_one_and_update_t::upsert() const { return _upsert; }

}  // namespace options
}  // namespace driver
}  // namespace mongo
