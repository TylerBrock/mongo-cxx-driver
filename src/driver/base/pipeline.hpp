/**
 * Copyright 2014 MongoDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "driver/config/prelude.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include "bson/document.hpp"

namespace mongo {
namespace driver {

class explain_result;

class MONGOCXX_EXPORT pipeline {
   public:
    pipeline& geoNear(/*something*/);
    pipeline& group(bson::document::view group);
    pipeline& limit(std::int32_t limit);
    pipeline& match(bson::document::view filter);
    pipeline& out(const std::string& collection_name);
    pipeline& project(bson::document::view projection);
    pipeline& redact(bson::document::view restrictions);
    pipeline& skip(std::int32_t skip);
    pipeline& sort(bson::document::view sort);
    pipeline& unwind(const std::string& field_name);

    bson::document::view operator[](int index) const;

   private:
    std::vector<bson::document::view> _pipeline;
};

}  // namespace driver
}  // namespace mongo
