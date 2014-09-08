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

#include <vector>

#include "driver/config/prelude.hpp"

#include "bson/document.hpp"

namespace mongo {
namespace driver {
namespace model {

class LIBMONGOCXX_EXPORT insert_many {

   public:
    template <typename T>
    insert_many(const T& documents)
        : insert_many() {
        for (auto&& x : documents) {
            insert_one(x);
        }
    }

    insert_many();

    insert_many& insert_one(bson::document::view view);

    const std::vector<bson::document::view>& documents() const;

   private:
    std::vector<bson::document::view> _documents;
};

}  // namespace model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
