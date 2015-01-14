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

#include "bson/document.hpp"
#include "stdx/optional.hpp"

namespace mongo {
namespace driver {
namespace model {

class LIBMONGOCXX_EXPORT update_many_t {

   public:
    update_many_t(bson::document::view filter, bson::document::view update);

    const bson::document::view& filter() const;
    const bson::document::view& update() const;

    update_many_t& upsert(bool upsert);
    const optional<bool>& upsert() const;

   private:
    bson::document::view _filter;
    bson::document::view _update;

    optional<bool> _upsert;

}; // class update_many_t

}  // namespace model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
